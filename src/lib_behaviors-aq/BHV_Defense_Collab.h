/************************************************************/
/*    NAME: Jonathan Schwartz                                              */
/*    ORGN: MIT                                             */
/*    FILE: BHV_Defense.h                                      */
/*    DATE:                                                 */
/************************************************************/

#ifndef Defense_Collab_HEADER
#define Defense_Collab_HEADER

#include <string>
#include "IvPBehavior.h"
#include "WaypointEngine.h"
#include "XYPolygon.h"
#include "NodeReport.h"

class BHV_Defense_Collab : public IvPBehavior {
public:
  BHV_Defense_Collab(IvPDomain);
  ~BHV_Defense_Collab() {};
  
  bool         setParam(std::string, std::string);
  void         onSetParamComplete();
  void         onCompleteState();
  void         onIdleState();
  void         onHelmStart();
  void         postConfigStatus();
  void         onRunToIdleState();
  void         onIdleToRunState();
  IvPFunction* onRunState();
  void getOppCoords(string);

protected: // Local Utility functions
  WaypointEngine m_waypoint_engine;
  IvPFunction* buildFunctionWithZAIC(double speed);

protected: // Configuration parameters

protected: // State variables
  double m_flagX;
  double m_flagY;
  double m_attX;
  double m_attY;
  double m_speed;
  vector<NodeReport> m_opp_list;
  double m_oppX;
  double m_oppY;
  double m_dist_from_flag;
  double m_osX;
  double m_osY;
  double m_destX;
  double m_destY;
  double m_attack_angle;
  string m_attacker;
  string m_curr_node_report;
  string m_team;
  string m_self;
  XYSegList m_points;
  bool m_move;
  double m_angle;
  double m_crit_dist;
};


#define IVP_EXPORT_FUNCTION

extern "C" {
  IVP_EXPORT_FUNCTION IvPBehavior * createBehavior(std::string name, IvPDomain domain) 
  {return new BHV_Defense_Collab(domain);}
}
#endif
