/************************************************************/
/*    NAME: Carter Fendley                                  */
/*    ORGN: MIT                                             */
/*    FILE: ButtonBox.cpp                                   */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "ButtonBox.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ButtonBox::ButtonBox()
{

  iterate_counter = 0;
  
}

//---------------------------------------------------------
// Destructor

ButtonBox::~ButtonBox()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ButtonBox::OnNewMail(MOOSMSG_LIST &NewMail)
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

     if(key != "APPCAST_REQ") // handled by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ButtonBox::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ButtonBox::Iterate()
{
  AppCastingMOOSApp::Iterate();
 
  m_valid_serial_connection = m_serial->IsGoodSerialComms(); //check for good connection to arduino

  if(!m_valid_serial_connection){
    m_valid_serial_connection = serialSetup(); // setup interface to arduino
  }
 
  while(m_valid_serial_connection && m_serial->DataAvailable()){ // grab data from arduino
    string data = m_serial->GetNextSentence(); 
    
    parseSerialString(data);
  }

  if (iterate_counter == 0) {

    for(std::vector<int>::size_type i = 0; i != m_button_values.size(); i++) { // post data to moos variables

      previous_button_values.push_back(m_button_values[i]);
      
    }

  } else {

      for(std::vector<int>::size_type i = 0; i != m_button_values.size(); i++) { // post data to moos variables

	if (previous_button_values[i] != m_button_values[i]) {

	  m_Comms.Notify(getName(i), m_button_values[i]);

	}
	
      }

  }

  iterate_counter++;

  
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ButtonBox::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams))
    reportConfigWarning("No config block found for " + GetAppName());

  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = toupper(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "PORT") { // define the port where we access the arduino
      handled = true;
      m_serial_port = line;
      
      if(m_serial_port.empty()){
	    reportConfigWarning("Mission file parameter PORT must not be blank");
      }
    }

    if(param == "BAUDRATE"){ // define the speed at which we receive data
        handled = true;
        m_baudrate = atoi(line.c_str());
    }

    if(param.substr(0,7) == "BUTTON_" && param.substr(param.length() - 5, param.length() ) == "_NAME"){ // grab the name of the buttons
      handled = true;
      m_button_names[param] = line;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
 
  m_valid_serial_connection = serialSetup(); 

  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void ButtonBox::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool ButtonBox::buildReport() 
{

  m_msgs << endl << "SETUP" << endl << "-----" << endl;
  m_msgs << "	PORT: " << m_serial_port << endl;
  m_msgs << "	BAUDRATE: " << m_baudrate << endl;

  m_msgs << endl << "STATUS" << endl << "-----" << endl;
  m_msgs << "	Valid serial connection: " << std::boolalpha << m_valid_serial_connection << endl;

  m_msgs << endl;

  if(m_button_values.size() == 0){
    m_msgs << "	No current button values." << endl;
  }  

  for(std::vector<int>::size_type i = 0; i != m_button_values.size(); i++) {
    m_msgs << getName(i) << ": " << m_button_values[i] << endl; 
  }

  return(true);
}

bool ButtonBox::serialSetup()
{
  std::string errMsg = "";
  m_serial = new SerialComms(m_serial_port, m_baudrate, errMsg);

  if (m_serial->IsGoodSerialComms()) {
    m_serial->Run();
    string msg = "Serial port opened. Communicating over port ";
    msg += m_serial_port;
    reportEvent(msg);
    return(true);
  }

  reportRunWarning("Unable to open serial port: " + errMsg);
  return(false);
}

void ButtonBox::parseSerialString(std::string data) //parse data sent via serial from arduino
{ 
  if(data.at(0) != '$'){
    reportRunWarning("Malformed data string! Does not begin with $ char");
    return;
  }

  std::string values = data.substr(data.find(":") + 1);
  
  for(unsigned int i = 0; i<values.length(); i++) {
    char c = values[i];
    if(c != '0' && c != '1' && c != ','){
      std::string err = "Malformed data string! Unrecognised char: ";
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
  
  m_button_values = button_values;
}

std::string ButtonBox::getName(int button_index){ //grab names of buttons for moos variables (defined in .moos file/config area)
    std::string key = "BUTTON_";
    std::ostringstream oss;
    oss << button_index << "_NAME";
    key += oss.str();

    if(m_button_names.find(key) == m_button_names.end()){
      oss.str("");
      oss.clear();

      oss << "BUTTON_" << button_index;
      return oss.str();
    }

    return m_button_names[key];

}
