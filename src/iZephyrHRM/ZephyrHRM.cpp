/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ZephyrHRM.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include "ZephyrHRM.h"
#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include <iostream>
#include <sstream>
#include <unistd.h>

using namespace std;


//---------------------------------------------------------
// Constructor

ZephyrHRM::ZephyrHRM()
{
  m_bt_mac = "";
  m_bt_channel = 1;
  m_packet_num = 0;
  m_life_sign_c = 0;

  m_last_hrm_data.worn = false;
  m_last_hrm_data.hr_conf = -1;
  m_last_hrm_data.bat_volt = -1;

  m_last_hrm_data.hr = -1;
  m_last_hrm_data.hrv = -1;
  m_last_hrm_data.posture = -1;
  m_last_hrm_data.resp_rate = -1;
}

//---------------------------------------------------------
// Destructor

ZephyrHRM::~ZephyrHRM(){
}

unsigned ZephyrHRM::crc8(unsigned char const *data, size_t len){
  unsigned crc8Poly = 0x8C;
  unsigned crc = 0;
  for(int i=0; i<len; i++){
    crc = (crc ^ data[i]) & 255;
    for(int loop=0; loop<8; loop++){
      if((crc & 1) == 1)
        crc = ((crc >> 1) ^ crc8Poly);
      else
        crc = (crc >> 1);
    }
    crc = crc & 255;
  }
  return crc;
}

void ZephyrHRM::CopyRange(const unsigned char* in, int start, int size, unsigned char* out){
  int c = 0;
  for(int i=start; c<size; i++){
    out[c] = in[i];
    c += 1;
  }
}

bool ZephyrHRM::sendLifeSign(int &s){
  unsigned char life_sign[] = {0x02, 0x23, 0x00, 0x00, 0x03};
  int size = sizeof(life_sign);
  int status = write(s, life_sign, size);

  if (status != size)
    return false;
  return true;
}

bool ZephyrHRM::requestGeneralPacket(int &s, bool active){
  unsigned char data[] = {0x02, 0x14, 0x01, 0x00, 0x00, 0x03};
  if(active)
    data[3] = 0x01;

  unsigned char crcCalc[1];
  CopyRange(data, 3, 1, crcCalc);
  data[4] = crc8(crcCalc, 1);

  size_t size = sizeof(data);

  int status = write(s, data, sizeof(data));

  if(status != size){
    return false;
  }
  return true;
}

bool ZephyrHRM::requestSummaryPacket(int &s, bool active){
  unsigned char data[] = {0x02, 0xBD, 0x02, 0x00, 0x00, 0x00, 0x03};
  if(active)
    data[3] = 0x01;

  unsigned char crcCalc[1];
  CopyRange(data, 3, 2, crcCalc);
  data[5] = crc8(crcCalc, 2);

  size_t size = sizeof(data);

  int status = write(s, data, sizeof(data));

  if(status != size){
    return false;
  }
  return true;
}

bool ZephyrHRM::BTThread(void* param){
  struct bt_data* data = (struct bt_data*)param;
  ZephyrHRM* main_t = (ZephyrHRM*) data->call_back;

  int s, status;
  s = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);

  struct sockaddr_rc addr = {0};
  addr.rc_family = AF_BLUETOOTH;
  addr.rc_channel = (uint8_t) data->channel;
  str2ba(data->mac.c_str(), &addr.rc_bdaddr);

  status = -1;
  while(status !=0){
    status = connect(s, (struct sockaddr*)&addr, sizeof(addr));
    data->connected = true;
  }

  usleep(1000);

  requestGeneralPacket(s, true);
  requestSummaryPacket(s, true);

  data->buf_begin = 0;
  data->buf_end = 0;

  while(1){
    //Mutex lock to prevent race conditions
    data->lock.Lock();
    int bytes_read = read(s, data->buf, data->buf_size-(data->buf_end));

    //Increase the size of end
    data->buf_end += bytes_read;
    data->lock.UnLock();

    if(bytes_read > 0){
      main_t->NewBytes();
    }

    status = sendLifeSign(s);
  }
}

void ZephyrHRM::ReportPacket(struct zephyr_packet* packet){
  std::stringstream ss;
  
  for(int i=0; i<packet->size; i++){
    ss << "[" << i << "] 0x" << std::setw(2) << std::setfill('0') << std::hex << (int)(packet->data[i]) << ", ";
  }
  ss << "\n";
  reportEvent(ss.str());
}

static std::string c2hex(unsigned char c){
  std::stringstream ss;
  ss << "0x" << std::setw(2) << std::setfill('0') << std::hex << (int)(c);
  return ss.str();
}

void ZephyrHRM::NewBytes(){
  m_comms_data->lock.Lock();
  static unsigned char* buf = m_comms_data->buf;
  int begin = m_comms_data->buf_begin;
  int end = m_comms_data->buf_end;

  if(false){
    std::stringstream ss;
    
    for(int i=begin; i<end; i++){
      ss << "0x" << std::setw(2) << static_cast<unsigned>( buf[i]) << ", ";
    }
    reportEvent(ss.str());
  }

  for(int i=begin; i<end; i++){
    if(buf[i] == STX){
      int size = 3 + buf[i+DLC_POS] + 2; // STX + MSGID + DLC + (Payload size) + CRC + ACK/NAK/ETX

      if(buf[i+(size-1)] == ETX || buf[i+(size-1)] == ACK || buf[i+(size-1)] == NAK){
        struct zephyr_packet* packet = new zephyr_packet;
        unsigned char packet_data[size];
        memcpy(packet_data, buf+i, size);
        
        packet->data = packet_data;
        packet->size = sizeof(packet_data);
        
        begin = i+(size);
        i += size-1;

        NewPacket(packet);
      }else{
        reportRunWarning("STX found. But no ETX/ACK/NAK at position specified by DLC");
      }
    }
  }

  if(begin != 0){
    //Shift any remaining data back to beginning of buffer
    memmove(buf+begin, buf, end-begin);
    end -= begin;
    
    m_comms_data->buf_begin = 0;
    m_comms_data->buf_end = end;
  }
      
  m_comms_data->lock.UnLock();
}

void ZephyrHRM::NewPacket(struct zephyr_packet* packet){
  std::stringstream ss;
  if(false){
    ss << "Number of bytes in packet: " <<  packet->size << std::endl;
    for(int i=0; i<packet->size; i++){
      ss << "0x" << std::setw(2) << static_cast<unsigned>( packet->data[i]) << ", ";
    }
    reportEvent(ss.str());
  }

  if(packet->size <= 0)
    reportRunWarning("Packet size zero! (This really shouldnt happend)");

  if(packet->data[STX_POS] != STX)
    reportRunWarning("STX not in correct position");

  if(packet->size < MIN_LENGTH)
    reportRunWarning("Packet length under min length");

  unsigned char dlc = packet->data[DLC_POS];
  if(packet->size < dlc + MIN_LENGTH)
    reportRunWarning("Packet size doesn't match DLC");

  unsigned char crc = packet->data[PAYLOAD_POS + dlc];
  unsigned char payload[dlc];
  CopyRange(packet->data, PAYLOAD_POS, dlc, payload);
  if(crc8(payload, sizeof(payload)) != crc)
    reportRunWarning("Packet parity check failed");

  unsigned char end = packet->data[PAYLOAD_POS + dlc + 1];
  if( end != ACK && end != NAK && end != ETX)
    reportRunWarning("Packet doesn't end in ACK/NAK/ETX");

  unsigned char msgID = packet->data[MSG_POS];

  if(msgID == 0x14){
    if(end == ACK)
      reportEvent("General packet request ACK. General packets should now be streaming.\n");
    else if(end == NAK) //Not sure if NAK is ever sent in zephyr protocol
      reportEvent("General packet request NAK.\n");
  }else if(msgID == 0x23){
    m_life_sign_c += 1;
  }else if(msgID == 0x2C){
    //Event Packet
  }else if(msgID == 0x20){
    //GENERAL PACKET
    int hr = (int) payload[9] & 0xFF;
    double resp_rate = (double) ((short)(payload[11] & 0xFF) | ((payload[12] & 0xFF) << 8))/10;
    
    int hr_confidence = (int) payload[37];
    short posture = (short) ((payload[15] & 0xFF) | ((payload[16] & 0xFF) << 8));
    
    short bat_temp = (short) (payload[21] & 0xFF) | ((payload[22] & 0xFF) << 8);
    double bat_volt = (double) (bat_temp/1000);

    unsigned char worn_status = (payload[52] & 0x80) >> 7;
    if(worn_status == 0x1)
      m_last_hrm_data.worn = true;
    else
      m_last_hrm_data.worn = false;

    if(m_last_hrm_data.hr != hr){
      m_last_hrm_data.hr = hr;
      Notify("HEART_RATE", hr);
    }
    if(m_last_hrm_data.posture != posture){
      m_last_hrm_data.posture = posture;
      Notify("POSTURE", posture);
    }
    if(m_last_hrm_data.bat_volt != bat_volt){
      m_last_hrm_data.bat_volt = bat_volt;
    }
    if(m_last_hrm_data.resp_rate != resp_rate){
      m_last_hrm_data.resp_rate = resp_rate;
      Notify("RESPERATION_RATE", resp_rate);
    }
  }else if(msgID == 0xBD){
    if(end == ACK)
      reportEvent("Summary packet request ACK. Summary packets should now be streaming");
    else if(end == NAK)
      reportEvent("Summary packet request NAK.");
    
  }else if(msgID == 0x2B){
    //SUMMARY PACKET
    //ReportPacket(packet);
    short hr_conf = (short) (payload[34] & 0xFF);
    int hrv = (int)((payload[35] & 0xFF) | ((payload[36] & 0xFF) << 8));
    
    if(m_last_hrm_data.hrv != hrv){
      m_last_hrm_data.hrv = hrv;
      Notify("HEART_RATE_VARIABILITY", hrv);
    }
    if(m_last_hrm_data.hr_conf != hr_conf){
      m_last_hrm_data.hr_conf = hr_conf;
      Notify("HEART_RATE_CONFIDENCE", hr_conf);
    }
  }else{
    std::stringstream warning;
    //warning << "Unhandled packet with ID: " << std::setw(2) << (0xFF & msgID);
    warning << "Unhandled packet with ID: 0x" << std::setw(2) << std::setfill('0') << std::hex << (int)(msgID);
   reportRunWarning(warning.str());
    //printf("Unhandled message with ID: %X\n", msgID);
  }

  m_packet_num += 1;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ZephyrHRM::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key    = msg.GetKey();

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

     if(key == "FOO") 
       cout << "great!";

     else if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ZephyrHRM::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ZephyrHRM::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ZephyrHRM::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  std::string name = GetAppName();
  //m_MissionReader.GetConfiguration(GetAppName(), sParams);
  if(!m_MissionReader.GetConfiguration("iZephyrHRM", sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;

    if(param == "mac") {
      m_bt_mac = value;
      handled = true;
    }
    else if(param == "channel") {
      std::istringstream(value) >> m_bt_channel;
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  if(m_bt_mac == ""){
    reportRunWarning("No MAC address given!");
  }
  
  m_comms_data = new bt_data();

  m_comms_data->mac = m_bt_mac;
  m_comms_data->channel = m_bt_channel;
  m_comms_data->connected = false;

  m_comms_data->call_back = this;
  m_comms_data->buf = m_bt_data_buf;
  m_comms_data->buf_size = m_bt_data_buf_size;

  m_comms_data->buf_end = 0;
  m_comms_data->buf_begin = 0;

  m_comms_data->failed_writes = 0;

  m_comms_data->lock = new CMOOSLock();

  m_bt_thread = new CMOOSThread();
  m_bt_thread->Initialise(BTThread, m_comms_data);
  m_bt_thread->Start();

  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void ZephyrHRM::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool ZephyrHRM::buildReport() 
{
  m_msgs << "\n============ Connection Status ============\n";
  m_msgs << "MAC: " << m_bt_mac.c_str() << "\n";
  m_msgs << "Channel: " << m_bt_channel << "\n";
  m_msgs << "Connected: " << std::boolalpha << m_comms_data->connected << "\n\n";

  if(m_comms_data->connected){
    m_msgs << "=============== Packet Info ===============\n";
    m_msgs << "Failed writes: " << m_comms_data->failed_writes << "\n";
    m_msgs << "Number of packets: " << m_packet_num << "\n";
    m_msgs << "Life sign count: " << m_life_sign_c << "\n\n";
    m_msgs << "=============== General Info===============\n";
    m_msgs << "Worn Status: " << std::boolalpha << m_last_hrm_data.worn << "\n";
    m_msgs << "Heart Rate Confidence: " << m_last_hrm_data.hr_conf << " %\n";
    m_msgs << "Battery Voltage: " << m_last_hrm_data.bat_volt << " V\n\n";
    m_msgs << "=============== Vital Signs ===============\n";
    m_msgs << "Heart Rate: " << m_last_hrm_data.hr << " bpm\n";
    m_msgs << "Heart Rate Variability: " << m_last_hrm_data.hrv << " ibi\n";
    m_msgs << "Posture: " << m_last_hrm_data.posture << " deg\n";
    m_msgs << "Resp Rate: " << m_last_hrm_data.resp_rate << " bpm\n";

  }

  return(true);
}



