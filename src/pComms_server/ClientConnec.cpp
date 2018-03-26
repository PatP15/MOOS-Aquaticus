#include "ClientConnec.h"

ClientConnec::ClientConnec()
{
  
}

ClientConnec::~ClientConnec()
{
  
}


int ClientConnec::CreateSocket()
{
  sock = socket( AF_INET, DOCK_DGRAM,IPPROTO_UDP);
  if( sock < 0){
    cout << endl << "Socket creation error" << endl;
    return -1;
  }
  else {
    return 1;
  }
}

int ClientConnec::SetParams(int portNo, string server_address)
{
  memset(&serv_addr, '0', sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(portNo);

  //check proper address and assign
  if(inet_pton(AF_INET, server_address, &serv_addr.sin_addr)<=0) {
    cout << endl << "Invalid address" << endl;
    return -1;
  }
  else {
    return 1;
  }

}
int ClientConnec::Connect()
{
  if(connect(sock, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0) {
    cout << endl << "Connection Failed" << endl;
    return -1;
  }
}
int ClientConn::Send(char* data, int length)
{
  return send(sock, data, length, 0);
}
int ClientConnec::Read( char * buffer, int length)
{
  return read(sock, buffer, length);
}
}
