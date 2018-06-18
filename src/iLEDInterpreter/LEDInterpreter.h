/************************************************************/
/*    NAME: Caileigh Fitzgerald                                              */
/*    ORGN: MIT                                             */
/*    FILE: LEDInterpreter.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef LEDInterpreter_HEADER
#define LEDInterpreter_HEADER

#include "MOOS/libMOOS/MOOSLib.h"
#include "iLEDInfoBar_enums.h"

class LEDInterpreter : public CMOOSApp
{
 public:
   LEDInterpreter();
   ~LEDInterpreter();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();

 protected:
   void RegisterVariables();

 private: // Configuration variables

 private: // State variables
};

#endif 
