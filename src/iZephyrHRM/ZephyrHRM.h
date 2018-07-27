/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ZephyrHRM.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef ZephyrHRM_HEADER
#define ZephyrHRM_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MOOS/libMOOS/Utils/MOOSThread.h"
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>

struct zephyr_packet{
  unsigned char *data;
  unsigned int size;
};

struct hrm_data{
  bool worn;
  short hr_conf;
  double bat_volt;

  int hr;
  int hrv;
  short posture;
  double resp_rate;
};


class ZephyrHRM : public AppCastingMOOSApp
{
 public:
   ZephyrHRM();
   ~ZephyrHRM();
   static bool BTThread(void* param);
   static bool sendLifeSign(int &s);
   static bool requestGeneralPacket(int &s, bool active);
   static bool requestSummaryPacket(int &s, bool active);
   void gotLifeSign();
   bool isConnectionStale();
   void NewBytes();
   void ReportPacket(struct zephyr_packet* packet);

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
   static unsigned crc8(unsigned char const *data, size_t len);
   static void CopyRange(const unsigned char* in, int start, int size, unsigned char* out);
   void NewPacket(struct zephyr_packet* packet);

 private:
   CMOOSThread* m_bt_thread;

   std::string m_bt_mac;
   int m_bt_channel;
   int m_packet_num;
   int m_life_sign_c;
   float m_last_life_sign_time;
   static const int m_bt_data_buf_size = 1024;
   unsigned char m_bt_data_buf[m_bt_data_buf_size];

   struct hrm_data m_last_hrm_data;
   struct bt_data* m_comms_data; 

   const static int STX = 0x02;
   const static int ETX = 0x03;
   const static int ACK = 0x06;
   const static int NAK = 0x15;

   const static int STX_POS = 0;
   const static int MSG_POS = STX_POS + 1;
   const static int DLC_POS = MSG_POS + 1;
   const static int PAYLOAD_POS = DLC_POS + 1;

   const static int MIN_LENGTH = 5;
};

struct bt_data{
  std::string mac;
  int channel;
  bool connected;

  int buf_size;
  unsigned char* buf;

  int buf_begin;
  int buf_end;

  unsigned int failed_writes;

  CMOOSLock lock;

  ZephyrHRM* call_back;
};

#endif 
