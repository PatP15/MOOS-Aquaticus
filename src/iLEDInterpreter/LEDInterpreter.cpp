/************************************************************/
/*    NAME: Caileigh Fitzgerald                                              */
/*    ORGN: MIT                                             */
/*    FILE: LEDInterpreter.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "LEDInterpreter.h"
bool debug = true;

using namespace std;

//---------------------------------------------------------
// Constructor

LEDInterpreter::LEDInterpreter()
{
}

//---------------------------------------------------------
// Destructor

LEDInterpreter::~LEDInterpreter()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool LEDInterpreter::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  bool handled = false;
   
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    string sval  = msg.GetString(); 
    string state;

    if (sval == "blinking") {
      state = "blinking";
    }
    else if (sval == "true") {
      state = "true";
    }
    else if (sval == "false") {
      state = "false";
    }


    if (key == m_tagged_var) {// * NEED TO CHANGE CONDITION keys & sval TO MATCH UFLDXX
      Notify("TAGGED", state);            
    }
    else if (key == m_flag_zone_var) {
      Notify("IN_FLAG_ZONE", state); 
    }
    else if (key == m_out_of_bounds_var) {
      Notify("OUT_OF_BOUNDS", state); 
    }
    else if (key == m_have_flag_var) {
      Notify("HAVE_FLAG", state); 
    }
    else if (key == m_in_tag_range_var) {
      Notify("IN_TAG_RANGE", state);     
    }

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

bool LEDInterpreter::OnConnectToServer()
{
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LEDInterpreter::Iterate()
{
  AppCastingMOOSApp::Iterate();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LEDInterpreter::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  bool handled = false;
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string orig  = *p;
      string line  = *p;
      string param = toupper(biteStringX(line, '='));
      string value = line;

      if (debug)
        cout << "value=" << value << endl;
      
      if(param == "TAGGED")
      {
        m_tagged_var=value;
        handled = true;
      }
      if (param == "OUT_OF_BOUNDS")
      {
        m_out_of_bounds_var=value;
        handled = true;
      }
      if (param == "HAVE_FLAG") 
      {
        m_have_flag_var=value;
        handled = true;
      }
      if (param == "IN_TAG_RANGE") 
      {
        m_in_tag_range_var=value;
        handled = true;
      }
      if (param == "IN_FLAG_ZONE") 
      {
        m_flag_zone_var=value;
        handled = true;
      }

      if(!handled)
        reportUnhandledConfigWarning(orig);

      handled = false; // reset
    }
  }
  
  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void LEDInterpreter::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register(m_tagged_var        , 0);
  Register(m_out_of_bounds_var , 0);
  Register(m_have_flag_var     , 0);
  Register(m_in_tag_range_var  , 0);
  Register(m_flag_zone_var     , 0);\
}

bool LEDInterpreter::buildReport()
{
  m_msgs << "I'm in buildReport()\n";
  return(true);
}
