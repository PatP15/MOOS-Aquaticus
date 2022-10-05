/************************************************************/
/*    NAME: Michael Novitzky                                */
/*    ORGN: MIT/USMA                                        */
/*    FILE: ZoneTrackOpponents.h                            */
/*    UPDATE: June 12th, 2022                               */
/************************************************************/

#ifndef ZoneTrackOpponents_HEADER
#define ZoneTrackOpponents_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MBUtils.h"
#include "NodeRecord.h"

class ZoneTrackOpponents : public AppCastingMOOSApp
{
 public:
   ZoneTrackOpponents();
   ~ZoneTrackOpponents();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
  bool handleOpForAssignment(std::string orig);
  bool handleZoneAssignment(std::string orig);
  bool handlePointsAssignment(std::string orig);
  bool handleHighValuePoint(std::string value);
  void handleMailNodeReport(std::string report);

 private: // Configuration variables
  std::string m_ownship;
  std::string m_op_for;
  double m_min_x;
  double m_min_y;
  double m_max_x;
  double m_max_y;
  double theta_1_a;
  double theta_1_b;
  double theta_2_a;
  double theta_2_b;
  double theta_3_a;
  double theta_3_b;
  double theta_4_a;
  double theta_4_b;
  double theta_1;
  double theta_2;
  double theta_3;
  double theta_4;
  double x1;
  double y1;
  double x2;
  double y2;
  double x3;
  double y3;
  double x4;
  double y4;
  std::string m_in_zone;
  bool m_high_value_point_set;
  double m_high_value_point_x;
  double m_high_value_point_y;
  std::string m_contact_track;

 private: // State variables
  std::map<std::string, NodeRecord> m_map_node_records;
  std::map<std::string, double  > m_map_intruders_x;
  std::map<std::string, double  > m_map_intruders_y;
  std::map<std::string, std::string> m_map_intruders_name;
};

#endif 
