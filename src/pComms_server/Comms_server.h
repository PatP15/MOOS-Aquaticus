/************************************************************/
/*    Original NAME: Oliver MacNeely                        */
/*    Updated NAME: Michael "Misha" Novitky                 */
/*    ORGN: MIT                                             */
/*    FILE: Comms_server.h                                  */
/*    DATE: March 19th 2018                                 */
/************************************************************/

#ifndef Comms_server_HEADER
#define Comms_server_HEADER

#include "MOOS/libMOOS/Thirdparty/AppCasting/AppCastingMOOSApp.h"

#include<iostream>
#include<arpa/inet.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include <portaudio.h>
#include <sndfile.h>
#include <netdb.h>
#include <netinet/in.h>
#include <vector>
#include <sys/poll.h>

#define FRAMES_PER_BUFFER (256)
#define NUM_CHANNELS (1)
#define SAMPLE_RATE (44100)
typedef short SAMPLE;
#define PA_SAMPLE_TYPE paInt16

class Comms_server : public AppCastingMOOSApp
{
 public:
   Comms_server();
   ~Comms_server();

 protected: // Standard MOOSApp functions to overload
   bool OnNewMail(MOOSMSG_LIST &NewMail);
   bool Iterate();
   bool OnConnectToServer();
   bool OnStartUp();
  bool buildReport();

 protected:
   void RegisterVariables();

 private: // Configuration variables
  int m_ServerSocket;
  std::string m_ServerIp;

 private: // State variables
  bool m_GoodState;
};

#endif