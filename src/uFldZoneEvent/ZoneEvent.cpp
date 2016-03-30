/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ZoneEvent.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <cstdlib>
#include <iostream>
#include <cmath>
#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "ZoneEvent.h"
#include "XYFormatUtilsPoly.h"
#include "NodeRecordUtils.h"
#include "ColorParse.h"

using namespace std;

//---------------------------------------------------------
// Constructor

ZoneEvent::ZoneEvent()
{
  p_events_w_lock = new CMOOSLock();

  m_view_zone = false;
  m_zone_color = "orange";

  m_zone_name = "";
  m_vname = "";
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool ZoneEvent::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  return UpdateMOOSVariables(NewMail);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool ZoneEvent::OnConnectToServer()
{
  registerVariables();


  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void ZoneEvent::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("NODE_REPORT", 0);
  Register("NODE_REPORT_LOCAL", 0);
  AddMOOSVariable("dbtime", "DB_UPTIME", "NaN", 0);

  RegisterMOOSVariables();
}

bool ZoneEvent::postZonePoly()
{
  string spec = m_zone.get_spec();

  Notify("VIEW_POLYGON", spec);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool ZoneEvent::Iterate()
{
  AppCastingMOOSApp::Iterate();

  PublishFreshMOOSVariables();

  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool ZoneEvent::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  STRING_LIST sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(!m_MissionReader.GetConfiguration(GetAppName(), sParams)){
    reportConfigWarning("No config block found for " + GetAppName());
  } else {
    STRING_LIST::iterator p;

    for (p=sParams.begin(); p != sParams.end(); p++) {
      string orig  = *p;
      string line  = *p;
      string param = tolower(biteStringX(line, '='));
      string value = line;
      bool handled = false;

      if (param == "zone")
        handled = handleConfigZone(value);
      else if (param == "zone_name")
        handled = handleConfigGroupName(value);
      else if (param == "vehicle_name")
        handled = handleConfigVehicleName(value);
      else if (param == "post_var")
        handled = handleConfigPostVar(value);
      else if (param == "view_zone")
        handled = handleConfigViewZone(value);

      if (!handled)
        reportUnhandledConfigWarning(orig);
    }
  }


  AddActiveQueue("node_reports", this, &ZoneEvent::onNodeReport);
  AddMessageRouteToActiveQueue("node_reports", "NODE_REPORT_LOCAL");
  AddMessageRouteToActiveQueue("node_reports", "NODE_REPORT");

  if(m_view_zone)
    postZonePoly();

  registerVariables();

  return(true);
}

bool ZoneEvent::handleConfigViewZone(const string& str)
{
  string s = tolower(str);
  if(s=="false") {
    m_view_zone = false;
    return true;
  }
  else if (s=="true"){
    m_view_zone = true;
    return true;
  }

  return false;
}

bool ZoneEvent::handleConfigZone(const string& str)
{
  XYPolygon poly = string2Poly(str);
  if(poly.size() == 0)
    return(false);

  poly.set_edge_size(1);
  poly.set_vertex_size(1);
  poly.set_transparency(0.5);

  poly.set_color("vertex", "pink");
  poly.set_color("edge", "pink");
  poly.set_color("fill", m_zone_color);
  if(m_zone_name != "")
    poly.set_label("uFldZoneEvent_" + m_zone_name);
  else
    poly.set_label("uFldZoneEvent_zone");
  m_zone = poly;

  return(true);
}

bool ZoneEvent::handleConfigGroupName(const string& zone_name)
{
  // Sanity check - this app only handles TWO zones
  if(zone_name == "")
    return(false);

  if(m_zone_name == "") {
    m_zone_name = zone_name;
    m_zone.set_label("uFldZoneEvent_" + zone_name);
  }
  else
    return(false);

  return(true);
}

bool ZoneEvent::handleConfigVehicleName(const string& vname)
{
  if(vname == "")
    return(false);

  m_vname = vname;
  return(true);
}

bool ZoneEvent::handleConfigPostVar(const string& var_name)
{
  if (var_name == "")
    return(false);

  // var_name="UNTAG_REQUEST=vname=$[VNAME]"
  string val = var_name; // val="vname=$[VNAME]" (cf. next line)
  string var = biteStringX(val, '='); // var="UNTAG_REQUEST"

  m_map_var_val[var] = val;

  AddMOOSVariable(var, "", var, 0);
  return(true);
}

bool ZoneEvent::onNodeReport(CMOOSMsg& node_report_msg)
{
  NodeRecord new_node_record = string2NodeRecord(node_report_msg.GetString());

  string vname = new_node_record.getName();
  string vgroup = new_node_record.getGroup();
  if(vgroup == "") {
    string msg = "Node report for " + vname + " w/ no group.";
    p_events_w_lock->Lock();
    m_events.push_back(msg);
    p_events_w_lock->UnLock();
    return(false);
  }
  if((vname != m_vname) && (m_vname!="")) {
    string msg = "Node report for " + vname + " but zone registered for " + m_vname;
    return(false);
  }

  if(vgroup == m_zone_name || vname == m_vname){
    checkNodeInZone(new_node_record);
  } else {
    string msg = "Node report for " + vname + " w/ different group: " + vgroup;
    p_events_w_lock->Lock();
    m_events.push_back(msg);
    p_events_w_lock->UnLock();
    return(false);
  }

  return(true);
}

bool ZoneEvent::checkNodeInZone(NodeRecord& node_record)
{
  double dbtime = GetMOOSVar("dbtime")->GetDoubleVal();

  string vname = node_record.getName();
  string vgroup = node_record.getGroup();

  // Sanity check (DeMorgan Rule)
  if ((vgroup != m_zone_name) && (vname != m_vname)){
    return(false);
  }

  double vx = node_record.getX();
  double vy = node_record.getY();

  if(!m_zone.contains(vx, vy)){
    //  delete element from map: erase needs a iterator returned by find
    map<string, NodeRecord>::iterator p = m_map_node_records.find(vname);
    if (p != m_map_node_records.end())
      m_map_node_records.erase(p);
    return(false);
  }

  map<string, string>::iterator p;
  for (p = m_map_var_val.begin(); p != m_map_var_val.end(); ++p){
    string var_name = p->first;
    string val = p->second;

    val = findReplace(val, "$[VNAME]", vname);
    val = findReplace(val, "$[GROUP]", vgroup);
    val = findReplace(val, "$[TIME]", doubleToString(dbtime));
    val = findReplace(val, "$[VX]", doubleToString(vx));
    val = findReplace(val, "$[VY]", doubleToString(vy));

    SetMOOSVar(var_name, val, m_curr_time);
  }

  m_map_node_records[vname] = node_record;

  return(true);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool ZoneEvent::buildReport()
{
  double dbtime = GetMOOSVar("dbtime")->GetDoubleVal();
  m_msgs << "Global Settings\n";
  m_msgs << "============================================\n";
  m_msgs << "Zone (Group) Name = " << m_zone_name << endl;
  m_msgs << "Vehicle's Name = " << m_vname << endl;

  m_msgs << "Registred variables to be published:" << endl;
  map<string, string>::iterator p;
  for (p = m_map_var_val.begin(); p != m_map_var_val.end(); ++p){
    m_msgs << "\t";
    string var_name = p->first;
    string val = p->second;
    m_msgs << var_name << " : " << val << endl;
  }

  m_msgs << endl;
  m_msgs << endl;

  ACTable actab(5);
  actab << "Time | Vehicle Name | Group | POS_X | POS_Y";
  actab.addHeaderLines();

  map<string, NodeRecord>::iterator pNR;
  for (pNR = m_map_node_records.begin(); pNR != m_map_node_records.end(); ++pNR){
    actab << dbtime << pNR->second.getName() << pNR->second.getGroup();
    actab << pNR->second.getX() << pNR->second.getY();
  }
  m_msgs << actab.getFormattedString();

  m_msgs << endl;
  m_msgs << endl;
  m_msgs << endl;
  m_msgs << "Events:\n";
  m_msgs << "============================================\n";

  p_events_w_lock->Lock();
  vector<string>::iterator it;
  for (it = m_events.begin() ; it != m_events.end(); ++it)
    m_msgs << dbtime << ": " << *it << endl;
  m_events.clear();
  p_events_w_lock->UnLock();


  return(true);
}
