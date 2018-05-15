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
  // Icons state is set to m_OFF by default
  // you can set the state to something else here as well
  m_icons.push_back(Icon(m_TAGGED));
  m_icons.push_back(Icon(m_HAVE_FLAG));
  m_icons.push_back(Icon(m_FLAG_ZONE));
  m_icons.push_back(Icon(m_IN_TAG_RANGE));
  m_icons.push_back(Icon(m_OUT_OF_BOUNDS));
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
// Procedure: deconflictStates()
//            called in Iterate() to make sure LED update is logical
bool LEDInfoBar::deconflictStates(Icon updated_icon)
{
  return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LEDInfoBar::Iterate()
{
  if (debug)
    cout << "\nI'm in Iterate()\n";

  AppCastingMOOSApp::Iterate();

  m_valid_serial_connection = m_serial->IsGoodSerialComms();
  if (!m_valid_serial_connection) // check arduino connection
  {
    m_valid_serial_connection = serialSetup(); // not connected, set it up
  }

  bool testing = true;
  while(m_valid_serial_connection && testing) 
  {
    if (debug)
      cout << "I'm inside while (m_valid_serial_connection && testing)\n";
    /*
    iterate through the vector of Icons and update them with new info
    ?? write a function to work out the conflicts with each LED situation ?? 
    */
    //if (deconflictStates(Icon(TAGGED, OFF, "")))
    if (debug)
    {
      // update and push to arduino
      m_serial->WriteToSerialPort("0");
      if (debug) cout << "\nSanity check! m_HAVE_FLAG=" << (m_HAVE_FLAG) << endl;
      m_serial->SerialSend();
      testing = false;
    }
    // else 
    // {
    //   // move on and don't update
    // }
  }


      //m_serial->serialport_flush();

  if (debug)
      cout << "I'm OUTSIDE while (m_valid_serial_connection && testing)\n";

  return(true);
}

//---------------------------------------------------------
// Procedure: parseSerialString(str)
//            opens serial ports

// GenerateSerial string instead??
void LEDInfoBar::parseSerialString(string data) //parse data sent via serial from arduino
{
  if (debug)
    cout << "\nI'm in parseSerialString()\n";

  if(data.at(0) != '$'){
    reportRunWarning("Malformed data string! Does not begin with $ char");
    return;
  }

  std::string values = data.substr(data.find(":") + 1);

  for(unsigned int i = 0; i<values.length(); i++) {
    char c = values[i];
    if(c != '0' && c != '1' && c != ','){
      std::string err = "Malformed data string! Unrecognized char: ";
      err += c;
      reportRunWarning(err);
      return;
    }
  }

  std::stringstream ss(values);
  std::vector<std::string> button_values;
  while( ss.good() ){
    string substr;
    getline( ss, substr, ',' );

    if(substr.compare("0") == 0){
      button_values.push_back("TRUE");
    }else if(substr.compare("1") == 0){
      button_values.push_back("FALSE");
    }
  }

  //m_button_values = button_values;
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

      if (debug) cout << "param=" << param << endl;
      if (debug) cout << " line=" << line << endl;
    
      if(param == "PORT") { // define the port where we access the arduino
        if (debug) cout << "in PORT block" << endl;
        m_serial_port = line;
        handled = true;

        if(m_serial_port.empty()){
          if (debug) cout << "m_serial_port is empty" << endl;
          reportConfigWarning("Mission file parameter PORT must not be blank");
        }
      }
      if(param == "BAUDRATE"){ // define the speed at which we receive data
        if (debug) cout << "in BAUDRATE block" << endl;
        m_baudrate = atoi(line.c_str());
        handled = true;
      }
      if(param == "TEAM_COLOR") {
        m_team_color=value;
        handled = true;
      }

      if(!handled)
        reportUnhandledConfigWarning(orig);
    }
  } else // !m_MissionReaderGetConfiguration()
    reportConfigWarning("No config block found for " + GetAppName());

  // now that we have everything to set up connection, do that now
  // had an issue with the serial port thinking it was setup when it wasn't
  // since I added it here it works
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
}

// bool buildReport()
// {
//   return(true);
// };






// /*
// /************************************************************/
// /*    NAME: Caileigh Fitzgerald                                              */
// /*    ORGN: MIT                                             */
// /*    FILE: LEDInfoBar.h                                          */
// /*    DATE:                                                 */
// /************************************************************/

// #ifndef LEDInfoBar_HEADER
// #define LEDInfoBar_HEADER

// #include "MBUtils.h"
// #include "MOOS/libMOOS/MOOSLib.h"
// #include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
// #include <string>
// #include <vector>
// #include <list>
// #include <iterator>

// class LEDInfoBar : public AppCastingMOOSApp
// {
//  public:
//   LEDInfoBar();
//   virtual ~LEDInfoBar();

//   bool OnNewMail(MOOSMSG_LIST &NewMail);
//   bool Iterate();
//   bool OnConnectToServer();
//   bool OnStartUp();
//   void RegisterVariables();
//   //-----------------------------------------------
//   // state for LED light state
//   enum STATE_ENUM
//   {
//     m_OFF,      // off no error
//     m_ACTIVE,   // steady light on
//     m_BLINKING, // blinking light on
//     m_ERROR     // error can't turn on LED
//   };
//   //------------------------------------------------
//   // Enums to make types more clear when comparing
//   // .. to new mail and vars from other apps
//   // 
//   enum TYPE_ENUM
//   {
//     m_HAVE_FLAG,
//     m_FLAG_ZONE,
//     m_TAGGED,
//     m_IN_TAG_RANGE,
//     m_OUT_OF_BOUNDS
//   };

// protected:
//   //------------------------------------------------
//   // struct for list of icons to keep track of LED states
//   //
//   struct Icon 
//   { 
//     Icon(TYPE_ENUM t, STATE_ENUM s=OFF, std::string str="") 
//     {
//       // type set in constructor
//       // optional state 
//       // optional string value, maybe fill in register for vars?
//       m_type_str = str;
//       m_type     = t; 
//       m_state    = s;
//     }
    
//     std::string m_type_str; // to keep track of different naming conventions
//     TYPE_ENUM   m_type; 
//     STATE_ENUM  m_state; 
//   }; 

//   std::vector<Icon> m_icons;
//   // Standard AppCastingMOOSApp function to overload
//   //bool buildReport();
// };

// #endif 

// */