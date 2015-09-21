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
#include "TagManager.h"
#include "MBUtils.h"
#include "ACTable.h"
#include "NodeRecordUtils.h"

using namespace std;

//------------------------------------------------------------
// Constructor

TagManager::TagManager()
{
  // Default visual hints
  m_hit_color  = "white";
  m_miss_color = "red";

  m_vtag_range = 50;
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
      handled = handleNodeReport(sval);
    else if(key == "VTAG_LAUNCH")
      handled = handleVTagLaunch(sval);
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
      handled = setVTagRange(value);

    if(param == "hit_color")
      handled = setColorOnString(m_hit_color, value);
    else if(param == "miss_color") 
      handled = setColorOnString(m_miss_color, value);

    else if(param == "visual_hints") 
      handled = handleVisualHints(value);

    if(!handled)
      reportUnhandledConfigWarning("Unhandled config: " + orig);
  }

  registerVariables();
  return(true);
}

//------------------------------------------------------------
// Procedure: Iterate()

bool TagManager::Iterate()
{
  AppCastingMOOSApp::Iterate();
  processPendingVTags();

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
  Register("VTAG_LAUNCH", 0);
  Register("VTAG_STATUS_REQ", 0);
}

//---------------------------------------------------------
// Procedure: handleNodeReport
//   Example: NAME=alpha,TYPE=KAYAK,UTC_TIME=1267294386.51,
//            X=29.66,Y=-23.49,LAT=43.825089, LON=-70.330030,
//            SPD=2.00, HDG=119.06,YAW=119.05677,DEPTH=0.00,   
//            LENGTH=4.0,MODE=ENGAGED

bool TagManager::handleNodeReport(const string& node_report_str)
{
  // Step 1: Deserialize the node record and check validity
  NodeRecord new_node_record = string2NodeRecord(node_report_str);
  if(!new_node_record.valid())
    return(false);

  // Step 2: Add/Update the node record and increment the counter
  string vname = new_node_record.getName();
  m_map_node_records[vname] = new_node_record;
  // No promise that m_map_node_reports_rcd[vname] will equal 0 when it is created
  //  Therefore we check if it exists, if not we create it and set it to 0
  if (m_map_node_reports_rcd.count(vname) == 0)
    m_map_node_reports_rcd[vname] = 0;
  m_map_node_reports_rcd[vname]++;

  return(true);
}

//---------------------------------------------------------
// Procedure: handleVTagLaunch
//   Example: vname=alpha

bool TagManager::handleVTagLaunch(const string& launch_str)
{
  string vname = tokStringParse(launch_str, "name",  ',', '=');
  
  // Part 1: Error checking
  // Confirm this request is coming from a known vehicle.
  if((vname == "")  || (m_map_node_records.count(vname) == 0)) {
    reportRunWarning("Failed VTag Post: Unknown vehicle[" + vname + "]");
    reportEvent("Failed VTag: Unknown vehicle[" + vname + "]");
    return(false);
  }

  // Increment the counter of requested tags made by this vehicle.
  m_map_node_vtags_requested[vname]++;
  
  // Part 2: Determine if this vehicle is allowed to launch a vtag
  // based on the last time it posted a vtag.
  double elapsed = m_curr_time - m_map_node_vtag_last[vname];
  if(elapsed < m_vtag_min_interval) {
    m_map_node_vtags_rej_2freq[vname]++;
    return(false);
  }
    
  
  
#if 0


  unsigned int amt_remaining = getChargesRemaining(vname);
  bool launch_allowed = (amt_remaining > 0);
  
  if(!launch_allowed) {
    string msg = toupper(vname) + "  ----[ooo] No More Depth Charges! ";
    reportEvent(msg);
    return(true);
  }

  m_map_node_launches_now[vname]++;
  m_map_node_launches_ever[vname]++;
  reportEvent(toupper(vname) + "  ----[---] Depth Charge deployed.... ");

  if(amt_remaining > 0)
    m_map_node_charges_have[vname]--;
  
  // Part 3: Create a new Depth Charge
  NodeRecord record = m_map_node_records[vname];
  double     dcharge_range = m_map_node_charge_range[vname];
  double     vx = record.getX();
  double     vy = record.getY();

  DepthCharge depth_charge;
  depth_charge.setVName(vname);
  depth_charge.setX(vx);
  depth_charge.setY(vy);
  depth_charge.setTimeLaunched(m_curr_time);
  depth_charge.setTimeDelay(delay_dval);
  depth_charge.setRange(dcharge_range);
  m_pending_charges.push_back(depth_charge);


  // Part 4: Post the RangePulse for the requesting vehicle. This is
  // purely a visual artifact.
  double pulse_duration = 10;
  double linger = delay_dval - pulse_duration;
  if(linger < 0)
    linger = 0;
  postRangePulse(vx, vy, m_hit_color, vname+"_dcharge", 
     pulse_duration, dcharge_range, linger);

#endif
  return(true);
}

//------------------------------------------------------------
// Procedure: postRangePulse

void TagManager::postRangePulse(double x, double y, const string& color,
           const string& label, double duration,
           double radius, double linger)
{
#if 0
  XYRangePulse pulse;
  pulse.set_x(x);
  pulse.set_y(y);
  pulse.set_label(label);
  pulse.set_rad(radius);  
  pulse.set_fill(0.60);
  pulse.set_fill_invariant(true);
  pulse.set_duration(duration);
  pulse.set_linger(linger);
  pulse.set_time(m_curr_time);
  if(color != "") {
    pulse.set_color("edge", color);
    pulse.set_color("fill", color);
  }
  string spec = pulse.get_spec();
  Notify("VIEW_RANGE_PULSE", spec);
#endif
}


//------------------------------------------------------------
// Procedure: getTrueNodeRange()

double TagManager::getTrueNodeRange(double x, double y, const string& node)
{
  if(m_map_node_records.count(node) == 0)
    return(-1);

  double node_x = m_map_node_records[node].getX();
  double node_y = m_map_node_records[node].getY();
  double range  = hypot((x-node_x), (y-node_y));

  return(range);
}


//------------------------------------------------------------
// Procedure: setVTagConfig

bool TagManager::setVTagConfig(string str)
{
  return(true);
}

//------------------------------------------------------------
// Procedure: setDefaultVTagRange
//      Note: Argument must be a non-negative number
//   Example: "50"

bool TagManager::setVTagRange(string str)
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
// Procedure: processPendingVTags

void TagManager::processPendingVTags()
{
#if 0
  list<VTag>::iterator p;
  for(p=m_pending_vtags.begin(); p!=m_pending_vtags.end(); ) {
    VTag vtag = *p;
    double elapsed = m_curr_time - dcharge.getTimeLaunched(); 
    if(elapsed >= dcharge.getTimeDelay()) {
      invokeDepthCharge(dcharge);
      p = m_pending_charges.erase(p);
    }
    else
      ++p;
  }
#endif
}

//------------------------------------------------------------
// Procedure: handleVisualHints

bool TagManager::handleVisualHints(string str)
{
  vector<string> svector = parseString(str, ',');
  for(unsigned int i=0; i<svector.size(); i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];
    
    if((param == "hit_color") && isColor(value))
      m_hit_color = value;
    else if((param == "miss_color") && isColor(value))
      m_miss_color = value;
    else
      return(false);
  }
  return(true);
}

//------------------------------------------------------------
// Procedure: invokeVTag()

void TagManager::invokeVTag(VTag vtag)
{
#if 0
  double dcharge_x = dcharge.getX();
  double dcharge_y = dcharge.getY();
  double dcharge_range = dcharge.getRange();
  string dcharge_vname = dcharge.getVName();
  
  if(m_map_node_launches_now[dcharge_vname] > 0)
    m_map_node_launches_now[dcharge_vname]--;

  double pulse_duration = 1;
  double linger = 20;
  postRangePulse(dcharge_x, dcharge_y, m_hit_color, dcharge_vname+"_dchargex", 
		 pulse_duration, dcharge_range, linger);
  
  
  reportEvent(toupper(dcharge_vname) + "  ----[***] Depth Charge Detonated.....");
  
  // Check all vehicles to see if they are in range of the depth charge
  map<string, NodeRecord>::iterator p;
  for (p = m_map_node_records.begin(); p != m_map_node_records.end(); p++) {
    string node_name  = p->first;
    
    // A vehicle cannot be a victim of its own depth charge
    if (node_name != dcharge_vname) {
      double node_range = getTrueNodeRange(dcharge_x, dcharge_y, node_name);
      
      // A depth charge hit!!!!
      if (node_range < dcharge_range) {
	m_map_node_launches_hit[dcharge_vname]++;
	
	// Declare the hit to the MOOSDB (outside the scope here as to the
	// consequences of the hit. Here we just declare it.)
	string msg = "target=" + node_name + ",range=" +
	  doubleToString(node_range, 1);
	Notify("DEPTH_CHARGE_HIT", msg);
	Notify("TARGET_HIT_ALL", node_name);
	
	// We don't remove the hit vehicle from the records, but we do
	// zero-out any depth charges that vehicle may have had.
	m_map_node_charges_have[node_name] = 0;
	
	reportEvent(toupper(dcharge_vname) +
		    "  ----[***] Depth Charge HIT: " + node_name); }
      
      // A depth charge miss...
      else {
	// Declare the miss to the MOOSDB
	string msg = "target=" + node_name + ",range =" + doubleToString(node_range, 1);
	Notify("DEPTH_CHARGE_MISS", msg);
	reportEvent(toupper(dcharge_vname) + "  ----[***] Depth Charge MISS");
      }
    }
  }
#endif
}

//------------------------------------------------------------
// Procedure: buildReport()
//   
//   Depth Charge Inventory
//   ======================================================
//                              Ever       Presently
//   Name       Amt    Range    Deployed   Deployed    Hits
//   --------   -----  -----    --------   ---------   ----
//   archie     2      35       11         1           1
//   betty      1      35       7          0           3
//   jackal     0      0        0          0           0

//   
//   Pending Depth Charges
//   =====================================================
//            Time to 
//   Source   Destination
//
//
//   Events (Last 8):
//   ====================================================
//   201.0   CHARLIE ----[---] Depth Charge HIT: jackal
//   201.0   CHARLIE ----[***] Depth Charge Detonated 

//   201.0   CHARLIE ----[---] Depth Charge Deployed
//   201.0   CHARLIE ----[---] Depth Charge Deployed
//   201.0   CHARLIE ----[ooo] No More Depth Charges!
//   .....
//   198.2   BETTY   ----)) 
//   198.2   DAVIS   <--     BETTY
//   198.2   CHARLIE <--     BETTY


bool TagManager::buildReport()
{
#if 0
  m_msgs << "Inventory Configuration Settings " << endl;
  m_msgs << "  Default Charge Range:  " << m_depth_charge_range_default  << endl;
  m_msgs << "  Default Charge Amount: " << m_depth_charge_amount_default << endl;
  m_msgs << "Delay Configuration Settings " << endl;
  m_msgs << "  Depth Charge Delay Max:     " << m_depth_charge_delay_max << endl;
  m_msgs << "Replenish Configuration Settings " << endl;
  m_msgs << "  Replenish range:   " << m_replenish_range << endl;
  m_msgs << "  Replenish time:    " << m_replenish_time  << endl;
  m_msgs << "Visual Hints " << endl;
  m_msgs << "  Miss Color: " << m_detonate_color  << endl;
  m_msgs << "  Hit Color:      " << m_hit_color       << endl;
  m_msgs << endl;


  // Part 2: Build a report on the Vehicles
  m_msgs << "Depth Charge Inventory" << endl;
  m_msgs << "=================================================" << endl;
  ACTable actab(6);
  actab << "     |      |       | Ever     | Presently |      ";
  actab << "Name | Amt  | Range | Deployed | Deployed  | Hits ";
  actab.addHeaderLines();

  map<string, NodeRecord>::iterator p;
  for(p=m_map_node_records.begin(); p!=m_map_node_records.end(); p++) {
    string vname = p->first;
    
    string chrgs = uintToString(m_map_node_charges_have[vname]);
    string range = doubleToStringX(m_map_node_charge_range[vname]);
    string launches_ever = uintToString(m_map_node_launches_ever[vname]);
    string launches_now  = uintToString(m_map_node_launches_now[vname]);
    string launches_hit  = uintToString(m_map_node_launches_hit[vname]);
    
    actab << vname << chrgs << range << launches_ever << launches_now
	  << launches_hit;
  }
  m_msgs << actab.getFormattedString();
#endif
  return(true);
}
