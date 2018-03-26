//Author: Michael "Misha" Novitzky
//Date: March 26th 2018
//Origin: MIT

#ifndef ClientConnec_HEADER
#define ClientConnec_HEADER

class ClientConnec
{ 
 public:
  struct sockaddr_in address;
  int sock =0;
  int valread;
  struct sockaddr_in serv_addr;

  int CreateSocket();
  int SetParams(int portNo, std::string server_address);
  int Connect();
  int Send(char* data, int length);
  int Read( char * buffer, int length);
};

  #endif
