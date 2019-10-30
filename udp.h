#ifndef UDP_H_
#define UDP_H_
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

class UDP {
private:
  int fd;
  double throughput;
  struct sockaddr_in myaddr, destaddr;
public:
  UDP(char *myAddress, char *destAddress, int myPort, int destPort);
  int sendData(char *data, int byteCount);
  int receiveData(char *data, int byteCount);
  int peek(char *data, int byteCount);
};

#endif
