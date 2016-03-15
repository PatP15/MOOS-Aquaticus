/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ZoneEvent.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef ZoneEvent_HEADER
#define ZoneEvent_HEADER

#include <vector>
#include <string>
#include <map>
#include <set>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"
#include "XYRangePulse.h"
#include "XYPolygon.h"

class ZoneEvent : public AppCastingMOOSApp
{
 public:
  ZoneEvent();
  ~ZoneEvent() {}

 protected:  // Standard MOOSApp functions to overload
  bool OnNewMail(MOOSMSG_LIST &NewMail);
  bool Iterate();
  bool OnConnectToServer();
  bool OnStartUp();

 protected:  // Config utilities
  bool handleConfigZone(std::string);
  bool handleConfigPostVar(std::string);
  bool handleConfigGroupName(std::string);
  bool handleConfigVehicleName(std::string);
  bool postZonePoly();

 protected:
  bool onNodeReport(CMOOSMsg&);
  bool checkNodeInZone(NodeRecord&);
  bool onDBUPTIME(CMOOSMsg&);

 protected:  // Standard AppCastingMOOSApp function to overload
  bool buildReport();

 protected:
  void registerVariables();

 protected:  // Configuration variables
  XYPolygon   m_zone;

  std::string m_zone_name;
  std::string m_zone_color;
  std::string m_vname;

  enum ReturnPostVal { rvname, rgroup, rtime, rvx, rvy, rstatic };
  std::map<std::string, ZoneEvent::ReturnPostVal> m_map_var_val;
  std::map<std::string, std::string> m_map_static_var_val;

  std::map<std::string, NodeRecord> m_map_node_records;

  CMOOSLock* p_events_w_lock;
  std::vector<std::string> m_events;

  double m_dbtime;
};

#endif
