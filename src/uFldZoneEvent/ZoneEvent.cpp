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
  m_zone_color = "white";

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

      if (!handled)
        reportUnhandledConfigWarning(orig);
    }
  }


  AddActiveQueue("node_reports", this, &ZoneEvent::onNodeReport);
  AddMessageRouteToActiveQueue("node_reports", "NODE_REPORT_LOCAL");
  AddMessageRouteToActiveQueue("node_reports", "NODE_REPORT");

  postZonePoly();

  return(true);
}

bool ZoneEvent::handleConfigZone(string str)
{
  XYPolygon poly = string2Poly(str);
  if(poly.size() == 0)
    return(false);

  poly.set_edge_size(1);
  poly.set_vertex_size(1);
  poly.set_transparency(0.5);

  poly.set_color("vertex", "white");
  poly.set_color("edge", "white");
  poly.set_color("fill", m_zone_color);
  if(m_zone_name != "")
    poly.set_label("uFldZoneEvent_" + m_zone_name);
  else
    poly.set_label("uFldZoneEvent_zone");
  m_zone = poly;

  return(true);
}

bool ZoneEvent::handleConfigGroupName(string zone_name)
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

bool ZoneEvent::handleConfigVehicleName(string vname)
{
  if(vname == "")
    return(false);

  m_vname = vname;
  return(true);
}

bool ZoneEvent::handleConfigPostVar(string var_name)
{
  if (var_name == "")
    return(false);

  // var_name="UNTAG_REQUEST=vname=$vname"
  string val = var_name; // val="vname=$vname" (cf. next line)
  string var = biteStringX(val, '='); // var="UNTAG_REQUEST"

  // val="vname=$vname"
  string static_val = val; // static_val="$vname"
  string static_var = biteStringX(static_val, '='); // static_var="vname"

  ZoneEvent::ReturnPostVal returnPost;
  if(static_val == "$vname")
    returnPost = rvname;
  else if (static_val == "$group")
    returnPost = rgroup;
  else if (static_val == "$time")
    returnPost = rtime;
  else if (static_val == "$vx")
    returnPost = rvx;
  else if (static_val == "$vy")
    returnPost = rvy;
  else
    returnPost = rstatic;

  m_map_var_val[var] = returnPost;
  if(returnPost==rstatic)
    m_map_static_var_val[var] = val;
  else
    m_map_static_var_val[var] = static_var + '=';

  AddMOOSVariable(var, "", var, 0);
  return(true);
}

bool ZoneEvent::onNodeReport(CMOOSMsg& node_report_str)
{
  NodeRecord new_node_record = string2NodeRecord(node_report_str.GetString());

  string vname = new_node_record.getName();
  string vgroup = new_node_record.getGroup();
  if(vgroup == "") {
    string msg = "Node report for " + vname + " w/ no group.";
    reportRunWarning(msg);
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
    reportRunWarning(msg);
    return(false);
  }

  return(true);
}

bool ZoneEvent::checkNodeInZone(NodeRecord& node_record)
{
  string vname = node_record.getName();
  string vgroup = node_record.getGroup();

  // Sanity check (DeMorgan Rule)
  if ((vgroup != m_zone_name) && (vname != m_vname)){
    return(false);
  }

  double vx = node_record.getX();
  double vy = node_record.getY();

  //TODO
  if(!m_zone.contains(vx, vy)){
    return(false);
  }

  map<string, ZoneEvent::ReturnPostVal>::iterator p;
  for (p = m_map_var_val.begin(); p != m_map_var_val.end(); ++p){
    string var_name = p->first;
    ZoneEvent::ReturnPostVal rpv = p->second;

    switch (rpv)
    {
      case rvname: //, rgroup, rtime, rvx, rvy, rstatic:
        SetMOOSVar(var_name, m_map_static_var_val[var_name] + vname, m_curr_time);
        break;
      case rtime:
        SetMOOSVar(var_name, m_map_static_var_val[var_name] + doubleToString(m_curr_time), m_curr_time);
        break;
      case rvx:
        SetMOOSVar(var_name, m_map_static_var_val[var_name] + doubleToString(vx), m_curr_time);
        break;
      case rvy:
        SetMOOSVar(var_name, m_map_static_var_val[var_name] + doubleToString(vy), m_curr_time);
        break;
      case rstatic:
      default:
        SetMOOSVar(var_name, m_map_static_var_val[var_name], m_curr_time);
        break;
    }
  }

  return(true);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool ZoneEvent::buildReport()
{
  m_msgs << "Global Settings\n";
  m_msgs << "============================================\n";
  m_msgs << "Zone (Group) Name = " << m_zone_name << endl;
  m_msgs << "Vehicle's Name = " << m_vname << endl;

  m_msgs << "Registred variables to be published:" << endl;
  map<string, ZoneEvent::ReturnPostVal>::iterator p;
  for (p = m_map_var_val.begin(); p != m_map_var_val.end(); ++p){
    m_msgs << "\t";
    string var_name = p->first;
    ZoneEvent::ReturnPostVal rpv = p->second;
    switch (rpv)
    {
      case rstatic:
        m_msgs << p->first << " (static)";
        break;
      default:
        m_msgs << p->first << " (dynamic)";
        break;
    }
    m_msgs << endl;
  }

  //TODO
  m_msgs << endl;
  m_msgs << "Events:\n";
  m_msgs << "============================================\n";

  // ACTable actab(4);
  // actab << "Time | Bravo | Charlie | Delta";
  // actab.addHeaderLines();
  // actab << "one" << "two" << "three" << "four";
  // m_msgs << actab.getFormattedString();

  return(true);
}
