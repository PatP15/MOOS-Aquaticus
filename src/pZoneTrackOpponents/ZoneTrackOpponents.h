/************************************************************/
/*    NAME:                                               */
/*    ORGN: MIT                                             */
/*    FILE: ZoneTrackOpponents.h                                          */
/*    DATE: December 29th, 1963                             */
/************************************************************/

#ifndef ZoneTrackOpponents_HEADER
#define ZoneTrackOpponents_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "MBUtils.h"

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

 private: // Configuration variables
  std::string m_op_for;
  double m_min_x;
  double m_min_y;
  double m_max_x;
  double m_max_y;

 private: // State variables
};

#endif 
