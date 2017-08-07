/*****************************y*******************************/
/*    NAME: Jonathan Schwartz and Arjun Gupta                */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Defense.cpp                                    */
/*    DATE: 07/26/2017                                                */
/************************************************************/

#ifdef _WIN32
#pragma warning(disable : 4786)
#pragma warning(disable : 4503)
#endif

#include <iterator>
#include <cstdlib>
#include "MBUtils.h"
#include "BuildUtils.h"
#include "BHV_Defense_Collab.h"
#include <stdlib.h>
#include <cmath>
#include <string>
#include <iostream>
#include "ZAIC_PEAK.h"
#include <sstream>
#include "OF_Coupler.h"
#include "XYPoint.h"
#include "NodeReport.h"

#define PI 3.14159265358979323846264338327

using namespace std;

//---------------------------------------------------------------
// Constructor

BHV_Defense_Collab::BHV_Defense_Collab(IvPDomain domain) :
  IvPBehavior(domain)
{
  // Provide a default behavior name
  //  IvPBehavior::setParam("name", "defaultname");

  // Declare the behavior decision space
  m_domain = subDomain(m_domain, "course,speed");
  m_descriptor = "defend";
  
  m_flagX = 0;
  m_flagY = 0;
  m_attX  = 0;
  m_attY  = 0;
  m_speed = 2;
  m_dist_from_flag = 10;
  m_oppX  = 0;
  m_oppY  = 0;
  m_osX = 0;
  m_osY = 0;
  m_destX = 0;
  m_destY = 0;
  m_attack_angle = 0;
  m_attacker = "";
  m_team ="";
  m_curr_node_report = "";
  m_move = false;
  m_angle = 0;
  m_crit_dist = 80;

// Add any variables this behavior needs to subscribe for
  addInfoVars("NAV_X, NAV_Y, NODE_REPORT_EVAN, NODE_REPORT_FELIX, NODE_REPORT_MOKAI_RED, NODE_REPORT_MOKAI_BLUE");
  postMessage("STAT", "finished initializing");
}

//---------------------------------------------------------------
// Procedure: setParam()

bool BHV_Defense_Collab::setParam(string param, string val)
{
  // Convert the parameter to lower case for more general matching
  param = tolower(param);

  // Get the numerical value of the param argument for convenience once
  double double_val = atof(val.c_str());
  if((param == "flag_x") && isNumber(val)) {
    m_flagX = double_val;
    return(true);
  }
  
  else if((param == "team")) {
    m_team = val;
    return(true);
  }
  
  else if (param == "flag") {
    vector<string> flag_coord = parseString(val, ',');
    m_flagX = atof(flag_coord[0].c_str());
    m_flagY = atof(flag_coord[1].c_str());
    return(true);
  }
  
  else if((param == "enemy_flag")){
    vector<string> flag_coord = parseString(val, ',');
    m_attX = atof(flag_coord[0].c_str());
    m_attY = atof(flag_coord[1].c_str());
    return true;
  }
  
  else if((param == "speed") && isNumber(val)) {
    m_speed = double_val;
    return(true);
  }

  else if((param == "enemies")){
    m_enemy_list= parseString(val, ',');
    return(true);
  }
  
  else if((param == "distance_from_flag") && isNumber(val)) {
    m_dist_from_flag = double_val;
    return true;
  }
  // If not handled above, then just return false;
  return(false);
}


//---------------------------------------------------------------
// Procedure: onSetParamComplete()
//   Purpose: Invoked once after all parameters have been handled.
//            Good place to ensure all required params have are set.
//            Or any inter-param relationships like a<b.

void BHV_Defense_Collab::onSetParamComplete()
{
}

//---------------------------------------------------------------
// Procedure: onHelmStart()
//   Purpose: Invoked once upon helm start, even if this behavior
//            is a template and not spawned at startup

void BHV_Defense_Collab::onHelmStart()
{
}

//---------------------------------------------------------------
// Procedure: onIdleState()
//   Purpose: Invoked on each helm iteration if conditions not met.

void BHV_Defense_Collab::onIdleState()
{
}

//---------------------------------------------------------------
// Procedure: onCompleteState()

void BHV_Defense_Collab::onCompleteState()
{
}

//---------------------------------------------------------------
// Procedure: postConfigStatus()
//   Purpose: Invoked each time a param is dynamically changed

void BHV_Defense_Collab::postConfigStatus()
{
}

//---------------------------------------------------------------
// Procedure: onIdleToRunState()
//   Purpose: Invoked once upon each transition from idle to run state

void BHV_Defense_Collab::onIdleToRunState()
{
}

//---------------------------------------------------------------
// Procedure: onRunToIdleState()
//   Purpose: Invoked once upon each transition from run to idle state

void BHV_Defense_Collab::onRunToIdleState()
{
}

//---------------------------------------------------------------
// Procedure: getOppCoords()
//   Purpose: Look at opposing node reports to get their position

void BHV_Defense_Collab::getOppCoords(string node)
{
  postMessage("STAT", "starting to get opponent coordinates");
  NodeReport new_report(node);
  if(new_report.name=="" || new_report.nav_x==0 || new_report.nav_y==0){
    postWMessage("Invalid Node Report");
  }
  bool found =false;
  for(int i = 0; i<m_opp_list.size(); i++){
    if(m_opp_list[i].name == new_report.name){
      m_opp_list[i]=new_report;
      found= true;
      postMessage("STAT","updating node report: "+node);
    }
  }
  
  if((!found) && (new_report.group != m_team)){
    m_opp_list.push_back(new_report);
    postMessage("STAT", "added "+new_report.name);
  }
  
  if(new_report.name == m_attacker){
    if(hypot(new_report.nav_x-m_flagX, new_report.nav_y-m_flagY)>m_crit_dist){
      m_attacker="";
    }
    else{
      m_oppX = new_report.nav_x;
      m_oppY = new_report.nav_y;
    }
  }
}



//------------------------------------------------------------
//Procedure: onRunState()
//    Purpose: To calculate the X and Y values of where the robot should be if it is to be between its flag 
//             and the enemy robot (at a set distance from the flag)

IvPFunction* BHV_Defense_Collab::onRunState()
{
  postMessage("STAT", "Starting OnRunState()");
  bool check1, check2, check3;
  double dx, dy=0;
  m_osX = getBufferDoubleVal("NAV_X", check1);
  m_osY = getBufferDoubleVal("NAV_Y", check2);

  if(!check1 || !check2) {
    postWMessage("BHV_DEFENSE ERROR: No X/Y value in info_buffer!");
    return 0;
  }

  for(int i=0; i<m_enemy_list.size();i++){
    m_curr_node_report = getBufferStringVal("NODE_REPORT_"+m_enemy_list[i], check3);
    getOppCoords(m_curr_node_report);
  }
  
  if(!check3){
    postWMessage("BHV_DEFENSE ERROR: Node_report not found in info_buffer!");
    return 0;
  }


  IvPFunction *ipf = 0;
  double deltX,deltY=0;
  //we have identified a valid attacker coming within range for the flag
  if(m_attacker!=""){
    deltX = m_oppX-m_flagX;
    deltY = m_oppY-m_flagY;
  }
  else{
    deltX =m_attX- m_flagX;
    deltY =m_attY- m_flagY;
  }
  
  m_attack_angle = atan( abs(deltY) / abs(deltX) );

  //m_attack_angle increases counter clockwise, but resets to 0 every 180 degrees
  
  if (deltX*deltY < 0)
    m_attack_angle = PI - m_attack_angle;
  
  if(deltY>0){   
    m_destX = cos(m_attack_angle)*m_dist_from_flag + m_flagX;
    m_destY = sin(m_attack_angle)*m_dist_from_flag + m_flagY;
  }
  
  else{
    m_destX = m_flagX - cos(m_attack_angle)*m_dist_from_flag;
    m_destY = m_flagY - sin(m_attack_angle)*m_dist_from_flag;
  }
  postMessage("STAT", "sending robot to point "+to_string(m_destX)+","+to_string(m_destY));
      
  dx = m_destX-m_osX;
  dy = m_destY-m_osY;

  double dx1 = m_oppX-m_flagX;
  double dy1 = m_oppY-m_flagY;

  if(hypot(dx1,dy1) < m_dist_from_flag){
    dx = m_oppX-m_osX;
    dy = m_oppY-m_osY;
    m_move=true;
    //postMessage("STAT", to_string(m_oppX)+","+to_string(m_oppY));
  }
  //else
    //postMessage("VIEW_POINT", to_string(m_destX)+","+to_string(m_destY));
  
  m_angle = 90-atan(abs(dy)/abs(dx))*180/PI;
  
  
  //convert m_angle so that it the ipf function reads it properly
  //(starts at 0 on the positive y-axis and increases counter-clockwise)

  if (dx*dy<0)
    m_angle = 90-m_angle;

  if (dx>0 && dy<0)
    m_angle += 90;
  else if (dx<0 && dy<0)
    m_angle += 180;
  else if (dx<0 && dy>0)
    m_angle += 270;
  
 
      
  // Part N: Prior to returning the IvP function, apply the priority wt
  // Actual weight applied may be some value different than the configured
  // m_priority_wt, depending on the behavior author's insite.


  //determines if defending vehicle is close enough to were it should be
  //if it isn't, m_move is set to true which activates the ipf function
  
  //we have not found a suitble attacker yet 
  if(m_attacker ==""){
    double min_index=0;
    double min_dist = 81;
    for(int i=0; i<m_opp_list.size();i++){
      double delta_x = m_opp_list[i].nav_x-m_flagX;
      double delta_y = m_opp_list[i].nav_y-m_flagY;
      if(hypot(delta_x, delta_y)<min_dist){
	min_dist=hypot(delta_x, delta_y);
	min_index=i;
	postMessage("STAT", "Found Minimal Attacker");
      }
    }
    if(min_dist<m_crit_dist){
      m_attacker=m_opp_list[min_index].name;
      m_oppX= m_opp_list[min_index].nav_x;
      m_oppY= m_opp_list[min_index].nav_y;
      postMessage("STAT", "Set attacker called "+ m_attacker);
      //postWMessage("Set attacker to "+m_attacker);
    }
  }
  
  if ((hypot(dx, dy)>5))
    m_move=true;
  
  if (m_move){
    int speed = min(int(5*hypot(dx,dy)/m_dist_from_flag*m_speed), int(m_speed));
    ipf = buildFunctionWithZAIC(speed);
    m_move=false;
  }
    //otherwise use just enough speed to allow it to turn to face the correct direction
  if(ipf)
    ipf->setPWT(m_priority_wt);

  return(ipf);
}

//---------------------------------------------------------------
// Procedure: buildFunctionWithZAIC()
//     Purpose: Builds and IvP function for the behavior

IvPFunction *BHV_Defense_Collab::buildFunctionWithZAIC(double speed)
{
  ZAIC_PEAK spd_zaic(m_domain, "speed");
  spd_zaic.setSummit(speed);
  spd_zaic.setBaseWidth(0.3);
  spd_zaic.setPeakWidth(0.0);
  spd_zaic.setSummitDelta(0.0);
  IvPFunction *spd_of = spd_zaic.extractIvPFunction();

  ZAIC_PEAK crs_zaic(m_domain, "course");
  crs_zaic.setSummit(m_angle);
  crs_zaic.setBaseWidth(180.0);
  crs_zaic.setValueWrap(true);
  IvPFunction *crs_of = crs_zaic.extractIvPFunction();

  OF_Coupler coupler;
  IvPFunction *ipf = coupler.couple(crs_of, spd_of);

  return(ipf);
}
