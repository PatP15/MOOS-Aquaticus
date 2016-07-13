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
  m_default_flag_width    = 3;  // meters
  m_default_flag_type     = "circle";
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
    if((key == "NODE_REPORT") || (key == "NODE_REPORT_LOCAL"))
      handled = handleMailNodeReport(sval);
    else if(key == "FLAG_RESET")
      handled = handleMailFlagReset(sval);
    else if(key == "FLAG_GRAB_REQUEST")
      handled = handleMailFlagGrab(sval, comm);
    else if(key == "TAGGED_VEHICLES")
      handled = handleMailTaggedVehicles(sval);
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

bool FlagManager::Iterate()
{
  AppCastingMOOSApp::Iterate();
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

  STRING_LIST pass2params;
  STRING_LIST::iterator p;
  for(p=sParams.begin(); p!=sParams.end(); p++) {
    string orig  = *p;
    string line  = *p;
    string param = tolower(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if((param == "grabbed_color") && isColor(value)) {
      m_grabbed_color = value;
      handled = true;
    }
    else if((param == "ungrabbed_color") && isColor(value)) {
      m_ungrabbed_color = value;
      handled = true;
    }
    else if((param == "default_flag_range") && isNumber(value)) {
      double dval = atof(value.c_str());
      if(dval >= 0) {
        m_default_flag_range = dval;
        handled = true;
      }
    }
    else if((param == "default_flag_width") && isNumber(value)) {
      double dval = atof(value.c_str());
      if(dval >= 0) {
        m_default_flag_width = dval;
        handled = true;
      }
    }
    else if(param == "default_flag_type") {
      value = tolower(value);
      if((value == "circle")  || (value == "square") ||
         (value == "diamond") || (value == "efield") ||
         (value == "gateway") || (value == "triangle")) {
        m_default_flag_type = value;
        handled = true;
      }
    }
    else
      pass2params.push_back(orig);
  }

  STRING_LIST::iterator p2;
  for(p2=pass2params.begin(); p2!=pass2params.end(); p2++) {
    string orig  = *p2;
    string line  = *p2;
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
  Register("FLAG_RESET", 0);
  Register("NODE_REPORT", 0);
  Register("NODE_REPORT_LOCAL", 0);
  Register("TAGGED_VEHICLES", 0);
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

  if(!flag.is_set_width())
    flag.set_width(m_default_flag_width);

  if(!flag.is_set_type())
    flag.set_type(m_default_flag_type);

  // Ensure that a unique label has been provided
  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(flag.get_label() == m_flags[i].get_label()) {
      reportConfigWarning("Flag with duplicate label: " + str);
      return(false);
    }
  }

  m_flags.push_back(flag);
  m_flags_changed.push_back(true);

  return(true);
}

//------------------------------------------------------------
// Procedure: handleMailTaggedVehicles()

bool FlagManager::handleMailTaggedVehicles(string str)
{
  m_tagged_vnames.clear();

  vector<string> svector = parseString(str, ',');
  for(unsigned int i=0; i<svector.size(); i++) {
    string vname = stripBlankEnds(svector[i]);
    m_tagged_vnames.insert(vname);
  }

  return(true);
}

//------------------------------------------------------------
// Procedure: handleMailNodeReport()

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
// Procedure: handleMailFlagReset()
//   Example: FLAG_RESET = vname=henry
//   Example: FLAG_RESET = label=alpha

bool FlagManager::handleMailFlagReset(string str)
{
  bool some_flags_were_reset = false;

  if(tolower(str) == "all") {
    some_flags_were_reset = resetFlagsAll();
  }
  else {
    string vname = tokStringParse(str, "vname", ',', '=');
    if(vname == "")
      vname = tokStringParse(str, "VNAME", ',', '=');
    if(vname != "")
      some_flags_were_reset = resetFlagsByVName(vname);

    string label = tokStringParse(str, "label", ',', '=');
    if(label != "")
      some_flags_were_reset = resetFlagsByLabel(label);

    if((label == "") && (vname == ""))
      return(false);
  }

  if(some_flags_were_reset) {
    postFlagMarkers();
    postFlagSummary();
  }

  return(true);
}

//------------------------------------------------------------
// Procedure: handleMailFlagGrab
//   Example: GRAB_FLAG_REQUEST = "vname=henry"

bool FlagManager::handleMailFlagGrab(string str, string community)
{
  m_total_grab_requests_rcvd++;

  // Part 1: Parse the Grab Request
  string grabbing_vname = tokStringParse(str, "vname", ',', '=');

  // Part 2: Sanity check on the Grab Request
  // Check if grabbing vname is set and matches message community
  if((grabbing_vname == "") || (grabbing_vname != community)) {
    return(false);
  }

  // If no node records of the grabbing vehicle, return false
  string up_vname = toupper(grabbing_vname);
  if(m_map_record.count(up_vname) == 0)
    return(false);


  // Part 3: OK grab, so increment counters.
  m_map_grab_count[up_vname]++;

  if(m_tagged_vnames.count(grabbing_vname) ||  m_tagged_vnames.count(up_vname))
    return(false);
    
  // Part 4: Get the grabbing vehicle's position from the record
  NodeRecord record = m_map_record[up_vname];
  double curr_vx = record.getX();
  double curr_vy = record.getY();
  string group = record.getGroup();

  // Part 5: For each flag with the grab_dist of the vehicle, GRAB
  string result;
  for(unsigned int i=0; i<m_flags.size(); i++) {
    if((m_flags[i].get_owner() == "") && (m_flags[i].get_label() != group)) {
      double x = m_flags[i].get_vx();
      double y = m_flags[i].get_vy();
      double dist = hypot(x-curr_vx, y-curr_vy);

      if(dist <= m_flags[i].get_range()) {
        if(result != "")
          result += ",";
        result += "grabbed=" + m_flags[i].get_label();
        m_flags[i].set_owner(grabbing_vname);
        m_flags_changed[i] = true;
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
// Procedure: resetFlagsByLabel
//      Note: Resets any flag with the given label to be not
//            ownedby anyone.
//   Returns: true if a flag was indeed reset, possibly visuals
//            then need updating

bool FlagManager::resetFlagsByLabel(string label)
{
  bool some_flags_were_reset = false;

  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(m_flags[i].get_label() == label) {
      if(m_flags[i].get_owner() != "") {
        m_flags[i].set_owner("");
        m_flags_changed[i] = true;
        some_flags_were_reset = true;
      }
    }
  }
  return(some_flags_were_reset);
}

//------------------------------------------------------------
// Procedure: resetFlagsAll
//      Note: Resets all flags regardless of who owned them
//   Returns: true if a flag was indeed reset, possibly visuals
//            then need updating

bool FlagManager::resetFlagsAll()
{
  bool some_flags_were_reset = false;

  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(m_flags[i].get_owner() != "") {
      m_flags[i].set_owner("");
      m_flags_changed[i] = true;
      some_flags_were_reset = true;
    }
  }
  return(some_flags_were_reset);
}

//------------------------------------------------------------
// Procedure: resetFlagsByVName
//      Note: Resets any flag presently owned by the given vehicle
//   Returns: true if a flag was indeed reset, possibly visuals
//            then need updating

bool FlagManager::resetFlagsByVName(string vname)
{
  bool some_flags_were_reset = false;

  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(m_flags[i].get_owner() == vname) {
      m_flags[i].set_owner("");
      m_flags_changed[i] = true;
      some_flags_were_reset = true;
    }
  }
  return(some_flags_were_reset);
}

//------------------------------------------------------------
// Procedure: postFlagMarkers
//      Note: Typically JUST called on startup unless marker
//            positions or colors are allowed to change.

void FlagManager::postFlagMarkers()
{
  for(unsigned int i=0; i<m_flags.size(); i++) {
    if(m_flags_changed[i]) {
      XYMarker marker = m_flags[i];
      if(m_flags[i].get_owner() == "") {
        if(!m_flags[i].color_set("primary_color"))
          marker.set_color("primary_color", m_ungrabbed_color);
      }
      else
        marker.set_color("primary_color", m_grabbed_color);
      marker.set_color("secondary_color", "black");

      string spec = marker.get_spec();
      Notify("VIEW_MARKER", spec);
      m_flags_changed[i] = false;
    }
  }
}

//------------------------------------------------------------
// Procedure: postFlagSummary

void FlagManager::postFlagSummary()
{
  string summary;
  for(unsigned int i=0; i<m_flags.size(); i++) {
    string spec = m_flags[i].get_spec();
    if(summary != "")
      summary += " # ";
    summary += spec;

    string var_label = toupper(m_flags[i].get_label());
    var_label += "_FLAG_GRABBED";
    if (m_flags[i].get_owner() == "")
      Notify(var_label, "false");
    else
      Notify(var_label, "true");
  }
  Notify("FLAG_SUMMARY", summary);


}

//------------------------------------------------------------
// Procedure: buildReport()

bool FlagManager::buildReport()
{

  m_msgs << "Configuration Summary: " << endl;
  m_msgs << "======================================" << endl;
  m_msgs << "  default_flag_range: " << m_default_flag_range << endl;
  m_msgs << "  default_flag_width: " << m_default_flag_width << endl;
  m_msgs << "  default_flag_type:  " << m_default_flag_type  << endl;
  m_msgs << endl;

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

  m_msgs << endl;

  m_msgs << "Vehicle Summary" << endl;
  m_msgs << "======================================" << endl;
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
  m_msgs << endl << endl;

  m_msgs << "Flag Summary" << endl;
  m_msgs << "======================================" << endl;
  actab = ACTable(4);
  actab << "Flag | Range | Owner | Spec";
  actab.addHeaderLines();

  for(unsigned int i=0; i<m_flags.size(); i++) {
    string label = m_flags[i].get_label();
    string vname = m_flags[i].get_owner();
    string range = doubleToStringX(m_flags[i].get_range(), 2);
    actab << label << range << vname << m_flags[i].get_spec();
  }
  m_msgs << actab.getFormattedString();

  return(true);
}
