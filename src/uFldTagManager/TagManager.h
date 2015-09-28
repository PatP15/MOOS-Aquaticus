/*****************************************************************/
/*    NAME: Michael Benjamin                                     */
/*    ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA     */
/*    FILE: TagManager.h                                         */
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

#ifndef UFLD_TAG_MANAGER_HEADER
#define UFLD_TAG_MANAGER_HEADER

#include <vector>
#include <string>
#include <map>
#include <set>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "VTag.h"
#include "NodeRecord.h"
#include "XYRangePulse.h"

class TagManager : public AppCastingMOOSApp
{
 public:
  TagManager();
  virtual ~TagManager() {};

  bool  OnNewMail(MOOSMSG_LIST &NewMail);
  bool  OnStartUp();
  bool  Iterate();
  bool  OnConnectToServer();

  void  registerVariables();
  bool  buildReport();

 protected: // Config Utilities
  bool    handleConfigVTagRange(std::string);

 protected: // Incoming mail utilities
  bool    handleNodeReport(const std::string&);
  bool    handleVTagPost(const std::string&);

 protected: // Processing Utilities
  double  getTrueNodeRange(double, double, std::string);
  double  getNoisyNodeRange(double true_range) const;

  void    processVTags();
  void    processVTag(VTag);  
  
 protected: // Outgoing mail utilities
  void    postRangePulse(double x, double y, std::string color,
			 std::string label, double dur, double radius);

  void    postResult(std::string event, std::string vname,
		     std::string vteam, std::string result);
  void    postResult(std::string event, std::string vname,
		     std::map<std::string, double>);
  
 protected: // State variables

  // Node (postion) records: Map keyed on vehicle name
  std::map<std::string, NodeRecord>   m_map_node_records;
  std::map<std::string, unsigned int> m_map_node_reports_rcd;

  // Perspective of vehicles doing tagging: Mapp keyed on vehicle name
  std::map<std::string, unsigned int> m_map_node_vtags_requested;
  std::map<std::string, unsigned int> m_map_node_vtags_accepted;
  std::map<std::string, unsigned int> m_map_node_vtags_succeeded;
  std::map<std::string, unsigned int> m_map_node_vtags_rejfreq;
  std::map<std::string, unsigned int> m_map_node_vtags_rejzone;
  std::map<std::string, double>       m_map_node_vtags_last_tag;

  // Perspective of vehicles being tagged: Mapp keyed on vehicle name
  std::map<std::string, unsigned int> m_map_node_vtags_beentagged;

  // Other key states
  std::list<VTag>  m_pending_vtags;

  std::map<std::string, std::set<std::string> > m_map_teams;
  
 protected: // Configuration variables
  double        m_vtag_range;
  double        m_vtag_min_interval;
  
  unsigned int  m_tag_events;

  // Visual hints
  std::string   m_post_color;
  std::string   m_hit_color;
  std::string   m_miss_color;
};

#endif 

