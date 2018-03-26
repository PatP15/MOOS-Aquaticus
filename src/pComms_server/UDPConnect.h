//Author: Michael "Misha" Novitzky
//Date: March 26th 2018
//Origin: MIT

#ifndef UDPConnect_HEADER
#define UDPConnect_HEADER

class UDPConnect
{ 
 public:
  struct sockaddr_in my_address;
  int sock =0;
  int valread;
  struct sockaddr_in send_to_address;

  int CreateSocket();
  int BindSocket( int myPortNo, std::string myAddress);
  int SendTo( char* data, int length);
  int Receive( char * buffer, int length);
};

  #endif
