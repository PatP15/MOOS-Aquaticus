/************************************************************/
/*   NAME: Mike Benjamin                                    */
/*   ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA */
/*   FILE: FlagManager.h                                    */
/*   DATE: August 18th, 2015                                */
/************************************************************/

#ifndef FLAG_MANAGER_HEADER
#define FLAG_MANAGER_HEADER

#include <string>
#include <vector>
#include <set>
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"
#include "XYMarker.h"

class FlagManager : public AppCastingMOOSApp
{
 public:
   FlagManager();
   ~FlagManager() {};

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload
   bool buildReport();

 protected:
   void registerVariables();
   bool handleConfigFlag(std::string);
   bool handleMailNodeReport(std::string str);
   bool handleMailFlagGrab(std::string, std::string);
   bool handleMailFlagReset(std::string);
   bool handleMailTaggedVehicles(std::string);

   bool resetFlagsByLabel(std::string);
   bool resetFlagsByVName(std::string);
   bool resetFlagsAll();
   void postFlagMarkers();
   void postFlagSummary();

 private: // Config variables

   // Flag Configurations
   std::vector<XYMarker>  m_flags;
   std::vector<bool>      m_flags_changed;

   double      m_default_flag_range;
   double      m_default_flag_width;
   std::string m_default_flag_type;
   bool        m_report_flags_on_start;
   std::string m_grabbed_color;
   std::string m_ungrabbed_color;

 private: // State variables

   // Vehicle node report state vars. Each map keyed on vname.
   std::map<std::string, NodeRecord>   m_map_record;
   std::map<std::string, double>       m_map_tstamp;
   std::map<std::string, unsigned int> m_map_rcount;
   std::map<std::string, unsigned int> m_map_grab_count;
   std::map<std::string, unsigned int> m_map_flag_count;
   unsigned int m_total_node_reports_rcvd;

   std::set<std::string>  m_tagged_vnames;
   
   // Grab request state vars
   unsigned int m_total_grab_requests_rcvd;

};

#endif
