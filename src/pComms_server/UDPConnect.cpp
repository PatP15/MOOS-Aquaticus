#include "UDPConnect.h"

UDPConnect::UDPConnect()
{
  sock = 0;
}

UDPConnect::~UDPConnect()
{
  
}

//Needed for both sending and receiving UDP 
int UDPConnect::CreateSocket()
{
  sock = socket( AF_INET, SOCK_DGRAM,IPPROTO_UDP);
  if( sock < 0){
    cout << endl << "Socket creation error" << endl;
    return -1;
  }
  else {
    return 1;
  }
}

//Only needed when waiting to use receive from a socket
int UDPConnect::BindSocket(int myPortNo, std::string myAddress)
{
  memset((char *)&my_address, 0, sizeof(my_address));
  my_address.sin_family = AF_INET;
  my_address.sin_port = htons(myPortNo);
  my_address.sin_addr.s_addr = inet_addr(myAddress.c_str());

  if(bind(sock, (struct sockaddr *) &my_address, sizeof(my_address)) < 0) {
    cout << endl << "Bind Failed!" << endl;
    return -1;
  }
  else {
    cout << endl << "Bind Success!" << endl;
    return 1;
  }
}

int UDPConnect::SendTo(char* data, int length, int destPortNo, std::string destIP)
{
  struct sockaddr_in dest_address;
  memset((char *)& dest_address, 0, sizeof(dest_address));
  dest_address.sin_family = AF_INET;
  dest_address.sin_port = htons(destPortNo);
  dest_address.sin_addr.s_addr = inet_addr(destIP.c_str());

  if(sendto(sock, data, length, 0, (struct sockaddr *) &dest_address, sizeof(dest_address)) < 0) {
      cout << endl << "error sending message" << endl;
      return -1;
    }
    else {
        cout << endl << "message sent" << endl;
        return 1;
      }
}

int UDPConnect::Receive( char * buffer, int length)
{
  int lengthReceived;
  struct sockaddr_in remoteAddress;
  socklen_t remoteAddresslength = sizeof(remoteAddresslength);

  lengthReceived = recvfrom(sock, buffer, length, 0, (struct sockaddr * ) &remoteAddress, &remoteAddresslength);
  if(lengthReceived<0){
    cout << endl << "Error Receiving Message" << endl;
    return -1;
  }
  else {
    return lengthReceived;
  }
}

