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
#include "XYFormatUtilsMarker.h"

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
    if(param == "FLAG_SUMMARY") {
      handled = handleMailFlagSummary(value);
    }
    else if(param == "BAR") {
      handled = true;
    }

    if(!handled)
      reportUnhandledConfigWarning(orig);

  }

  cout << "Successfully started" << endl;
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
  cout << "In handleMailFlagSummary TOP" << endl;

  // Part 1: Process the summary making a new vector of flags for now.
  vector<XYMarker> new_flags;
  vector<string> svector = parseString(str, '#');
  for(unsigned int i=0; i<svector.size(); i++) {
    string flag_spec = svector[i];
    XYMarker flag = string2Marker(flag_spec);
    if(flag.is_set_x() && flag.is_set_y() && (flag.get_label() != "")) 
      new_flags.push_back(flag);
    else
      reportRunWarning("Ivalid Flag Summary: " + flag_spec);
  }

  // Part 2: Handle the new vector of flags, using the label as a key.
  // Replace any existing flags if a flag has changed. Add a new flag if
  // a newly received flag was not known prior.
  
  bool some_flags_were_changed_or_added = false;

  for(unsigned int i=0; i<new_flags.size(); i++) {
    XYMarker new_flag = new_flags[i];
    bool found = false;
    for(unsigned int j=0; j<m_flags.size(); j++) {
      if(new_flag.get_label() == m_flags[j].get_label()) {
	found = true;
	if(!flagsMatch(new_flag, m_flags[j])) {
	  m_flags[j] = new_flag;
	  some_flags_were_changed_or_added = true;
	}
      }
    }
    if(!found) {
      m_flags.push_back(new_flag);
      some_flags_were_changed_or_added = true;
    }
  }

  // Increment the counter and timestamp of the latest summary
  m_flag_summaries_received++;
  m_flag_summary_tstamp = m_curr_time - m_start_time;
  
  cout << "In handleMailFlagSummary BOTTOM" << endl;
  return(true);
}


//------------------------------------------------------------
// Procedure: flagsMatch

bool FlagStrategy::flagsMatch(const XYMarker& ma, const XYMarker& mb) const
{
  if(ma.get_vx() != mb.get_vx())
    return(false);
  if(ma.get_vy() != mb.get_vy())
    return(false);
  if(ma.get_range() != mb.get_range())
    return(false);
  if(ma.get_owner() != mb.get_owner())
    return(false);
  return(false);
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

  for(unsigned int i=0; i<m_flags.size(); i++) {
    string label, ownedby, s_x, s_y, s_range;
    label   = m_flags[i].get_label();
    ownedby = m_flags[i].get_owner();
    s_x = doubleToStringX(m_flags[i].get_vx(),2);
    s_y = doubleToStringX(m_flags[i].get_vy(),2);
    s_range = doubleToStringX(m_flags[i].get_range(),2);
    actab << label << ownedby << s_x << s_y << s_range;
  }

  m_msgs << actab.getFormattedString();

  return(true);
}
