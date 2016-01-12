/************************************************************/
/*    NAME: Hugh Dougherty                                              */
/*    ORGN: MIT                                             */
/*    FILE: Blinkstick.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef Blinkstick_HEADER
#define Blinkstick_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class Blinkstick : public CMOOSApp
{
 public:
   Blinkstick();
   ~Blinkstick();

 protected:
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
   void RegisterVariables();

 private: // Configuration variables

 private: // State variables
   unsigned int m_iterations;
   double       m_timewarp;
};

#endif 
