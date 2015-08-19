/************************************************************/
/*   NAME: Mike Benjamin                                    */
/*   ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA */
/*   FILE: FlagManager.cpp                                  */
/*   DATE: August 18th, 2015                                */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "FlagManager.h"
#include "NodeRecordUtils.h"
#include "XYMarker.h"
#include "XYFormatUtilsMarker.h"

using namespace std;

//---------------------------------------------------------
// Constructor

FlagManager::FlagManager()
{
  // Default config values
  m_default_flag_range    = 10; // meters
  m_report_flags_on_start = true;

  // Default state values
  m_total_node_reports_rcvd  = 0;
  m_total_grab_requests_rcvd = 0;

  m_grabbed_color   = "white";
  m_ungrabbed_color = "red";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool FlagManager::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    string sval  = msg.GetString(); 
    string comm  = msg.GetCommunity();

#if 0 // Keep these around just for template
    double dval  = msg.GetDouble();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

    bool handled = false;
    if(key == "NODE_REPORT") 
      handled = handleMailNodeReport(sval);
    else if(key == "FLAG_GRAB_REQUEST") 
      handled = handleMailFlagGrab(sval, comm);
    else 
      reportRunWarning("Unhandled Mail: " + key);
  }
	
  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool FlagManager::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool FlagManager::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool FlagManager::OnStartUp()
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
    if(param == "flag") 
      handled = handleConfigFlag(value);

    if(!handled)
      reportUnhandledConfigWarning(orig);
  }
  
  // Post a bunch of viewable artifacts
  postFlagMarkers();

  // Possibly post a report for all vehicles to know the flags
  if(m_report_flags_on_start)
    postFlagSummary();

  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void FlagManager::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("FLAG_GRAB_REQUEST", 0);
  Register("NODE_REPORT", 0);
}


//---------------------------------------------------------
// Procedure: handleConfigFlag
//   Example: flag = x=2,y=3,range=10,label=one


bool FlagManager::handleConfigFlag(string str)
{
  XYMarker flag = string2Marker(str);

  if(!flag.is_set_x() || !flag.is_set_y()) {
    reportConfigWarning("Flag with missing x-y position: " + str);
    return(false);
  }
  if(flag.get_label() == "") {
    reportConfigWarning("Flag with missing label: " + str);
    return(false);
  }

  if(!flag.is_set_range())
    flag.set_range(m_default_flag_range);


  // Ensure that a unique label has been provided
  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(flag.get_label() == m_flags[i].get_label()) {
      reportConfigWarning("Flag with duplicate label: " + str);
      return(false);
    }
  }

  m_flags.push_back(flag);
  m_flags_ownedby.push_back("");
  
  return(true);
}

//------------------------------------------------------------ 
// Procedure: handleMailNodeReport      

bool FlagManager::handleMailNodeReport(string str)
{
  NodeRecord new_record = string2NodeRecord(str);

  string whynot;
  if(!new_record.valid("x,y,name", whynot)) {
    reportRunWarning("Unhandled NodeReport: " + whynot);
    return(false);
  }

  string vname = toupper(new_record.getName());

  m_map_record[vname] = new_record;
  m_map_tstamp[vname] = m_curr_time;

  if(m_map_rcount.count(vname) == 0)
    m_map_rcount[vname] = 1;
  else
    m_map_rcount[vname]++;

  m_total_node_reports_rcvd++;

  return(true);
}

//------------------------------------------------------------ 
// Procedure: handleMailFlagGrab
//   Example: GRAB_FLAG_REQUEST = "vname=henry"

bool FlagManager::handleMailFlagGrab(string str, string community)
{

  // Part 1: Parse the Grab Request
  string grabbing_vname = tokStringParse(str, "vname", ',', '=');

  // Part 2: Sanity check on the Grab Request
  // If the grabbing vehicle is not set, return false
  if(grabbing_vname == "")
    return(false);

  // If no node records of the grabbing vehicle, return false
  string up_vname = toupper(grabbing_vname);
  if(m_map_record.count(up_vname) == 0)
    return(false);

  // Part 3: OK grab, so increment counters.
  m_map_grab_count[up_vname]++;
  m_total_grab_requests_rcvd++;
  
  // Part 4: Get the grabbing vehicle's position from the record
  NodeRecord record = m_map_record[up_vname];
  double curr_vx = record.getX();
  double curr_vy = record.getY();

  // Part 5: For each flag with the grab_dist of the vehicle, GRAB
  string result;
  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(m_flags_ownedby[i] == "") {
      double x = m_flags[i].get_vx();
      double y = m_flags[i].get_vy();
      double dist = hypot(x-curr_vx, y-curr_vy);
      if(dist <= m_flags[i].get_range()) {
	if(result != "")
	  result += ",";
	result += "grabbed=" + m_flags[i].get_label();
	m_flags_ownedby[i] = grabbing_vname;
	m_map_flag_count[up_vname]++;
      }
    }
  }
  if(result == "")
    result = "nothing_grabbed";
  else {
    postFlagSummary();
    postFlagMarkers();
  }

  Notify("FLAG_GRAB_REPORT", result);

  return(true);
}


//------------------------------------------------------------ 
// Procedure: postFlagMarkers
//      Note: Typically JUST called on startup unless marker 
//            positions or colors are allowed to change.

void FlagManager::postFlagMarkers()
{
  for(unsigned int i=0; i<m_flags.size(); i++) {
    XYMarker marker = m_flags[i];
    marker.set_width(2);
    marker.set_type("circle");
    if(m_flags_ownedby[i] == "")
      marker.set_color("primary_color", m_ungrabbed_color);
    else
      marker.set_color("primary_color", m_grabbed_color);
    marker.set_color("secondary_color", "black");
    string spec = marker.get_spec();
    Notify("VIEW_MARKER", spec);
  }
}

//------------------------------------------------------------ 
// Procedure: postFlagSummary

void FlagManager::postFlagSummary()
{
  string summary;
  for(unsigned int i=0; i<m_flags.size(); i++) {
    string spec = "label=" + m_flags[i].get_label();
    spec += ",x=" + doubleToString(m_flags[i].get_vx(),2);
    spec += ",y=" + doubleToString(m_flags[i].get_vy(),2);
    spec += ",range=" + doubleToString(m_flags[i].get_range(),2);
    if(m_flags_ownedby[i] != "")
      spec += ",ownedby=" + m_flags_ownedby[i];
    else
      spec += ",ownedby=none";
    if(summary != "")
      summary += " # ";
    summary += spec;
  }
  Notify("FLAG_SUMMARY", summary);
}

//------------------------------------------------------------
// Procedure: buildReport()

bool FlagManager::buildReport() 
{
  m_msgs << "Node Report Summary"                    << endl;
  m_msgs << "======================================" << endl;
  m_msgs << "        Total Received: " << m_total_node_reports_rcvd << endl;

  map<string, unsigned int>::iterator p;
  for(p=m_map_rcount.begin(); p!=m_map_rcount.end(); p++) {
    string vname = p->first;
    unsigned int total = p->second;
    string pad_vname  = padString(vname, 20);
    m_msgs << "  " << pad_vname << ": " << total;

    double elapsed_time = m_curr_time - m_map_tstamp[vname];
    string stime = "(" + doubleToString(elapsed_time,1) + ")";
    stime = padString(stime,12);
    m_msgs << stime << endl;
  }

  m_msgs << endl << endl;

  ACTable actab(3);
  actab << "VName | Grabs | Flags ";
  actab.addHeaderLines();

  map<string,unsigned int>::iterator p2;
  for(p2=m_map_rcount.begin(); p2!=m_map_rcount.end(); p2++) {
    string vname = p2->first;
    unsigned int grab_count = 0;
    if(m_map_grab_count.count(vname) != 0)
      grab_count = m_map_grab_count[vname];
    unsigned int flag_count = 0;
    if(m_map_flag_count.count(vname) != 0)
      flag_count = m_map_flag_count[vname];

    string s_grab_count = uintToString(grab_count);
    string s_flag_count = uintToString(flag_count);

    actab << vname << s_grab_count << s_flag_count;
  }

  m_msgs << actab.getFormattedString();

  return(true);
}




