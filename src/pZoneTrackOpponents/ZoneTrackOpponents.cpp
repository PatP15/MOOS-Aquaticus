/************************************************************/
/*    NAME: Michael Novitzky                                */
/*    ORGN: MIT/USMA                                        */
/*    FILE: ZoneTrackOpponents.cpp                          */
/*    DATE: June 12th, 2022                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "ZoneTrackOpponents.h"
#include "NodeRecordUtils.h"
#include <math.h>
#include <stdlib.h>

using namespace std;

//---------------------------------------------------------
// Constructor

ZoneTrackOpponents::ZoneTrackOpponents()
{
  m_high_value_point_set = false;
  m_contact_track = "";
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
    string comm  = msg.GetCommunity();op_for_x
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
    //extract location and check if within bounds
    double op_for_x = node_record.getX();
    double op_for_y = node_record.getY();
    //get bounding angle
    /*
    theta_1_a = abs(atan((y2 - y1)/(x2 - x1)));
    theta_1_b = abs(atan((y4 - y1)/(x4 - x1)));
    theta_2_a = abs(atan((y3 - y2)/(x3 - x2)));
    theta_2_b = abs(atan((y1 - y2)/(x1 - x2)));
    theta_3_a = abs(atan((y4 - y3)/(x4 - x3)));
    theta_3_b = abs(atan((y2 - y3)/(x2 - x3)));
    theta_4_a = abs(atan((y1 - y4)/(x1 - x4)));
    theta_4_b = abs(atan((y3 - y4)/(x3 - x4)));
    */
    if((x1 <= x2) && (y1 <= y2)){
    	theta_1_a = (atan((y2 - y1)/(x2 - x1)))*(180/M_PI);
    } else if ((x1 <= x2) && (y1 > y2)){
    	theta_1_a = 360 + (atan((y2 - y1)/(x2 - x1)))*(180/M_PI);
    } else if (((x1 >= x2) && (y1 <= y2)) || ((x1 >= x2) && (y1 >= y2))) {
    	theta_1_a = 180 + (atan((y2 - y1)/(x2 - x1)))*(180/M_PI);
    }
    if((x1 <= x4) && (y1 <= y4)){
    	theta_1_b = (atan((y4 - y1)/(x4 - x1)))*(180/M_PI);
    } else if ((x1 <= x4) && (y1 > y4)){
    	theta_1_b = 360 + (atan((y4 - y1)/(x4 - x1)))*(180/M_PI);
    } else if (((x1 >= x4) && (y1 <= y4)) || ((x1 >= x4) && (y4 >= y2))) {
    	theta_1_b = 180 + (atan((y4 - y1)/(x4 - x1)))*(180/M_PI);
    }
    if((x2 <= x3) && (y2 <= y3)){
    	theta_2_a = (atan((y3 - y2)/(x3 - x2)))*(180/M_PI);
    } else if ((x2 <= x3) && (y2 > y3)){
    	theta_2_a = 360 + (atan((y3 - y2)/(x3 - x2)))*(180/M_PI);
    } else if (((x2 >= x3) && (y2 <= y3)) || ((x2 >= x3) && (y2 >= y3))) {
    	theta_2_a = 180 + (atan((y3 - y2)/(x3 - x2)))*(180/M_PI);
    }
    if((x2 <= x1) && (y2 <= y1)){
    	theta_2_b = (atan((y1 - y2)/(x1 - x2)))*(180/M_PI);
    } else if ((x2 <= x1) && (y2 > y1)){
    	theta_2_b = 360 + (atan((y1 - y2)/(x1 - x2)))*(180/M_PI);
    } else if (((x2 >= x1) && (y2 <= y1)) || ((x2 >= x1) && (y2 >= y1))) {
    	theta_2_b = 180 + (atan((y1 - y2)/(x1 - x2)))*(180/M_PI);
    }
    if((x3 <= x4) && (y3 <= y4)){
    	theta_3_a = (atan((y4 - y3)/(x4 - x3)))*(180/M_PI);
    } else if ((x3 <= x4) && (y3 > y4)){
    	theta_3_a = 360 + (atan((y4 - y3)/(x4 - x3)))*(180/M_PI);
    } else if (((x3 >= x4) && (y3 <= y4)) || ((x3 >= x4) && (y3 >= y4))) {
    	theta_3_a = 180 + (atan((y4 - y3)/(x4 - x3)))*(180/M_PI);
    }
    if((x3 <= x2) && (y3 <= y2)){
    	theta_3_b = (atan((y2 - y3)/(x2 - x3)))*(180/M_PI);
    } else if ((x3 <= x2) && (y3 > y2)){
    	theta_3_b = 360 + (atan((y2 - y3)/(x2 - x3)))*(180/M_PI);
    } else if (((x3 >= x2) && (y3 <= y2)) || ((x3 >= x2) && (y3 >= y2))) {
    	theta_3_b = 180 + (atan((y2 - y3)/(x2 - x3)))*(180/M_PI);
    }
    if((x4 <= x1) && (y4 <= y1)){
    	theta_4_a = (atan((y1 - y4)/(x1 - x4)))*(180/M_PI);
    } else if ((x4 <= x1) && (y4 > y1)){
    	theta_4_a = 360 + (atan((y1 - y4)/(x1 - x4)))*(180/M_PI);
    } else if (((x4 >= x1) && (y4 <= y1)) || ((x4 >= x1) && (y4 >= y1))) {
    	theta_4_a = 180 + (atan((y1 - y4)/(x1 - x4)))*(180/M_PI);
    }
    if((x4 <= x3) && (y4 <= y3)){
    	theta_4_b = (atan((y3 - y4)/(x3 - x4)))*(180/M_PI);
    } else if ((x4 <= x3) && (y4 > y3)){
    	theta_4_b = 360 + (atan((y3 - y4)/(x3 - x4)))*(180/M_PI);
    } else if (((x4 >= x3) && (y4 <= y3)) || ((x4 >= x3) && (y4 >= y3))) {
    	theta_4_b = 180 + (atan((y3 - y4)/(x3 - x4)))*(180/M_PI);
    }
    /*
    theta_1_b = atan((y4 - y1)/(x4 - x1));
    theta_2_a = atan((y3 - y2)/(x3 - x2));
    theta_2_b = atan((y1 - y2)/(x1 - x2));
    theta_3_a = atan((y4 - y3)/(x4 - x3));
    theta_3_b = atan((y2 - y3)/(x2 - x3));
    theta_4_a = atan((y1 - y4)/(x1 - x4));
    theta_4_b = atan((y3 - y4)/(x3 - x4));
    */
    //check if within bounds
    /*
    theta_1 = abs(atan((op_for_y - y1)/(op_for_x - x1)));
    theta_2 = abs(atan((op_for_y - y2)/(op_for_x - x2)));
    theta_3 = abs(atan((op_for_y - y3)/(op_for_x - x3)));
    theta_4 = abs(atan((op_for_y - y4)/(op_for_x - x4)));
    */
    if((x1 <= op_for_x) && (y1 <= op_for_y)){
    	theta_1 = (atan((op_for_y - y1)/(op_for_x - x1)))*(180/M_PI);
    } else if ((x1 <= op_for_x) && (y1 >= op_for_y)){
    	theta_1 = 360 + (atan((op_for_y - y1)/(op_for_x - x1)))*(180/M_PI);
    } else if (((x1 >= op_for_x) && (y1 <= op_for_y)) || ((x1 >= op_for_x) && (y1 >= op_for_y))) {
    	theta_1 = 180 + (atan((op_for_y - y1)/(op_for_x - x1)))*(180/M_PI);
    }
    if((x2 <= op_for_x) && (y2 <= op_for_y)){
    	theta_2 = (atan((op_for_y - y2)/(op_for_x - x2)))*(180/M_PI);
    } else if ((x2 <= op_for_x) && (y2 >= op_for_y)){
    	theta_2 = 360 + (atan((op_for_y - y2)/(op_for_x - x2)))*(180/M_PI);
    } else if (((x2 >= op_for_x) && (y2 <= op_for_y)) || ((x2 >= op_for_x) && (y2 >= op_for_y))) {
    	theta_2 = 180 + (atan((op_for_y - y2)/(op_for_x - x2)))*(180/M_PI);
    }
    if((x3 <= op_for_x) && (y3 <= op_for_y)){
    	theta_3 = (atan((op_for_y - y3)/(op_for_x - x3)))*(180/M_PI);
    } else if ((x3 <= op_for_x) && (y3 >= op_for_y)){
    	theta_3 = 360 + (atan((op_for_y - y3)/(op_for_x - x3)))*(180/M_PI);
    } else if (((x3 >= op_for_x) && (y3 <= op_for_y)) || ((x3 >= op_for_x) && (y3 >= op_for_y))) {
    	theta_3 = 180 + (atan((op_for_y - y3)/(op_for_x - x3)))*(180/M_PI);
    }
    if((x4 <= op_for_x) && (y4 <= op_for_y)){
    	theta_4 = (atan((op_for_y - y4)/(op_for_x - x4)))*(180/M_PI);
    } else if ((x4 <= op_for_x) && (y4 >= op_for_y)){
    	theta_4 = 360 + (atan((op_for_y - y4)/(op_for_x - x4)))*(180/M_PI);
    } else if (((x4 >= op_for_x) && (y4 <= op_for_y)) || ((x4 >= op_for_x) && (y4 >= op_for_y))) {
    	theta_4 = 180 + (atan((op_for_y - y4)/(op_for_x - x4)))*(180/M_PI);
    }
    /*
    theta_1 = atan((op_for_y - y1)/(op_for_x - x1));
    theta_2 = atan((op_for_y - y2)/(op_for_x - x2));
    theta_3 = atan((op_for_y - y3)/(op_for_x - x3));
    theta_4 = atan((op_for_y - y4)/(op_for_x - x4));
    */
    if(((theta_1 >= theta_1_b) || (theta_1 <= theta_1_a)) && ((theta_2 >= theta_2_b) && (theta_2 <= theta_2_a)) && ((theta_3 >= theta_3_b) && (theta_3 <= theta_3_a)) && ((theta_4 >= theta_4_b) && 	(theta_4 <= theta_4_a))){
	foundIntrudingContact = true;
        Notify("AGGRESSIVE","TRUE");
        Notify("AGGRESSIVE_CONTACT",contact_name);
        m_in_zone = "TRUE";
        if(m_high_value_point_set) {
        	//we keep intruder location for high value threat
        	m_map_intruders_x[contact_name] = op_for_x;
        	m_map_intruders_y[contact_name] = op_for_y;
        	m_map_intruders_name[contact_name] = contact_name; //used for convenience
    	}
    }
      /*
      //check x first
      if(op_for_x > m_min_x && op_for_x < m_max_x){
        //ok now check within y
        if(op_for_y > m_min_y && op_for_y < m_max_y){
          foundIntrudingContact = true;
          Notify("AGGRESSIVE","TRUE");
          Notify("AGGRESSIVE_CONTACT",contact_name);
          m_in_zone = "TRUE";
          if(m_high_value_point_set) {
            //we keep intruder location for high value threat
            m_map_intruders_x[contact_name] = op_for_x;
            m_map_intruders_y[contact_name] = op_for_y;
            m_map_intruders_name[contact_name] = contact_name; //used for convenience

          }
                              
        }
      }
*/      
      else {
        //means a contact may previously exist and should be removed
        if(m_high_value_point_set) {
          m_map_intruders_x.erase(contact_name);
          m_map_intruders_y.erase(contact_name);
          m_map_intruders_name.erase(contact_name);
        }
        
      }
    }
  
  
  if(!foundIntrudingContact){
    Notify("AGGRESSIVE","FALSE");
    m_in_zone = "FALSE";
    m_contact_track = "";
  }
  else {
    //means we have found an intruding contact
    //do we check against high value point?
    if(m_high_value_point_set){
      //should be tracking
      if(m_map_intruders_x.size() == 1){
        //only contact means only contact
        std::map<string,string>::iterator it = m_map_intruders_name.begin();
        std::string close_contact = it->second;
        std::string proper_form = "contact=";
        proper_form+=close_contact;
        Notify("CR_INTERCEPT_UPDATES",proper_form );
	m_contact_track = proper_form;
      }
      else{
        std::string close_contact;
        //we assume at most 2 contacts
        std::map<std::string, double>::iterator it_x = m_map_intruders_x.begin();
        std::map<std::string, double>::iterator it_y = m_map_intruders_y.begin();
        std::map<std::string, std::string>::iterator it_name = m_map_intruders_name.begin();

        double temp_x_1 = it_x->second;
        double temp_y_1 = it_y->second;
        std::string temp_name_1 = it_name->second;
        //calculate range to high value point
        double temp_range_1 = sqrt(pow(temp_x_1-m_high_value_point_x,2)+ pow(temp_y_1-m_high_value_point_y,2));

        it_x++;
        it_y++;
        it_name++;

        double temp_x_2 = it_x->second;
        double temp_y_2 = it_y->second;
        std::string temp_name_2 = it_name->second;
        //calculate range to high value point
        double temp_range_2 = sqrt(pow(temp_x_2-m_high_value_point_x,2)+ pow(temp_y_2-m_high_value_point_y,2));

        std::string proper_form = "contact=";
        //now let's compare the ranges
        if(temp_range_1 <= temp_range_2 ){
          proper_form+=temp_name_1;
          Notify("CR_INTERCEPT_UPDATES",proper_form );
	  m_contact_track = proper_form;
        }
        else{
          proper_form+=temp_name_2;
          Notify("CR_INTERCEPT_UPDATES",proper_form);
	  m_contact_track = proper_form;
        }
      }
    }
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
    else if(param == "pts"){
      handled = handlePointsAssignment(orig);
    }
    else if(param == "opfor"){
      handled = handleOpForAssignment(value);
    }
    else if(param == "point"){
      handled = handleHighValuePoint(value);
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
  m_msgs << "South West Corner: x = " << x1 << " y = " << y1 << endl;
  m_msgs << "North West Corner: x = " << x2 << " y = " << y2 << endl;
  m_msgs << "North East Corner: x = " << x3 << " y = " << y3 << endl;
  m_msgs << "South East Corner: x = " << x4 << " y = " << y4 << endl;
  m_msgs << "Theta 1 a: " << theta_1_a << " Theta 1 b: " << theta_1_b << " Theta 1: " << theta_1 << endl;
  m_msgs << "Theta 2 a: " << theta_2_a << " Theta 2 b: " << theta_2_b << " Theta 2: " << theta_2 << endl;
  m_msgs << "Theta 3 a: " << theta_3_a << " Theta 3 b: " << theta_3_b << " Theta 3: " << theta_3 << endl;
  m_msgs << "Theta 4 a: " << theta_4_a << " Theta 4 b: " << theta_4_b << " Theta 4: " << theta_4 << endl;
  if(m_high_value_point_set){
    m_msgs << "high value point x: " << m_high_value_point_x << " y: " << m_high_value_point_y <<endl;
  }
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
  m_msgs << "Contact Tracking: " << m_contact_track << endl;
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
// Procedure: handlePointsAssignment

bool ZoneTrackOpponents::handlePointsAssignment(std::string orig)
{
  //expecting in .moos parameter file: pts={x1,y1:x2,y2:x3,y3:x4,y4}
  std::string ptsString = biteStringX(orig, '=');
  vector<std::string> str_vector = parseString(orig, ',');
  if(str_vector.size() !=8){
    return false;
  }

    //we will convert strings to double
  x1 = atof(str_vector[0].c_str());
  y1 = atof(str_vector[1].c_str());
  x2 = atof(str_vector[2].c_str());
  y2 = atof(str_vector[3].c_str());
  x3 = atof(str_vector[4].c_str());
  y3 = atof(str_vector[5].c_str());
  x4 = atof(str_vector[6].c_str());
  y4 = atof(str_vector[7].c_str());

 return true;
}


//---------------------------------------------------------
// Procedure: handleHighValuePoint

bool ZoneTrackOpponents::handleHighValuePoint(std::string value)
{
  //expecting 1 point1 point
  //expecting in .moos parameter file: point = x,y
  vector<std::string> str_vector = parseString(value, ',');
  if(str_vector.size() !=2){
    return false;
  }

  //we will convert strings to double
  m_high_value_point_x =  atof(str_vector[0].c_str());
  m_high_value_point_y =  atof(str_vector[1].c_str());
  m_high_value_point_set = true;

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
 



