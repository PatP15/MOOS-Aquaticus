/************************************************************/
/*    NAME: Caileigh Fitzgerald                                              */
/*    ORGN: MIT                                             */
/*    FILE: LEDInterpreter.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "LEDInterpreter.h"

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
  MOOSMSG_LIST::iterator p;
   
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
    else
      continue; // if state is unknown skip for now


    if (key == "TAGGED") {// * NEED TO CHANGE CONDITION keys & sval TO MATCH UFLDXX
      Notify("TAGGED", state);            
    }
    else if (key == "IN_FLAG_ZONE") {
      Notify("IN_FLAG_ZONE", state); 
    }
    else if (key == "OUT_OF_BOUNDS") {
      Notify("OUT_OF_BOUNDS", state); 
    }
    else if (key == "HAVE_FLAG") {
      Notify("HAVE_FLAG", state); 
    }
    else if (key == "IN_TAG_RANGE") {
      Notify("IN_TAG_RANGE", state);     
    }
    else if (key == "ALL_OFF") {  // for debugging
      Notify("ALL_OFF", state); 
    }
    else if (key == "ALL_ON") {   // for debugging
      Notify("ALL_ON", state); 
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
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);
	
   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool LEDInterpreter::Iterate()
{
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool LEDInterpreter::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string line  = *p;
      string param = tolower(biteStringX(line, '='));
      string value = line;
      
      if(param == "foo") {
        //handled
      }
      else if(param == "bar") {
        //handled
      }
    }
  }
  
  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void LEDInterpreter::RegisterVariables()
{
  Register("TAGGED"       , 0);     // need to edit variables names for actual use
  Register("IN_FLAG_ZONE" , 0);
  Register("HAVE_FLAG"    , 0);
  Register("OUT_OF_BOUNDS", 0);
  Register("ALL_OFF"      , 0);
  Register("ALL_ON"       , 0);
}

