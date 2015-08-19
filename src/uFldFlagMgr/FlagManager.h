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
#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "NodeRecord.h"

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
   void postFlagMarkers();
   void postFlagSummary();
   
 private: // Config variables

   // Flag Configuration
   std::vector<double>      m_flags_x;
   std::vector<double>      m_flags_y;
   std::vector<double>      m_flags_grab_dist;
   std::vector<std::string> m_flags_ownedby;
   std::vector<std::string> m_flags_label;
   double m_default_grab_dist;
   
   bool   m_report_flags_on_start;
     
 private: // State variables
 
   // Vehicle node report state vars
   std::map<std::string, NodeRecord>   m_map_record;
   std::map<std::string, double>       m_map_tstamp;
   std::map<std::string, unsigned int> m_map_rcount;
   std::map<std::string, unsigned int> m_map_grab_count;
   std::map<std::string, unsigned int> m_map_flag_count;
   unsigned int m_total_reports_rcvd;

   // Grab request state vars
   unsigned int m_total_grabs_rcvd;
};

#endif 
