/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ZoneTrackOpponents.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "ZoneTrackOpponents.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ZoneTrackOpponents::ZoneTrackOpponents()
{
}

//---------------------------------------------------------
// Destructor

ZoneTrackOpponents::~ZoneTrackOpponents()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ZoneTrackOpponents::OnNewMail(MOOSMSG_LIST &NewMail)
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

bool ZoneTrackOpponents::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ZoneTrackOpponents::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ZoneTrackOpponents::OnStartUp()
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
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "foo") {
      handled = true;
    }
    else if(param == "bar") {
      handled = true;
    }
    else if(param == "zone"){
      handleZoneAssignment(orig);
    }
    else if(param == "opfor"){
      handleOpForAssignment(orig);
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void ZoneTrackOpponents::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NODE_REPORT",0);
  Register("NAV_X",0);
  Register("NAV_Y",0);

  // Register("FOOBAR", 0);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool ZoneTrackOpponents::buildReport() 
{
  m_msgs << "============================================ \n";
  m_msgs << "File:                                        \n";
  m_msgs << "============================================ \n";

  ACTable actab(4);
  actab << "Alpha | Bravo | Charlie | Delta";
  actab.addHeaderLines();
  actab << "one" << "two" << "three" << "four";
  m_msgs << actab.getFormattedString();

  return(true);
}

//---------------------------------------------------------
// Procedure: handleOpForAssignment

bool ZoneTrackOpponents::handleOpForAssignment(std::string orig)
{
  //expecting in .moos parameter file opfor = blue or red
  //TODO: error checking?
  m_op_for = orig;
  return true;
}

//---------------------------------------------------------
// Procedure: handleZoneAssignment

bool ZoneTrackOpponents::handleZoneAssignment(std::string orig)
{
  //TODO: implement any shape convex polygon of 4 points
  //expecting in .moos parameter file: zone = pts = xmin,ymin,xmax,ymax
  std::string ptsString = biteStringX(orig, '=');
  vector<std::string> str_vector = parseString(orig, ',');
  if(str_vector.size() !=4){
    return false;
  }

    //we will convert strings to double
  m_min_x = atof(str_vector[0].c_str());
  m_min_y = atof(str_vector[1].c_str());
  m_max_x = atof(str_vector[2].c_str());
  m_max_y = atof(str_vector[3].c_str());

 return true;
}




