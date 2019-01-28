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
#include "NodeRecordUtils.h"

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
     else if (key == "NODE_REPORT"){
       std::string sval = msg.GetString();
       handleMailNodeReport(sval);
     }
     else if (key == "NAV_X"){
       
     }
     else if (key == "NAV_Y"){
 
     }
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
  map<string, NodeRecord>::const_iterator p;
  bool foundIntrudingContact = false;
  for(p=m_map_node_records.begin(); p!=m_map_node_records.end();p++) {
    std::string contact_name = p->first;
    NodeRecord node_record = p->second;

    //is contact name from opfor?
    bool followContact = strContains(contact_name, m_op_for);

    if(!followContact){
      //not a contact of opposing force
      continue;
    }
    else {
      //extract location and check if within bounds
      double op_for_x = node_record.getX();
      double op_for_y = node_record.getY();

      //check if within bounds
      //check x first
      if(op_for_x > m_min_x && op_for_x < m_max_x){
        //ok now check within y
        if(op_for_y > m_min_y && op_for_y < m_max_y){
          foundIntrudingContact = true;
          Notify("AGGRESSIVE","TRUE");
          Notify("AGGRESSIVE_CONTACT",contact_name);
          m_in_zone = "TRUE";
        }
      }
    }
  }
  if(!foundIntrudingContact){
    Notify("AGGRESSIVE","FALSE");
    m_in_zone = "FALSE";
  }

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ZoneTrackOpponents::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  m_ownship = m_host_community;

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
      handled = handleZoneAssignment(orig);
    }
    else if(param == "opfor"){
      handled = handleOpForAssignment(value);
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
  m_msgs << "Ownship: " << m_ownship << endl;
  m_msgs << "OpFor: " << m_op_for << endl;
  m_msgs << "min x: " << m_min_x << " max x: " << m_max_x << endl;
  m_msgs << "min y: " << m_min_y << " max y: " << m_max_y << endl;
  //show contacts and in or out of state
  m_msgs << "============================================ \n";
  m_msgs << "In Zone: " << m_in_zone << endl;
  m_msgs << "Contacts:" << endl;
  map<std::string, NodeRecord>::const_iterator p;
  for(p=m_map_node_records.begin(); p!=m_map_node_records.end();p++){
    std::string contact_name = p->first;
    NodeRecord node_record = p->second;
    std::string x = doubleToString(node_record.getX());
    std::string y = doubleToString(node_record.getY());

    m_msgs << contact_name << " x: " << x << " y: " << y << endl;
  }
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

//---------------------------------------------------------
// Procedure: handleMailNodeReport

void ZoneTrackOpponents::handleMailNodeReport(std::string report)
{
  NodeRecord new_node_record = string2NodeRecord(report, true);

  //if incoming node matches own ship, we just ignore it
  std::string vname = new_node_record.getName();

  if(vname == m_ownship)
    return;

  m_map_node_records[vname] = new_node_record;

}
 



