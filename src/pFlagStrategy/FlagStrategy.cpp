/************************************************************/
/*   NAME: Mike Benjamin                                    */
/*   ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA */
/*   FILE: FlagStrategy.cpp                                 */
/*   DATE: August 18th, 2015                                */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "ACTable.h"
#include "FlagStrategy.h"

using namespace std;

//---------------------------------------------------------
// Constructor

FlagStrategy::FlagStrategy()
{
  m_flag_summaries_received = 0;
  m_flag_summary_tstamp = 0;
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool FlagStrategy::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);

  MOOSMSG_LIST::iterator p;
  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;
    string key   = msg.GetKey();
    string sval  = msg.GetString(); 

#if 0 // Keep these around just for template
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif

    bool handled = false;
    if(key == "FLAG_SUMMARY") 
      handled = handleMailFlagSummary(sval);
	
     else if(key != "APPCAST_REQ") // handle by AppCastingMOOSApp
       reportRunWarning("Unhandled Mail: " + key);
   }
	
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool FlagStrategy::OnConnectToServer()
{
   registerVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool FlagStrategy::Iterate()
{
  AppCastingMOOSApp::Iterate();
  // Do your thing here!
  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool FlagStrategy::OnStartUp()
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
    string param = toupper(biteStringX(line, '='));
    string value = line;

    bool handled = false;
    if(param == "FOO") {
      handled = true;
    }
    else if(param == "BAR") {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }
  
  registerVariables();	
  return(true);
}

//---------------------------------------------------------
// Procedure: registerVariables

void FlagStrategy::registerVariables()
{
  AppCastingMOOSApp::RegisterVariables();
  Register("FLAG_SUMMARY", 0);
}

//---------------------------------------------------------
// Procedure: handleMailFlagSummary
//   Example: "label=one,x=8,y=4,grab_dist=10,ownedby=none #
//             label=two,x=2,y=9,grab_dist=10,ownedby=henry"

bool FlagStrategy::handleMailFlagSummary(string str)
{
  // First clear everything we know about flags
  m_flags_x.clear();
  m_flags_y.clear();
  m_flags_grab_dist.clear();
  m_flags_ownedby.clear();
  m_flags_label.clear();

  // Process the new flag summary
  vector<string> svector = parseString(str, '#');
  for(unsigned int i=0; i<svector.size(); i++) {
    vector<string> jvector = parseString(svector[i], ',');
    string label, ownedby;
    double x=0;
    double y=0;
    double grab_dist=0;
    for(unsigned int j=0; j<jvector.size(); j++) {
      string param = biteStringX(jvector[j], '=');
      string value = jvector[j];
      if(param == "label")
	label = value;
      else if(param == "ownedby")
	ownedby = value;
      else if(param == "x")
	x = atof(value.c_str());
      else if(param == "y")
	y = atof(value.c_str());
      else if(param == "grab_dist")
	grab_dist = atof(value.c_str());
    }
    if(label != "") {
      m_flags_x.push_back(x);
      m_flags_y.push_back(y);
      m_flags_grab_dist.push_back(grab_dist);
      m_flags_label.push_back(label);
      m_flags_ownedby.push_back(ownedby);
    }
  }

  // Increment the counter and timestamp of the latest summary
  m_flag_summaries_received++;
  m_flag_summary_tstamp = m_curr_time - m_start_time;
  
  return(true);
}


//------------------------------------------------------------
// Procedure: buildReport()

bool FlagStrategy::buildReport() 
{
  string s_last_summary_tstamp = "n/a";
  if(m_flag_summary_tstamp > 0)
    s_last_summary_tstamp = doubleToString(m_flag_summary_tstamp,2);
  
  m_msgs << "Flag Summaries Received: " << m_flag_summaries_received;
  m_msgs << " (" << s_last_summary_tstamp << ")" << endl << endl;


  ACTable actab(5);
  actab << "Label | OwnedBy | X | Y | GrabDist";
  actab.addHeaderLines();

  for(unsigned int i=0; i<m_flags_x.size(); i++) {
    string label, ownedby, s_x, s_y, s_grab_dist;
    label = m_flags_label[i];
    ownedby = m_flags_ownedby[i];
    s_x = doubleToStringX(m_flags_x[i],2);
    s_y = doubleToStringX(m_flags_y[i],2);
    s_grab_dist = doubleToStringX(m_flags_grab_dist[i],2);
    actab << label << ownedby << s_x << s_y << s_grab_dist;
  }

  m_msgs << actab.getFormattedString();

  return(true);
}
