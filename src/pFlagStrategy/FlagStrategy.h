/************************************************************/
/*   NAME: Mike Benjamin                                    */
/*   ORGN: Dept of Mechanical Eng / CSAIL, MIT Cambridge MA */
/*   FILE: FlagStrategy.h                                   */
/*   DATE: August 18th, 2015                                */
/************************************************************/

#ifndef FLAG_STRATEGY_HEADER
#define FLAG_STRATEGY_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"
#include "XYMarker.h"

class FlagStrategy : public AppCastingMOOSApp
{
 public:
   FlagStrategy();
   ~FlagStrategy() {};

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected: // Standard AppCastingMOOSApp function to overload 
   bool buildReport();

 protected:
   void registerVariables();
   bool handleMailFlagSummary(std::string);

   bool flagsMatch(const XYMarker&, const XYMarker&) const;

   
 private: // Configuration variables
   
 private: // State variables
   
   unsigned int m_flag_summaries_received;
   double       m_flag_summary_tstamp;

   std::vector<XYMarker>    m_flags;
};

#endif 
