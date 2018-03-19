/************************************************************/
/*    NAME: Oliver                                              */
/*    ORGN: MIT                                             */
/*    FILE: Comms_server.cpp                                        */
/*    DATE:                                                 */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "Comms_server.h"
#include <cstdlib>   // for the strtoul() function


using namespace std;

char ipstr[INET6_ADDRSTRLEN]; // string that contains the ip of the connected client
int port; // int that contains the port of the connected client

std::vector<unsigned short int> clients; // vector of the ports of clients connected to the server
std::vector<std::string> ips; // vector of the strings of clients

struct AudioBuffer { // structure that contains the information for each packet of audio

  short *recording; // pointer to packet data itself
  size_t size; // size of packet
  size_t recorded_size; // size of recording
  short *recordedSamples; // pointer to beginning of collated recordings

} AudioBuffer;

// initialize necessary variables

int message_counter = 0;
int port_counter = 0;

bool pushBACK = false;

// initialize structure for holding audio data

struct AudioBuffer buffer = {(short*) malloc(FRAMES_PER_BUFFER* sizeof(short)*NUM_CHANNELS), FRAMES_PER_BUFFER* sizeof(short)*NUM_CHANNELS, 0, NULL};

// initialize socket information for receiving data

// structures for holding server and client data for receiving data

struct sockaddr_in server = {AF_INET, htons(11111), inet_addr("18.111.102.119")}; // hardcoded setup for server socket

struct sockaddr_in client;


int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // create socket for receiving data

int binding = bind(sock,( struct sockaddr *) &server, sizeof(server)); // bind the socket

socklen_t l = sizeof(client);

//initialize socket information for sending data

struct sockaddr_in sender; // structures for holding server and client data for sending data

int socket_sender = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // create socket for sending data

void * set_size = memset(&sender, 0, sizeof(sender));



//---------------------------------------------------------
// Constructor

Comms_server::Comms_server()
{
}

//---------------------------------------------------------
// Destructor

Comms_server::~Comms_server()
{
}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Comms_server::OnNewMail(MOOSMSG_LIST &NewMail)
{
  MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;

#if 0 // Keep these around just for template
    string key   = msg.GetKey();
    string comm  = msg.GetCommunity();
    double dval  = msg.GetDouble();
    string sval  = msg.GetString();
    string msrc  = msg.GetSource();
    double mtime = msg.GetTime();
    bool   mdbl  = msg.IsDouble();
    bool   mstr  = msg.IsString();
#endif
  }
   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Comms_server::OnConnectToServer()
{
   // register for variables here
   // possibly look at the mission file?
   // m_MissionReader.GetConfigurationParam("Name", <string>);
   // m_Comms.Register("VARNAME", 0);

   RegisterVariables();
   return(true);
}

//---------------------------------------------------------
// Procedure: Iterate()
//            happens AppTick times per second

bool Comms_server::Iterate()
{

  pushBACK = false; // assume we shouldn't add the first connected port to list of clients

  recvfrom(sock, buffer.recording, buffer.size, 0, (struct sockaddr *) &client, &l); // receive audio from connected client


  // check if ip4 or ip6 for grabbing port + ip address
  if (client.sin_family == AF_INET) {
    struct sockaddr_in *s = (struct sockaddr_in *)&client;
    port = ntohs(s->sin_port);
    inet_ntop(AF_INET, &s->sin_addr, ipstr, sizeof ipstr);
  } else { // AF_INET6
    struct sockaddr_in6 *s = (struct sockaddr_in6 *)&client;
    port = ntohs(s->sin6_port);
    inet_ntop(AF_INET6, &s->sin6_addr, ipstr, sizeof ipstr);
  }


  if (message_counter == 0) { // if this is the first time, add the client to list

        clients.push_back(port);
        ips.push_back(ipstr);

        port_counter++;

      } else { // if not, iterate through list of clients and check to see whether connected client is there

        for (int i = 0; i < port_counter; i++) {

          if (clients[i] == port) {

            cout << "Same port!" << endl;
            cout << "saved port: " << clients[i] << endl;
            cout << "new port: " << port << endl;

            pushBACK = false; // if the client is there, don't add to list

            break;

          } else {

            pushBACK = true; // if the client isn't there, add to list

          }

        }

        if (pushBACK == true) { // depending on value, add client to list of conected clients

          clients.push_back(port);
          ips.push_back(ipstr);

          port_counter++;

        }

      }

  for (int i = 0; i < port_counter; i++) {

    if (clients[i] == port) {

      cout << "I continued, port = " << clients[i] << endl;

    } else {

      // setup a information to send the audio for each client that isn't the one that send the audio
      sender.sin_family = AF_INET;
      sender.sin_port = htons(11112);
      sender.sin_addr.s_addr = inet_addr(ips[i].c_str());

      bind(socket_sender, (struct sockaddr *) &sender, sizeof(sender));

      socklen_t p = sizeof(sender);

      sendto(socket_sender, buffer.recording , buffer.size, 0, (struct sockaddr *) &sender, p);

    }

  }

  message_counter++;

  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Comms_server::OnStartUp()
{
  list<string> sParams;
  m_MissionReader.EnableVerbatimQuoting(false);
  if(m_MissionReader.GetConfiguration(GetAppName(), sParams)) {
    list<string>::iterator p;
    for(p=sParams.begin(); p!=sParams.end(); p++) {
      string original_line = *p;
      string param = stripBlankEnds(toupper(biteString(*p, '=')));
      string value = stripBlankEnds(*p);

      if(param == "FOO") {
        //handled
      }
      else if(param == "BAR") {
        //handled
      }
      else if(param == "ServerSocket") {
          uint64_t new_value = strtoul(value.c_str(), NULL, 0);
          m_ServerSocket = new_value;
      }
      else if(param == "ServerIP") {
        m_ServerIp = value; 
      }

    }
  }

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void Comms_server::RegisterVariables()
{
  // Register("FOOBAR", 0);
}
