/*****************************************************************/
/*    NAME: Michael Benjamin, Henrik Schmidt, and John Leonard   */
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

 protected: // Incoming mail utility
  bool    handleNodeReport(const std::string&);
  bool    handleVTagLaunch(const std::string&);


 protected: // Outgoing mail utility
  void    postVTagLaunch(const std::string&, const std::string&, double rng); 
  void    postVTagHit(const std::string&, const std::string&, double rng); 
  void    postRangePulse(double x, double y, const std::string& color,
			 const std::string& label, double dur, double radius, 
			 double linger=0);

 protected: // Utilities
  bool    setVTagConfig(std::string);
  bool    setVTagRange(std::string);
  bool    setVTagAmount(std::string);
  bool    setVTagDelayMax(std::string);
  bool    setVTagDefault(std::string);

  double  getTrueNodeRange(double, double, const std::string&);
  double  getNoisyNodeRange(double true_range) const;

  unsigned int getVTagsRemaining(std::string) const;

  void    processPendingVTags();
  void    invokeVTag(VTag);

  bool    handleVisualHints(std::string);

 protected: // State variables

  // Map is keyed on the name of the vehicle
  std::map<std::string, NodeRecord>   m_map_node_records;
  std::map<std::string, unsigned int> m_map_node_reports_rcd;
  std::map<std::string, unsigned int> m_map_node_vtags_now;
  std::map<std::string, unsigned int> m_map_node_vtags_ever;
  std::map<std::string, unsigned int> m_map_node_vtags_hit;
  std::map<std::string, unsigned int> m_map_node_vtags_have;
  std::map<std::string, double>       m_map_node_vtag_range;

  std::list<VTag>  m_pending_vtags;

 protected: // Configuration variables
  // Config params for depth charge range and amount. These values may
  // but overridden with unique values for a particular vehicle.
  double        m_vtag_range;

  // Visual hints
  std::string   m_drop_color;
  std::string   m_hit_color;
  std::string   m_miss_color;
};

#endif 

