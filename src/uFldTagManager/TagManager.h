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
  bool    handleVTagStatusRequest(const std::string&);
  bool    handleReplenishRequest(const std::string&);

 protected: // Outgoing mail utility
  void    postVTagLaunch(const std::string&, const std::string&, double rng); 
  void    postVTagHit(const std::string&, const std::string&, double rng); 
  void    postRangePulse(double x, double y, const std::string& color,
			 const std::string& label, double dur, double radius, 
			 double linger=0);
  bool    postReplenishRules();
  void    postReplenishStatus(const std::string&);

 protected: // Utilities
  bool    setVTagConfig(std::string);
  bool    setVTagRange(std::string);
  bool    setVTagAmount(std::string);
  bool    setReplenishTime(std::string);
  bool    setReplenishRange(std::string);
  bool    setReplenishStation(std::string);
  bool    setVTagDelayMax(std::string);
  bool    setVTagDefault(std::string);

  double  getTrueNodeRange(double, double, const std::string&);
  double  getNoisyNodeRange(double true_range) const;

  double  rangeToReplenishStation(std::string vname);

  unsigned int getVTagsRemaining(std::string) const;
  bool    isUUV(std::string&);

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
  std::map<std::string, bool>         m_map_node_replenish_req;
  std::map<std::string, double>       m_map_node_replenish_req_time;

  std::list<VTag>  m_pending_charges;

 protected: // Configuration variables
  // Config params for depth charge range and amount. These values may
  // but overridden with unique values for a particular vehicle.
  double        m_vtag_range_default;
  unsigned int  m_vtag_amount_default;

  // Config params for depth charge delays. Cannot be overridden with 
  // unique values per vehicle - applies to all vehicles.
  double        m_vtag_delay_max;
  double        m_vtag_delay_default;

  // Config params for replenishment. Applies equally to all vehicles.
  double m_replenish_x;
  double m_replenish_y;
  double m_replenish_range;
  double m_replenish_time;
  
  // Visual hints
  std::string   m_drop_color;
  std::string   m_detonate_color;
  std::string   m_hit_color;
};

#endif 

