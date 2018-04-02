/************************************************************/
/*    ORIGINAL NAME: Oliver MacNeely                        */
/*    NAME: Michael "Misha" Novitzky                        */
/*    ORGN: MIT                                             */
/*    FILE: Comms_client.h                                  */
/*    DATE: March 21 2018                                   */
/************************************************************/

#ifndef Comms_client_HEADER
#define Comms_client_HEADER

#include  "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

class Comms_client : public AppCastingMOOSApp
{
 public:
   Comms_client();
   ~Comms_client();

 protected: // Standard MOOSApp functions to overload  
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
  bool buildReport(); 

protected:
   void RegisterVariables();

 private: // Configuration variables
  int m_ClientSocket;
  std::string m_ClientIP;
  int m_ServerSocket;
  std::string m_ServerIP;
  std::string m_ListenForMOOSVar;
  std::string m_ListenForMOOSValue;

 private: // State variables
  bool m_GoodState;
  bool m_SendAudio;
  bool m_Transmitting;
  bool m_Receiving; 
};

#endif 
