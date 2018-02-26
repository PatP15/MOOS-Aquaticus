/************************************************************/
/*    NAME: Oliver                                              */
/*    ORGN: MIT                                             */
/*    FILE: Comms_client.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef Comms_client_HEADER
#define Comms_client_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

class Comms_client : public CMOOSApp
{
 public:
   Comms_client();
   ~Comms_client();

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
