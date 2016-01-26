/************************************************************/
/*    NAME: Hugh Dougherty                                  */
/*    ORGN: MIT                                             */
/*    FILE: Blinkstick.cpp                                  */
/*    DATE: January 7, 2016                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "Blinkstick.h"
#include <stdlib.h>
#include <string>

using namespace std;

//---------------------------------------------------------
// Constructor

Blinkstick::Blinkstick()
{
  m_iterations = 0;
  m_timewarp   = 1;
}

//---------------------------------------------------------
// Destructor

Blinkstick::~Blinkstick()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Blinkstick::OnNewMail(MOOSMSG_LIST &NewMail)
{
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

//iBlink C++ for MOOS                                                      
  string test;
  test = "blinkstick --pulse red --repeats=5";
    system(test.c_str());
    return 0;
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Blinkstick::OnConnectToServer()
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

bool Blinkstick::Iterate()
{
  m_iterations++;
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Blinkstick::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);
      
      if(param == "FOO") {
        //handled
      }
      else if(param == "BAR") {
        //handled
      }
    }
  }
  
  m_timewarp = GetMOOSTimeWarp();

  RegisterVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void Blinkstick::RegisterVariables()
{
  // AppCastingMOOSApp::RegisterVariables();
  // Register("FOOBAR", 0);
  Register("BLINK", 0);
}
