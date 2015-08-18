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

using namespace std;

//---------------------------------------------------------
// Constructor

FlagManager::FlagManager()
{
  m_default_grab_dist = 10; // meters

  m_total_reports_rcvd    = 0;
  m_total_grabs_rcvd      = 0;
  m_report_flags_on_start = true;
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
  
  registerVariables();	


  postFlagMarkers();
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

bool FlagManager::handleConfigFlag(string str)
{
  double x = 0;
  double y = 0;
  double grab_dist = m_default_grab_dist;
  string label;
  
  vector<string> svector = parseString(str, ',');
  for(unsigned int i=0; i<svector.size(); i++) {
    string param = tolower(biteStringX(svector[i], '='));
    string value = svector[i];
    if((param == "x") && isNumber(value))
      x = atof(value.c_str());
    else if((param == "y") && isNumber(value))
      y = atof(value.c_str());
    else if((param == "grab_dist") && isNumber(value))
      grab_dist = atof(value.c_str());
    else if(param == "label")
      label = value;
  }
  
  // Ensure that a non-empty label has been provided
  if(label == "") {
    reportConfigWarning("Flag with missing label: " + str);
    return(false);
  }

  // Ensure that a unique label has been provided
  for(unsigned int j=0; j<m_flags_label.size(); j++) {
    if(label == m_flags_label[j]) {
      reportConfigWarning("Flag with duplicate label: " + str);
      return(false);
    }
  }
    
  m_flags_x.push_back(x);
  m_flags_y.push_back(y);
  m_flags_grab_dist.push_back(grab_dist);
  m_flags_label.push_back(label);
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

  m_total_reports_rcvd++;

  return(true);
}

//------------------------------------------------------------ 
// Procedure: handleMailFlagGrab
//   Example: GRAB_FLAG_REQUEST = "vname=henry"

bool FlagManager::handleMailFlagGrab(string str, string community)
{
  string grabbing_vname;

  vector<string> svector = parseString(str, ',');
  for(unsigned int i=0; i<svector.size(); i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];

    if(param == "vname") {
      string vname = tolower(value);
      if(community == vname)
	grabbing_vname = vname;
    }
  }

  // If the grabbing vehicle is not set, return false
  if(grabbing_vname == "")
    return(false);

  // If no node records of the grabbing vehicle, return false
  string up_vname = toupper(grabbing_vname);
  if(m_map_record.count(up_vname) == 0)
    return(false);

  m_map_grab_count[up_vname]++;
  m_total_grabs_rcvd++;

  // Get the grabbing vehicle's position from the record
  NodeRecord record = m_map_record[up_vname];
  double curr_vx = record.getX();
  double curr_vy = record.getY();

  // For each flag with the grab_dist of the vehicle, GRAB
  string result;
  for(unsigned int i=0; i<m_flags_x.size(); i++) {
    if(m_flags_ownedby[i] == "") {
      double x = m_flags_x[i];
      double y = m_flags_y[i];
      double dist = hypot(x-curr_vx, y-curr_vy);
      if(dist <= m_flags_grab_dist[i]) {
	if(result != "")
	  result += ",";
	result += "grabbed=" + m_flags_label[i];
	m_flags_ownedby[i] = grabbing_vname;
	m_map_flag_count[up_vname]++;
      }
    }
  }
  if(result == "")
    result = "nothing_grabbed";

  Notify("FLAG_GRAB_REPORT", result);

  return(true);
}

//------------------------------------------------------------ 
// Procedure: postFlagMarkers

void FlagManager::postFlagMarkers()
{
  for(unsigned int i=0; i<m_flags_x.size(); i++) {
    XYMarker new_marker;
    new_marker.set_vx(m_flags_x[i]);
    new_marker.set_vy(m_flags_y[i]);
    new_marker.set_label(m_flags_label[i]);
    new_marker.set_width(2);
    new_marker.set_type("circle");
    new_marker.set_color("fill_color", "red");
    new_marker.set_color("edge_color", "black");
    string spec = new_marker.get_spec();
    Notify("VIEW_MARKER", spec);
  }
}

//------------------------------------------------------------
// Procedure: buildReport()

bool FlagManager::buildReport() 
{
  m_msgs << "Node Report Summary"                    << endl;
  m_msgs << "======================================" << endl;
  m_msgs << "        Total Received: " << m_total_reports_rcvd << endl;

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




