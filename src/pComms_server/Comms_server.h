/************************************************************/
/*    NAME: Oliver                                              */
/*    ORGN: MIT                                             */
/*    FILE: Comms_server.h                                          */
/*    DATE:                                                 */
/************************************************************/

#ifndef Comms_server_HEADER
#define Comms_server_HEADER

#include "MOOS/libMOOS/MOOSLib.h"

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

class Comms_server : public CMOOSApp
{
 public:
   Comms_server();
   ~Comms_server();

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
