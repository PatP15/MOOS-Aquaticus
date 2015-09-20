/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
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
  // Default depth charge range and amount for all vehicles
  m_depth_charge_range_default  = 25; // meters
  m_depth_charge_amount_default = 5;

  // Default visual hints
  m_drop_color     = "blue";
  m_detonate_color = "white";
  m_hit_color      = "red";

  // Default Replenish Position, Range and Time
  m_replenish_x = 0;
  m_replenish_y = 0;
  m_replenish_range = 50;
  m_replenish_time = 30;

  // Depth charge delay parameters
  m_depth_charge_delay_max = 60;
  m_depth_charge_delay_default = 15;
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
    else if(key == "DEPTH_CHARGE_LAUNCH")
      handled = handleDepthChargeLaunch(sval);
    else if(key == "DEPTH_CHARGE_STATUS_REQ")
      handled = handleDepthChargeStatusRequest(sval);
    else if(key == "DEPTH_CHARGE_REPLENISH_REQ")
      handled = handleReplenishRequest(sval);
    else if(key == "REPLENISH_CLARIFY")
      handled = postReplenishRules();
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
    string sLine  = *p;
    string origl  = *p;
    string param  = tolower(biteStringX(sLine, '='));
    string value  = sLine;
    
    bool handled = true;
    if(param == "depth_charge_config")
      handled = setDepthChargeConfig(value);
    else if((param == "depth_charge_range_default") || 
	    (param == "default_depth_charge_range"))
      handled = setDefaultChargeRange(value);
    else if((param == "default_depth_charge_amount") ||
	    (param == "depth_charge_amount_default"))
      handled = setDefaultChargeAmount(value);
    else if(param == "depth_charge_delay_default")
      handled = setDepthChargeDelayDefault(value);
    else if(param == "depth_charge_delay_max")
      handled = setDepthChargeDelayMax(value);


    else if(param == "replenish_time")
      handled = setReplenishTime(value);
    else if(param == "replenish_range")
      handled = setReplenishRange(value);
    else if(param == "replenish_station")
      handled = setReplenishStation(value);
    else if((param == "drop_color") && isColor(value)) {
      m_drop_color = value;
      handled = true;
    }
    else if((param == "hit_color") && isColor(value)) {
      m_hit_color = value;
      handled = true;
    }
    else if(param == "visual_hints") 
      handled = handleVisualHints(value);

    if(!handled)
      reportUnhandledConfigWarning("Unhandled config: " + origl);
  }

  registerVariables();
  return(true);
}

//------------------------------------------------------------
// Procedure: Iterate()

bool TagManager::Iterate()
{
  AppCastingMOOSApp::Iterate();
  processPendingDepthCharges();

  // Check for Replenish Requests
  map<string, NodeRecord>::iterator p;
  for(p=m_map_node_records.begin(); p!=m_map_node_records.end(); p++) {
    string vname = p->first;
    if(m_map_node_replenish_req[vname] == true)
      postReplenishStatus(vname);
  }

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
  Register("DEPTH_CHARGE_LAUNCH", 0);
  Register("DEPTH_CHARGE_REPLENISH_REQ", 0);
  Register("DEPTH_CHARGE_STATUS_REQ", 0);
  Register("REPLENISH_CLARIFY", 0);
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

  // Step 3: If number of node depth charges are not pre-config, set to node default
  if(m_map_node_charges_have.count(vname) == 0)
    if (isUUV(vname))
      m_map_node_charges_have[vname] = 0;
    else
      m_map_node_charges_have[vname] = m_depth_charge_amount_default;

  // Step 4: If node depth charge range not pre-config, set to node default
  if(m_map_node_charge_range.count(vname) == 0)
    if (isUUV(vname))
      m_map_node_charge_range[vname] = 0;
    else
      m_map_node_charge_range[vname] = m_depth_charge_range_default;
  
  // Step 5: If node replenish request not pre-config, set to false
  if (m_map_node_replenish_req.count(vname) == 0) {
    m_map_node_replenish_req[vname] = false;
  }
  
  return(true);
}

//---------------------------------------------------------
// Procedure: handleDepthChargeLaunch
//   Example: vname=alpha,delay=30

bool TagManager::handleDepthChargeLaunch(const string& launch_str)
{
  string vname = tokStringParse(launch_str, "name",  ',', '=');
  string delay = tokStringParse(launch_str, "delay", ',', '=');
  
  // Part 1: Error checking
  // Confirm this request is coming from a known vehicle.
  if((vname == "")  || (m_map_node_records.count(vname) == 0)) {
    reportRunWarning("Failed DepthCharge Launch: Unknown vehicle["+vname+"]");
    reportEvent("Failed Range Request: Unknown vehicle["+vname+"]");
    return(false);
  }

  // Convert the string delay value to a numerical value
  double delay_dval = atof(delay.c_str());
  if(!isNumber(delay) || (delay_dval < 0)) {
    delay_dval = m_depth_charge_delay_default;
    string msg = "DepthCharge Launch by " + vname + " invalid delay. Default used.";
    reportRunWarning(msg);
    reportEvent(msg);
  }

  // Make sure the depth charge delay is not over the maximum allowed.
  if(delay_dval > m_depth_charge_delay_max)
    delay_dval = m_depth_charge_delay_max;

  // Part 2: Determine if this vehicle is allowed to launch a depth
  // charge based on the amount of charges remaining for this vehicle.

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
  postRangePulse(vx, vy, m_drop_color, vname+"_dcharge", 
     pulse_duration, dcharge_range, linger);

  return(true);
}

//---------------------------------------------------------
// Procedure: handleDepthChargeStatusRequest
//   Example: vname=alpha

bool TagManager::handleDepthChargeStatusRequest(const string& sval_in)
{
  // Get the vname from the request
  string vname = tokStringParse(sval_in, "name",  ',', '=');
  
  // Verify vehicle is already known to us
  if (m_map_node_records.count(vname) == 0)
    return true;

  // Get the status of the vehicle to be published
  string amt = uintToString(m_map_node_charges_have[vname]);
  string range = doubleToStringX(m_map_node_charge_range[vname]);
  string launches_ever = uintToString(m_map_node_launches_ever[vname]);
  string launches_now  = uintToString(m_map_node_launches_now[vname]);
  string launches_hit  = uintToString(m_map_node_launches_hit[vname]);

  // Post the status to variable to be bridged to vehicle
  string charge_status = "DEPTH_CHARGE_STATUS_" + toupper(vname);
  string sval = "name=" + vname + ",amt=" + amt + ",range=" + range +
    ",launches_ever=" + launches_ever + ",launches_now=" +
    launches_now + ",hits=" + launches_hit;
  Notify(charge_status,sval);

  return(true);
}

//---------------------------------------------------------
// Procedure: handleReplenishRequest
//   Example: vname=alpha

bool TagManager::handleReplenishRequest(const string& sval)
{
  // Get the vname from the request
  string vname = tokStringParse(sval, "name",  ',', '=');
  
  // Check to see if vehicle is close to replenish spot
  double distance = rangeToReplenishStation(vname);

  if(isUUV(vname)) {
    //TODO: Publish message that vehicle is not eligible to replenish
    return true;
  }
  
  if (!m_map_node_replenish_req[vname])
    if (distance < m_replenish_range) {
      m_map_node_replenish_req[vname] = true;
      m_map_node_replenish_req_time[vname] = MOOSTime();
    }

  //TODO: Publish message that replenish will not happen due to distance
    else {
    }
  
  return(true);
}

//------------------------------------------------------------
// Procedure: postReplenishRules

bool TagManager::postReplenishRules()
{
  stringstream ss;
  ss << "station=" << m_replenish_x << ":" << m_replenish_y << ",";
  ss << "range=" << m_replenish_range << ",";
  ss << "time=" << m_replenish_time;

  Notify("REPLENISH_RULES", ss.str());
  return(true);
}


//------------------------------------------------------------
// Procedure: postReplenishStatus

void TagManager::postReplenishStatus(const string& vname)
{
  double distance = rangeToReplenishStation(vname);
  double time_remaining = m_map_node_replenish_req_time[vname] 
    + m_replenish_time - MOOSTime();

  // Get the Replenish Status
  string sval;
  if(distance > m_replenish_range) {
    sval = "name=" + vname + ",status=out_of_range";
    m_map_node_replenish_req[vname] = false;
  }
  else if(time_remaining > 0)
    sval = "name=" + vname + ",status=replenishing,time=" +
      doubleToStringX(time_remaining);
  else {
    sval = "name=" + vname + ",status=complete,time=" +
      doubleToStringX(MOOSTime());
    m_map_node_charges_have[vname] = m_depth_charge_amount_default;
    m_map_node_replenish_req[vname] = false;
  }

  string var = "REPLENISH_STATUS_" + toupper(vname);
  Notify(var,sval);
}


//------------------------------------------------------------
// Procedure: isUUV
bool TagManager::isUUV(string& vname)
{
  return (MOOSStrCmp(m_map_node_records[vname].getType(), "UUV"));
}


//------------------------------------------------------------
// Procedure: postRangePulse

void TagManager::postRangePulse(double x, double y, const string& color,
           const string& label, double duration,
           double radius, double linger)
{
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
// Procedure: setDepthChargeConfig
//   Example: "name=henry, range=35, amt=3"

bool TagManager::setDepthChargeConfig(string str)
{
  vector<string> svector = parseString(str, ',');

  string vname, range, amount;

  unsigned int i, vsize = svector.size();
  for(i=0; i<vsize; i++) {
    string left  = biteStringX(svector[i], '=');
    string right = tolower(svector[i]); 
    
    if(left == "name")
      vname = right;

    else if(left == "range") {
      range = right;
      if(range == "")
  return(false);
    }
    else if(left == "amt") {
      amount = right;
      if(amount == "")
  return(false);
    }
  }

  if(vname == "")
    return(false);

  // Handle setting the depth charge range
  double drange = atof(range.c_str());
  if(!isNumber(range) || (drange < 0))
    return(false);
  else
    m_map_node_charge_range[vname] = drange;

  // Handle setting the depth charge count
  int int_amt = atoi(amount.c_str());
  if(!isNumber(amount) || (int_amt < 0))
    return(false);
  //else
    //m_map_node_charges_have[vname] = int_amt;
  
  return(true);
}

//------------------------------------------------------------
// Procedure: setDefaultChargeRange
//      Note: Argument must be a non-negative number
//   Example: "50"

bool TagManager::setDefaultChargeRange(string str)
{
  if(!isNumber(str))
    return(false);

  double range = atof(str.c_str());
  if(range < 0)
    return(false);

  m_depth_charge_range_default = range;
  return(true);
}

//------------------------------------------------------------
// Procedure: setDefaultChargeAmount
//      Note: Argument must be a non-negative number
//   Example: "3"

bool TagManager::setDefaultChargeAmount(string str)
{
  if(!isNumber(str))
    return(false);

  int amount = atof(str.c_str());
  if(amount < 0)
    return(false);

  m_depth_charge_amount_default = amount;
  return(true);
}

//------------------------------------------------------------
// Procedure:  setReplenishTime
//     Notes: Sets the time required for replenish
//            Argument must be a non-negative number
//   Example:  "30"

bool TagManager::setReplenishTime(string str)
{
  if(!isNumber(str))
    return(false);

  double amount = atof(str.c_str());
  if(amount < 0)
    return(false);

  m_replenish_time = amount;
  return(true);
}

//------------------------------------------------------------
// Procedure:  setReplenishRange
//     Notes: Sets the range (m) from the replenish point within which
//            a vehicle needs to be to replenish its depth charges.
//   Example:  "50"

bool TagManager::setReplenishRange(string str)
{
  if(!isNumber(str))
    return(false);

  double range = atof(str.c_str());
  if(range < 0)
    return(false);

  m_replenish_range = range;
  return(true);
}

//------------------------------------------------------------
// Procedure:  setDepthChargeDelayMax
//   Example:  "90"

bool TagManager::setDepthChargeDelayMax(string str)
{
  if(!isNumber(str))
    return(false);

  double delay = atof(str.c_str());
  if(delay < 0)
    return(false);

  m_depth_charge_delay_max = delay;
  return(true);
}

//------------------------------------------------------------
// Procedure: setDepthChargeDelayDefault
//      Note: This is the depth charge delay time used if a launch request
//            is sent without specifying a depth charge delay.

bool TagManager::setDepthChargeDelayDefault(string str)
{
  if(!isNumber(str))
    return(false);

  double delay = atof(str.c_str());
  if(delay < 0)
    return(false);

  m_depth_charge_delay_default = delay;
  return(true);
}

//------------------------------------------------------------
// Procedure: setReplenishStation
//     Notes: Sets location of the replenish station in local coordinates.
//   Example: "10,50"

bool TagManager::setReplenishStation(string str)
{
  string x_str = biteStringX(str, ',');
  string y_str = str;

  if(!isNumber(x_str) || !isNumber(y_str))
    return(false);
  
  double x_dbl = atof(x_str.c_str());
  double y_dbl = atof(y_str.c_str());

  m_replenish_x = x_dbl;
  m_replenish_y = y_dbl;
  
  return(true);
}

//------------------------------------------------------------
// Procedure: rangeToReplenishStation()

double TagManager::rangeToReplenishStation(string vname) 
{
  NodeRecord record = m_map_node_records[vname];
  double x = record.getX();
  double y = record.getY();
  double xdelta = (x - m_replenish_x);
  double ydelta = (y - m_replenish_y);
  double distance = hypot(xdelta, ydelta);

  return(distance);
}

//------------------------------------------------------------
// Procedure: getChargesRemaining
//   Example: "henry"

unsigned int TagManager::getChargesRemaining(string vname) const
{
  map<string, unsigned int>::const_iterator p;
  p = m_map_node_charges_have.find(vname);
  if(p == m_map_node_charges_have.end())
    return(0);
  else
    return(p->second);
}

//------------------------------------------------------------
// Procedure: processPendingDepthCharges

void TagManager::processPendingDepthCharges()
{
  list<DepthCharge>::iterator p;
  for(p=m_pending_charges.begin(); p!=m_pending_charges.end(); ) {
    DepthCharge dcharge = *p;
    double elapsed = m_curr_time - dcharge.getTimeLaunched(); 
    if(elapsed >= dcharge.getTimeDelay()) {
      invokeDepthCharge(dcharge);
      p = m_pending_charges.erase(p);
    }
    else
      ++p;
  }
}

//------------------------------------------------------------
// Procedure: handleVisualHints

bool TagManager::handleVisualHints(string str)
{
  vector<string> svector = parseString(str, ',');
  for(unsigned int i=0; i<svector.size(); i++) {
    string param = biteStringX(svector[i], '=');
    string value = svector[i];
    
    if((param == "drop_color") && isColor(value))
      m_drop_color = value;
    else if((param == "detonate_color") && isColor(value))
      m_detonate_color = value;
    else if((param == "hit_color") && isColor(value))
      m_hit_color = value;
    else
      return(false);
  }
  return(true);
}

//------------------------------------------------------------
// Procedure: invokeDepthCharge()

void TagManager::invokeDepthCharge(DepthCharge dcharge)
{
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
                string msg = "target=" + node_name + ",range=" + doubleToString(node_range, 1);
                Notify("DEPTH_CHARGE_HIT", msg);
                Notify("TARGET_HIT_ALL", node_name);

                // We don't remove the hit vehicle from the records, but we do
                // zero-out any depth charges that vehicle may have had.
                m_map_node_charges_have[node_name] = 0;

                reportEvent(toupper(dcharge_vname) + "  ----[***] Depth Charge HIT: " + node_name); }

            // A depth charge miss...
            else {
                // Declare the miss to the MOOSDB
                string msg = "target=" + node_name + ",range =" + doubleToString(node_range, 1);
                Notify("DEPTH_CHARGE_MISS", msg);
                reportEvent(toupper(dcharge_vname) + "  ----[***] Depth Charge MISS"); } } }
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
  m_msgs << "Inventory Configuration Settings " << endl;
  m_msgs << "  Default Charge Range:  " << m_depth_charge_range_default  << endl;
  m_msgs << "  Default Charge Amount: " << m_depth_charge_amount_default << endl;
  m_msgs << "Delay Configuration Settings " << endl;
  m_msgs << "  Depth Charge Delay Max:     " << m_depth_charge_delay_max << endl;
  m_msgs << "  Depth Charge Delay Default: " << m_depth_charge_delay_default << endl;
  m_msgs << "Replenish Configuration Settings " << endl;
  m_msgs << "  Replenish station: " << m_replenish_x << "," << m_replenish_y << endl;
  m_msgs << "  Replenish range:   " << m_replenish_range << endl;
  m_msgs << "  Replenish time:    " << m_replenish_time  << endl;
  m_msgs << "Visual Hints " << endl;
  m_msgs << "  Drop Color:     " << m_drop_color      << endl;
  m_msgs << "  Detonate Color: " << m_detonate_color  << endl;
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
    
    actab << vname << chrgs << range << launches_ever << launches_now << launches_hit;
  }
  m_msgs << actab.getFormattedString();
  return(true);
}
