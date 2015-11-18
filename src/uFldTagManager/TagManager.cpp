/*****************************************************************/
/*    NAME: Michael Benjamin                                     */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: TagManager.cpp                                       */
/*    DATE: Sep 20th, 2015                                       */
/*                                                               */
/* This program is free software; you can redistribute it and/or */
/* modify it under the terms of the GNU General Public License   */
/* as published by the Free Software Foundation; either version  */
/* 2 of the License, or (at your option) any later version.      */
/*                                                               */
/* This program is distributed in the hope that it will be       */
/* useful, but WITHOUT ANY WARRANTY; without even the implied    */
/* warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR       */
/* PURPOSE. See the GNU General Public License for more details. */
/*                                                               */
/* You should have received a copy of the GNU General Public     */
/* License along with this program; if not, write to the Free    */
/* Software Foundation, Inc., 59 Temple Place - Suite 330,       */
/* Boston, MA 02111-1307, USA.                                   */
/*****************************************************************/

#include <cstdlib>
#include <iostream>
#include <cmath>
#include "AngleUtils.h"
#include "ColorParse.h"
#include "TagManager.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "NodeRecordUtils.h"
#include "XYFormatUtilsPoly.h"

using namespace std;

//------------------------------------------------------------
// Constructor

TagManager::TagManager()
{
  // Default visual hints
  m_post_color = "white";
  m_hit_color  = "red";
  m_miss_color = "green";
  m_vtag_range = 25;

  m_tag_events = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool TagManager::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    
    string key  = msg.GetKey();
    string sval = msg.GetString();

    bool handled = false;
    if((key == "NODE_REPORT") || (key == "NODE_REPORT_LOCAL"))
      handled = handleMailNodeReport(sval);
    else if(key == "TAG_POST")
      handled = handleMailVTagPost(sval);
    else if(key == "APPCAST_REQ")
      handled = true;

    if(!handled)
      reportRunWarning("Unhandled Mail: " + key);
  }
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()

bool TagManager::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.GetConfiguration(GetAppName(), sParams);
  
  STRING_LIST::iterator p;
  for(p = sParams.begin(); p!=sParams.end(); p++) {
    string line  = *p;
    string orig  = line;
    string param = tolower(biteStringX(line, '='));
    string value = line;
    
    bool handled = true;
    if(param == "vtag_range")
      handled = handleConfigVTagRange(value);    
    if(param == "post_color")
      handled = setColorOnString(m_post_color, value);
    else if(param == "hit_color")
      handled = setColorOnString(m_hit_color, value);
    else if(param == "miss_color") 
      handled = setColorOnString(m_miss_color, value);
    else if(param == "zone_one") 
      handled = handleConfigZone(1, value);
    else if(param == "zone_two") 
      handled = handleConfigZone(2, value);
    else if(param == "team_one") 
      handled = handleConfigTeamName(1, value);
    else if(param == "team_two") 
      handled = handleConfigTeamName(2, value);

    if(!handled)
      reportUnhandledConfigWarning("Unhandled config: " + orig);
  }

  postZonePolys();
  registerVariables();
  return(true);
}

//------------------------------------------------------------
// Procedure: Iterate()

bool TagManager::Iterate()
{
  AppCastingMOOSApp::Iterate();
  processVTags();

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool TagManager::OnConnectToServer()
{
  registerVariables();  
  return(true);
}

//------------------------------------------------------------
// Procedure: registerVariables

void TagManager::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("NODE_REPORT", 0);
  Register("NODE_REPORT_LOCAL", 0);
  Register("TAG_POST", 0);
  Register("TAG_STATUS_REQ", 0);
}

//---------------------------------------------------------
// Procedure: handleMailNodeReport
//   Example: NAME=alpha,TYPE=KAYAK,UTC_TIME=1267294386.51,X=29.66,Y=3.9,
//            LAT=43.825089, LON=-70.330030,SPD=2.0, HDG=119.06,
//            YAW=119.05677,DEPTH=0, LENGTH=4.0,MODE=ENGAGED

bool TagManager::handleMailNodeReport(const string& node_report_str)
{
  // Step 1: Deserialize the node record and check validity
  NodeRecord new_node_record = string2NodeRecord(node_report_str);
  if(!new_node_record.valid())
    return(false);

  // Step 2: Add/Update the node record and increment the counter
  string vname = new_node_record.getName();
  m_map_node_records[vname] = new_node_record;
  m_map_node_reports_rcd[vname]++;

  // Step 3: Get the Group/Team name and make sure we know this
  // vehicle and associate it with the proper team.
  string vteam = new_node_record.getGroup();
  if(vteam == "") {
    string msg = "Node report for " + vname + " with no group.";
    reportRunWarning(msg);
    return(false);
  }
  if((vteam != m_team_one) && (vteam != m_team_two)) {
    string msg = "Node report for " + vname + " w/ unknown team: " + vteam;
    reportRunWarning(msg);
    return(false);
  }
  
  m_map_teams[vteam].insert(vname);
  
  return(true);
}

//---------------------------------------------------------
// Procedure: handleMailVTagPost
//   Example: vname=alpha

bool TagManager::handleMailVTagPost(const string& launch_str)
{
  // Part 1: Confirm request is coming from a known vehicle.
  string vname = tokStringParse(launch_str, "vname",  ',', '=');
  if((vname == "") || (m_map_node_records.count(vname) == 0)) {
    string msg = "Failed VTag Post: Unknown vehicle [" + vname + "]";
    Notify("TAG_RESULT_VERBOSE", msg);
    reportRunWarning(msg);
    reportEvent(msg);
    return(false);
  }

  // Part 2: Legal tag request, so increment key counters
  m_tag_events++;
  m_map_node_vtags_requested[vname]++;


  // Part 3: Tag is accepted. Add an appcasting event.
  stringstream ss;
  ss << "Tag requested by " << vname << "[" << m_tag_events << "]";
  reportEvent(ss.str());

  // Part 4: Create a VTag and push it on the list of pending vtags
  // for subsequent processing.
  NodeRecord record = m_map_node_records[vname];
  double     vx = record.getX();
  double     vy = record.getY();
  string     group = record.getGroup();
  VTag vtag(vname, vx, vy, m_curr_time);
  vtag.setEvent(m_tag_events);
  vtag.setVTeam(group);
  m_pending_vtags.push_back(vtag);
    
  // Part 5: Post the RangePulse for the requesting vehicle. This is
  // purely a visual artifact.
  double pulse_duration = 10;
  postRangePulse(vx, vy, m_post_color, vname+"_vtag", 
		 pulse_duration, m_vtag_range);
  
  return(true);
}

//------------------------------------------------------------
// Procedure: postRangePulse

void TagManager::postRangePulse(double x, double y, string color,
				string label, double duration, double radius)
{
  XYRangePulse pulse;
  pulse.set_x(x);
  pulse.set_y(y);
  pulse.set_label(label);
  pulse.set_rad(radius);  
  pulse.set_fill(0.60);
  pulse.set_fill_invariant(true);
  pulse.set_duration(duration);
  pulse.set_linger(2);
  pulse.set_time(m_curr_time);
  if(color != "") {
    pulse.set_color("edge", color);
    pulse.set_color("fill", color);
  }
  string spec = pulse.get_spec();
  Notify("VIEW_RANGE_PULSE", spec);
}


//------------------------------------------------------------
// Procedure: getTrueNodeRange()

double TagManager::getTrueNodeRange(double x, double y, string node)
{
  if(m_map_node_records.count(node) == 0)
    return(-1);

  double node_x = m_map_node_records[node].getX();
  double node_y = m_map_node_records[node].getY();
  double range  = hypot((x-node_x), (y-node_y));

  return(range);
}


//------------------------------------------------------------
// Procedure: handleConfigVTagRange()

bool TagManager::handleConfigVTagRange(string str)
{
  if(!isNumber(str))
    return(false);

  double range = atof(str.c_str());
  if(range < 0)
    return(false);

  m_vtag_range = range;
  return(true);
}


//------------------------------------------------------------
// Procedure: handleConfigZone()
//      Note: Zone number must be 1 or 2. Polygon must be convex
//   Example: pts={0,-20:120,-20:120,-160:0,-160}

bool TagManager::handleConfigZone(int zone_number, string str)
{
  if((zone_number != 1) && (zone_number != 2))
    return(false);

  XYPolygon poly = string2Poly(str);
  if(poly.size() == 0)
    return(false);

  poly.set_edge_size(1);
  poly.set_vertex_size(1);
  poly.set_transparency(0.1);

  if(zone_number == 1) {
    poly.set_color("vertex", "gray50");
    poly.set_color("edge", "gray50");
    poly.set_color("fill", "white");
    if(m_team_one != "")
      poly.set_label(m_team_one);
    else
      poly.set_label("zone_one");
    m_zone_one = poly;
  }
  else {
    poly.set_color("vertex", "gray50");
    poly.set_color("edge", "gray50");
    poly.set_color("fill", "green");
    if(m_team_two != "")
      poly.set_label(m_team_two);
    else
      poly.set_label("zone_two");
    m_zone_two = poly;
  }

  return(true);
}


//------------------------------------------------------------
// Procedure: handleConfigTeamName()

bool TagManager::handleConfigTeamName(int zone_number, string team_name)
{
  // Sanity check - this app only handles TWO zones
  if((zone_number != 1) && (zone_number != 2))
    return(false);
  if(team_name == "")
    return(false);
  
  // Set the m_team_one member variable and apply the label to the
  // polygon representing the zone. Even if the polygon has not been
  // set yet. Now that m_team_one is set, it will be applied when/if
  // the polygon/zone is later configured.
  if((zone_number == 1) && (m_team_one == "")) {
    m_team_one = team_name;
    m_zone_one.set_label(team_name);
  }
  else if((zone_number == 2) && (m_team_two == "")) {
    m_team_two = team_name;
    m_zone_two.set_label(team_name);
  }
  else
    return(false);
  
  return(true);
}


//------------------------------------------------------------
// Procedure: processVTags

void TagManager::processVTags()
{
  list<VTag>::iterator p;
  for(p=m_pending_vtags.begin(); p!=m_pending_vtags.end(); p++) {
    VTag vtag = *p;
    if(vtag.valid())
      processVTag(vtag);
    else
      reportEvent("Invalid vtag: " + vtag.str());
  }
  m_pending_vtags.clear();
}

//------------------------------------------------------------
// Procedure: processVTag()

void TagManager::processVTag(VTag vtag)
{
  double vx = vtag.getX();
  double vy = vtag.getY();
  string vname = vtag.getVName();
  string vteam = vtag.getVTeam();
  string event = uintToString(vtag.getEvent());
  
  // Part 1: Check if tag allowed based on frequency
  // based on the last time it posted a vtag.
  double elapsed = m_curr_time - m_map_node_vtags_last_tag[vname];
  if(elapsed < m_vtag_min_interval) {
    m_map_node_vtags_rejfreq[vname]++;
    string result = "rejected=freq";
    postResult(event, vname, vteam, result);
    return;
  }
  
  // Part 2: Check if tag-target vehicle in zone for tagging
  bool in_own_zone = false;
  if((vteam == m_team_one) && (m_zone_one.contains(vx, vy)))
    in_own_zone = true;
  else if((vteam == m_team_two) && (m_zone_two.contains(vx, vy)))
    in_own_zone = true;

  if(!in_own_zone) {
    m_map_node_vtags_rejzone[vname]++;
    string result = "rejected=zone";
    postResult(event, vname, vteam, result);
    return;
  }
  
  
  // Tag request ok in terms of frequency, zone etc, so declare the
  // tag to be accepted and increment the counter.
  m_map_node_vtags_accepted[vname]++;
  m_map_node_vtags_last_tag[vname] = m_curr_time;
  
  // Part 3: Measure and collect the range to each non-team member
  //         Taking note of the closest target.
  string node_closest;
  map<string, double> map_node_range;
  map<string, NodeRecord>::iterator p;
  for(p=m_map_node_records.begin(); p!=m_map_node_records.end(); p++) {
    string targ_name = p->first;
    string targ_team = p->second.getGroup();
    
    // Disregard members of the same team
    if(targ_team != vteam) {
      double targ_range = getTrueNodeRange(vx, vy, targ_name);
      map_node_range[targ_name] = targ_range;
      if(node_closest == "")
	node_closest = targ_name;
      else if(targ_range < map_node_range[node_closest])
	node_closest = targ_name;
    }
  }
  // Always post the full range results to the verbose variable
  postResult(event, vname, map_node_range);

  // Part 4: Sanity checks. 
  if(map_node_range.size() == 0)
    return;
  if(node_closest == "")
    return;
  if(map_node_range.count(node_closest) == 0)
    return;
  
  // Part 5: Examine the closest target, declare it tagged if in range
  double node_closest_dist = map_node_range[node_closest];
  string result = "tagged=none";
  if(node_closest_dist <= m_vtag_range) {
    result = "tagged=" + node_closest;
    m_map_node_vtags_succeeded[vname]++;
    m_map_node_vtags_beentagged[node_closest]++;
  }

  postResult(event, vname, vteam, result);
}


//------------------------------------------------------------
// Procedure: postResult

void TagManager::postResult(string event, string vname,
			    string vteam, string result)
{
  string msg = "event=" + event;
  msg += ",src=" + vname;
  msg += ",team=" + vteam;
  msg += result;
  
  reportEvent(msg);
  Notify("TAG_RESULT"+toupper(vname), msg);
}
  

//------------------------------------------------------------
// Procedure: postResult

void TagManager::postResult(string event, string vname,
			    map<string, double> map_node_range)
{
  string msg = "event=" + event;
  msg += ",src=" + vname;
  msg += ",ranges=";
  
  map<string,double>::iterator p;
  for(p=map_node_range.begin(); p!=map_node_range.end(); p++) {
    string targ = p->first;
    double dist = p->second;
    if(p != map_node_range.begin())
      msg += "#";
    msg += targ + ":" + doubleToString(dist,1);
  }
  Notify("TAG_RESULT_VERBOSE", msg);
}
  
//------------------------------------------------------------
// Procedure: postZonePolys

void TagManager::postZonePolys()
{
  string spec_one = m_zone_one.get_spec();
  string spec_two = m_zone_two.get_spec();

  Notify("VIEW_POLYGON", spec_one);
  Notify("VIEW_POLYGON", spec_two);
}
  


//------------------------------------------------------------
// Procedure: buildReport()
//   
//   Global Settings
//   ======================================================
//   Tag Range: 35
//   Tag Interval: 10
//   Team [red]: lou, mal, ned, opi (4)
//   Team [blue]: lima, mesa, nuuk, oslo (4)
//
//   Tag Application Stats
//   ======================================================
//             ReQ   Rejec  Rejec            Applied  Time
//   Name      Tags  Zone   Freq   Accepted     Tags  Next 
//   --------  ----  -----  -----  --------   ------   ----
//   lou        5        1      3        1         1   n/a
//   mal        8        2      1        5         0    23
//   ned        23      11      2       10         4     4
//   opi        18      2       9        7         1   n/a
//   --------  ----   ----  -----  --------   ------   ----
//   lima       2        0      1        1         1   n/a
//   mesa       4        2      1        1         0    23
//   nuuk       11      11      0        0         0   n/a
//   oslo       1        0      0        1         1    44
//
//   Tag Receiver Stats
//   ======================================================
//             Times   Currently  Time    
//   Name      Tagged  Tagged     Remain   Taggable
//   --------  ------  ---------  ------   ---------   
//   lou            2         no     n/a       no 
//   mal            1        YES      14      YES 
//   ned            1         no     n/a      YES 
//   opi            0         no     n/a      YES 
//   --------  ------  ---------  ------   ---------   
//   lima           0         no     n/a       no 
//   mesa           1        YES      58      YES 
//   nuuk           1        YES       7      YES 
//   oslo           2         no     n/a      YES 
//
//   Events (Last 8):
//   ====================================================
//   201.0   Tag Attempt by HENRY [27]
//   201.0   Tag Attempt by HENRY [27]
//   201.0       BETTY tagged by ARCHIE
//   201.0   Tag Attempt by ARCHIE [23]
//   .....
//   198.2   Failed VTag Post: Unknown vehicle [jake]

bool TagManager::buildReport()
{
  // Part 1: Build the easy global settings output
  m_msgs << "Global Settings            " << endl;
  m_msgs << "===========================" << endl;
  m_msgs << "Tag Range:    " << doubleToStringX(m_vtag_range,1)   << endl;
  m_msgs << "Tag Interval: " << doubleToStringX(m_vtag_min_interval,1) << endl;

  // Part 2: Produce the team structure
  map<string, set<string> >::iterator pp;
  for(pp=m_map_teams.begin(); pp!=m_map_teams.end(); pp++) {
    string team_name = pp->first;
    set<string> team = pp->second;
    m_msgs << "Team [" << team_name << "]: ";
    set<string>::iterator qq;
    for(qq=team.begin(); qq!=team.end(); qq++) {
      if(qq!=team.begin())
	m_msgs << ", ";
      m_msgs << *qq;
    }
    m_msgs << " (" << team.size() << ")" << endl;
  }
  m_msgs << endl;

  // Part 3: Build report from perspective of tagging vehicles
  m_msgs << "Tag Application Stats:         " << endl;
  m_msgs << "=================================================" << endl;
  ACTable actab(7);
  actab << "     | ReQ  | Rejec | Rejec |          | Applied | Time ";
  actab << "Name | Tags | Zone  | Freq  | Accepted |    Tags | Next";
  actab.addHeaderLines();

  map<string, NodeRecord>::iterator p1;
  for(p1=m_map_node_records.begin(); p1!=m_map_node_records.end(); p1++) {
    string vname = p1->first;  // col1
    string tags = uintToString(m_map_node_vtags_requested[vname]);   // col2
    string zone = uintToString(m_map_node_vtags_rejzone[vname]);     // col3
    string freq = uintToString(m_map_node_vtags_rejfreq[vname]);     // col4
    string accp = uintToString(m_map_node_vtags_accepted[vname]);    // col5
    string hits = uintToString(m_map_node_vtags_succeeded[vname]);   // col6
    string next = "n/a";  // col7
    
    actab << vname << tags << zone << freq << accp << hits << next;
  }
  m_msgs << actab.getFormattedString();
  m_msgs << endl << endl;

  // Part 4: Build report from perspective of vehicles being tagged.
  m_msgs << "Tag Receiver Stats:         " << endl;
  m_msgs << "=================================================" << endl;
  ACTable actabb(5);
  actabb << "     | Times  | Currently | Time   |          ";
  actabb << "Name | Tagged | Tagged    | Remain | Taggable ";
  actabb.addHeaderLines();


  for(pp=m_map_teams.begin(); pp!=m_map_teams.end(); pp++) {
    if(pp!=m_map_teams.begin())
      actabb.addHeaderLines();
    set<string> team = pp->second;
    set<string>::iterator qq;
    for(qq=team.begin(); qq!=team.end(); qq++) {
      string vname = *qq;
      string times = uintToString(m_map_node_vtags_beentagged[vname]);   // col2
      string tamt  = "no";
      string trem  = "n/a";
      string tgabl = "YES"; 
      actabb << vname << times << tamt << trem << tgabl;
    }
  }

  m_msgs << actabb.getFormattedString();
  return(true);
}

