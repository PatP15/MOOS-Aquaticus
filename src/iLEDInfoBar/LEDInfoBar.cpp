/************************************************************/
/*    NAME: Caileigh Fitzgerald                                              */
/*    ORGN: MIT                                             */
/*    FILE: LEDInfoBar.cpp                                        */
/*    DATE: May 5th 2018                                            */
/************************************************************/

#include "LEDInfoBar.h"
using namespace std;
bool debug = true;

//---------------------------------------------------------
// Constructor

LEDInfoBar::LEDInfoBar()
{
  if (debug)
    cout << "\nI'm in the LEDInfoBar constructor\n";
  // Icons state is set to m_OFF to start
  m_icons_map.insert(pair<TYPE_ENUM, STATE_ENUM>(m_TAGGED,       m_OFF));
  m_icons_map.insert(pair<TYPE_ENUM, STATE_ENUM>(m_HAVE_FLAG,    m_OFF));
  m_icons_map.insert(pair<TYPE_ENUM, STATE_ENUM>(m_FLAG_ZONE,    m_OFF));
  m_icons_map.insert(pair<TYPE_ENUM, STATE_ENUM>(m_IN_TAG_RANGE, m_OFF));
  m_icons_map.insert(pair<TYPE_ENUM, STATE_ENUM>(m_OUT_OF_BOUNDS,m_OFF));
}

//---------------------------------------------------------
// Destructor

LEDInfoBar::~LEDInfoBar()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LEDInfoBar::OnNewMail(MOOSMSG_LIST &NewMail)
{
  if (debug)
    cout << "\nI'm in OnNewMail()\n";

  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p; 
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    string sval  = msg.GetString(); 
    string str_out;

    if(key == "TAGGED")
    {
      if (sval=="blinking") m_icons_map[m_TAGGED] = m_BLINKING;       // check for blink
      else m_icons_map[m_TAGGED] = (sval=="true" ? m_ACTIVE : m_OFF); // updating state var
      str_out = toString(m_TAGGED, m_icons_map[m_TAGGED]);            // building str for ardunio
    }
    else if(key == "IN_FLAG_ZONE")
    {
      if (sval=="blinking") m_icons_map[m_FLAG_ZONE] = m_BLINKING;
      else m_icons_map[m_FLAG_ZONE] = (sval=="true" ? m_ACTIVE : m_OFF);
      str_out = toString(m_FLAG_ZONE, m_icons_map[m_FLAG_ZONE]);
    }
    else if(key == "OUT_OF_BOUNDS")
    {
      if (sval=="blinking") m_icons_map[m_OUT_OF_BOUNDS] = m_BLINKING;
      else m_icons_map[m_OUT_OF_BOUNDS] = (sval=="true" ? m_ACTIVE : m_OFF);
      str_out = toString(m_OUT_OF_BOUNDS, m_icons_map[m_OUT_OF_BOUNDS]);
    }
    else if(key == "HAVE_FLAG")
    {
      if (sval=="blinking") m_icons_map[m_HAVE_FLAG] = m_BLINKING;
      else m_icons_map[m_HAVE_FLAG] = (sval=="true" ? m_ACTIVE : m_OFF);
      str_out = toString(m_HAVE_FLAG, m_icons_map[m_HAVE_FLAG]);
    }
    else if (key == "IN_TAG_RANGE")
    {
      if (sval=="blinking") m_icons_map[m_IN_TAG_RANGE] = m_BLINKING;
      else m_icons_map[m_IN_TAG_RANGE] = (sval=="true" ? m_ACTIVE : m_OFF);
      str_out = toString(m_IN_TAG_RANGE, m_icons_map[m_IN_TAG_RANGE]);     
    }
    else if (key == "ALL_OFF")    // for debugging
    {
      str_out = toString(ALL_OFF, m_OFF);
    }
    else if (key == "ALL_ON")     // for debugging
    {
      str_out = toString(ALL_ON, (sval=="true" ? m_ACTIVE : m_OFF));
    }
      
    m_serial->WriteToSerialPort(str_out);
    m_serial->SerialSend();

#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString(); 
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool LEDInfoBar::OnConnectToServer()
{
  if (debug)
    cout << "\nI'm in OnConnectToServer()\n";
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);
	
   RegisterVariables();
   return(true);
}
//---------------------------------------------------------
// Procedure: toString(int i)
//            quick enum > string
string LEDInfoBar::toString(int i)
{
  stringstream ss;
  ss << i;
  return(ss.str());
}

//---------------------------------------------------------
// Procedure: toString(int type, int state)
//            quick enum > string OVERLOADED for type and state
string LEDInfoBar::toString(int type, int state)
{
  stringstream ss;
  ss << type << state;
  cout << "toString() : type|state = " << ss.str() << endl;
  return(ss.str());
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LEDInfoBar::Iterate()
{
  // if (debug)
  //   cout << "\nI'm in Iterate()\n";

  AppCastingMOOSApp::Iterate();

  m_valid_serial_connection = m_serial->IsGoodSerialComms();
  // checking again in case serial closes during execution
  if (!m_valid_serial_connection) // check arduino connection
  {
    m_valid_serial_connection = serialSetup(); // not connected, set it up
  }

  return(true);
}

//---------------------------------------------------------
// Procedure: serialSetup()
//            opens serial ports
bool LEDInfoBar::serialSetup()
{
  if (debug)
    cout << "\nI'm in serialSetup()\n";

  std::string errMsg = "";
  m_serial = new SerialComms(m_serial_port, m_baudrate, errMsg);

  if (m_serial->IsGoodSerialComms()) {
    m_serial->Run();
    string msg = "Serial port opened. Communicating over port ";
    msg += m_serial_port;

    if (debug) cout << '\t' << msg << endl;
    reportEvent(msg);
    return(true);
  }

  reportRunWarning("Unable to open serial port: " + errMsg);
  return(false);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LEDInfoBar::OnStartUp()
{
  if (debug)
    cout << "\nI'm in OnStartUp()\n";

  AppCastingMOOSApp::OnStartUp();

  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);

  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    bool handled = false;
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string orig  = *p;
      string line  = *p;
      string param = toupper(biteStringX(line, '='));
      string value = line;
    
      if(param == "PORT") { // define the port where we access the ardunio
        m_serial_port = line;
        handled = true;

        if(m_serial_port.empty()){
          if (debug) cout << "m_serial_port is empty" << endl;
          reportConfigWarning("Mission file parameter PORT must not be blank");
        }
      }
      if(param == "BAUDRATE"){ // define the speed at which we receive data
        m_baudrate = atoi(line.c_str());
        handled = true;
      }
      if(param == "TEAM_COLOR") {
        m_team_color=value;
        handled = true;
      }

      if(!handled)
        reportUnhandledConfigWarning(orig);

      handled = false; // reset
    }
  } else // !m_MissionReaderGetConfiguration()
    reportConfigWarning("No config block found for " + GetAppName());

  // now that we have everything to set up connection, do that now
  serialSetup();

  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void LEDInfoBar::RegisterVariables()
{
  if (debug)
   cout << "\nI'm in RegisterVariables()\n";

  Register("TAGGED"       , 0);
  Register("IN_FLAG_ZONE" , 0);
  Register("HAVE_FLAG"    , 0);
  Register("OUT_OF_BOUNDS", 0);
  Register("ALL_OFF"      , 0);
  Register("ALL_ON"       , 0);
}

// bool buildReport()
// {
//   AppCastingMOOSApp::buildReport();
//   return(true);
// }

