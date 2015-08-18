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


 private: // Configuration variables
   
 private: // State variables
   std::vector<double>      m_flags_x;
   std::vector<double>      m_flags_y;
   std::vector<double>      m_flags_grab_dist;
   std::vector<std::string> m_flags_ownedby;
   std::vector<std::string> m_flags_label;

   double m_default_grab_dist;
   
   
   // Holds last node report received for vehicle vname     
   std::map<std::string, NodeRecord>   m_map_record;
   std::map<std::string, double>       m_map_tstamp;
   std::map<std::string, unsigned int> m_map_rcount;

   unsigned int m_total_reports_rcvd;

};

#endif 
