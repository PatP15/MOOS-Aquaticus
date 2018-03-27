/************************************************************/
/*    Original NAME: Oliver MacNeely                        */
/*    NAME: Michael "Misha" Novitzky                        */
/*    ORGN: MIT                                             */
/*    FILE: Comms_client.cpp                                */
/*    DATE: March 21 2018                                   */
/************************************************************/

#include <iterator>
#include "MBUtils.h"
#include "Comms_client.h"

#include <iostream>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <portaudio.h>
#include "sndfile.h"
#include <sys/poll.h>
#include "UDPConnect.h"

// these define the variables needed for initializing audio transmission and handling
#define FRAMES_PER_BUFFER (256)
#define NUM_CHANNELS (1)
#define SAMPLE_RATE (44100)
typedef short SAMPLE;
#define PA_SAMPLE_TYPE paInt16

using namespace std;

size_t received_size = FRAMES_PER_BUFFER *sizeof(short) * NUM_CHANNELS; // size of received recording from server

double moos_value; // value from moos variable that decides whether to record or not, currently only records when true

struct AudioBuffer { // structure that holds audio data before being saved

  short *recording; // pointer to recording address
  size_t size; // size of received packet
  size_t recorded_size; // total size of recording
  short *recordedSamples; // pointer to all recorded samples so far

} AudioBuffer;

int message_counter = 1;

PaStream *stream = NULL;

PaError err = Pa_Initialize(); // initialize stream

// create pointers for output info and input info
const PaDeviceInfo *inputInfo = Pa_GetDeviceInfo(Pa_GetDefaultInputDevice());
const PaDeviceInfo *outputInfo = Pa_GetDeviceInfo(Pa_GetDefaultOutputDevice());

// standard structure for holding parameters
PaStreamParameters inputParameters =
  {

    .device = Pa_GetDefaultInputDevice(),
    .channelCount = 1,
    .sampleFormat = paInt16,
    .suggestedLatency = inputInfo->defaultLowInputLatency,
    .hostApiSpecificStreamInfo = NULL

  };

PaStreamParameters outputParameters =
  {

    .device = Pa_GetDefaultOutputDevice(),
    .channelCount = 1,
    .sampleFormat = paInt16,
    .suggestedLatency = outputInfo->defaultLowOutputLatency,
    .hostApiSpecificStreamInfo = NULL

  };


// create a stream and start it
PaError open_stream = Pa_OpenStream(&stream, &inputParameters, &outputParameters, SAMPLE_RATE, FRAMES_PER_BUFFER, paClipOff, NULL, NULL);
PaError start_stream = Pa_StartStream(stream);


//allocate memory for the buffer of audio to be sent
struct AudioBuffer buffer = {(short*) malloc (FRAMES_PER_BUFFER * sizeof(short) * NUM_CHANNELS), FRAMES_PER_BUFFER * sizeof(short) * NUM_CHANNELS, 0, NULL};

//allocate memory for buffer of audio that is received
short *received_recording = (short*) malloc(FRAMES_PER_BUFFER * sizeof(short) * NUM_CHANNELS);

//sending data as a client to the server

struct sockaddr_in server = {AF_INET, htons(11111), inet_addr("18.111.102.119")}; // server's port and address (hardcoded right now)

struct sockaddr_in client; // create client

void *set_memory = memset(&server, 0, sizeof(server)); // set memory for the server

int sock = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // create socket

int binding_client = bind(sock, (struct sockaddr *) &server, sizeof(server)); // bind client to socket

socklen_t m = sizeof(server);

//receving data as a server, when the client receives data from the sever, it acts as a server

int sock_ss = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP); // create socket for receiving data

struct sockaddr_in server_ss = {AF_INET, htons(11112), inet_addr("18.111.102.119")}; //own port and own address

struct sockaddr_in client_ss; // set up client to receive data from

int binding_server = bind(sock_ss, (struct sockaddr *) &server_ss, sizeof(server_ss)); // bind the server to socket

socklen_t q = sizeof(client_ss); //variable for the size of the client

struct pollfd ufds[1] = // set up polling so that the client isn't waiting to receive data from the server
  {

    ufds[0].fd = sock_ss,
    ufds[0].events = POLLIN

  };
int rv; // polling variable




//---------------------------------------------------------
// Constructor

Comms_client::Comms_client()
{
}

//---------------------------------------------------------
// Destructor

Comms_client::~Comms_client()
{

  // free recorded audio buffer
  free(buffer.recordedSamples);
  buffer.recordedSamples = NULL;

  Pa_Terminate();

}

//---------------------------------------------------------
// Procedure: OnNewMail

bool Comms_client::OnNewMail(MOOSMSG_LIST &NewMail)
{
  AppCastingMOOSApp::OnNewMail(NewMail);
 MOOSMSG_LIST::iterator p;

  for(p=NewMail.begin(); p!=NewMail.end(); p++) {
    CMOOSMsg &msg = *p;


    string key   = msg.GetKey();
    moos_value  = msg.GetDouble(); // grab variable from MOOS


   }

   return(true);
}

//---------------------------------------------------------
// Procedure: OnConnectToServer

bool Comms_client::OnConnectToServer()
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

bool Comms_client::Iterate()
{
  AppCastingMOOSApp::Iterate();

  PaError read_stream = Pa_ReadStream(stream, buffer.recording, FRAMES_PER_BUFFER); // read audio from the mic
  sendto(sock, buffer.recording, buffer.size, 0, (struct sockaddr *) &server, m); // send this audio to the server

  rv = poll(ufds, 1, 1); // check to see if there is data from the server

  if (rv != 0) { // if there is data, receive it

    recvfrom(sock_ss, received_recording, received_size, 0, (struct sockaddr *) &client_ss, &q);

  }

  PaError write_stream = Pa_WriteStream(stream, received_recording, FRAMES_PER_BUFFER); // write received data to speaker

  buffer.recordedSamples = (short *) realloc(buffer.recordedSamples, buffer.size * message_counter); // enlarge buffer for new recording to be appended

  buffer.recorded_size = buffer.size * (message_counter); // increase size of buffer for next recording

  memcpy((char *) buffer.recordedSamples + ((message_counter - 1) * buffer.size), buffer.recording, buffer.size); // append data from audio buffer to recording buffer

  if(moos_value) { // if signalled, dump audio buffer to .wav file

    char filename[50];

    snprintf(filename, 100, "file:%d.wav", message_counter); // create filename from messages received so far

    // standard structure for holding parameters for writing audio
    SF_INFO sfinfo =
      {

        sfinfo.channels = 1,
        sfinfo.samplerate = 44100,
        sfinfo.format = SF_FORMAT_WAV | SF_FORMAT_PCM_16

      };

    // open .wav file
    SNDFILE *outfile = sf_open(filename, SFM_WRITE, &sfinfo);

    // write audio to .wav file
    long wr = sf_writef_short(outfile, buffer.recordedSamples, buffer.recorded_size / sizeof(short));

    // clean up and close file
    sf_write_sync(outfile);
    sf_close(outfile);

    // free buffer that recorded audio packet
    free(buffer.recordedSamples);
    buffer.recordedSamples = NULL;

  }

  message_counter++;


  AppCastingMOOSApp::PostReport();
  return(true);
}

//---------------------------------------------------------
// Procedure: OnStartUp()
//            happens before connection is open

bool Comms_client::OnStartUp()
{
  AppCastingMOOSApp::OnStartUp();

  //boolean value to make sure all 4 critical values are set
  bool server_sock_param_set = false;
  bool server_ip_param_set = false;
  bool client_sock_param_set = false;
  bool client_ip_param_set = false;

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
      else if(param == "SERVERSOCKET") {
        uint64_t  converted_value = strtoul(value.c_str(), NULL, 0);
        m_ServerSocket = converted_value;
        server_sock_param_set = true;
      }
      else if(param == "SERVERIP") {
        m_ServerIP = value;
        server_ip_param_set = true;
      }
      else if(param == "CLIENTSOCKET") {
        uint64_t  converted_value = strtoul(value.c_str(), NULL, 0);
        m_ClientSocket = converted_value;
        client_sock_param_set = true;
      }
      else if(param == "CLIENTIP") {
        m_ClientIP = value;
        client_ip_param_set = true;
      }
    }
  }

  //check that all important params have been set
  if(!(server_sock_param_set && server_ip_param_set && client_ip_param_set && client_sock_param_set )) {
    reportConfigWarning("Not all necessary parameters were set for Client and Server IP and Socket.");
  }

  RegisterVariables();
  return(true);
}

//---------------------------------------------------------
// Procedure: RegisterVariables

void Comms_client::RegisterVariables()
{
  AppCastingMOOSApp::RegisterVariables();

  Register("SAVE", 0);
}

bool Comms_client::buildReport()
{
  m_msgs << "============================================ \n";
  m_msgs << "Client:                                        \n";
  m_msgs << "============================================ \n";

  m_msgs << "    Client IP: " << m_ClientIP << endl;
  m_msgs << "Client Socket: " << m_ClientSocket << endl;
  m_msgs << endl;
  m_msgs << "    Server IP: " << m_ServerIP << endl;
  m_msgs << "Server Socket: " << m_ServerSocket << endl;

  return(true);
}
