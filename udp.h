#ifndef UDP_H_
#define UDP_H_
#include <string>
#include <netinet/in.h>
#include <sys/socket.h>

using namespace std;

class UDP {
private:
  int fd; // file descriptor
  bool pulse;
  double throughput;
  struct sockaddr_in myaddr;
  struct sockaddr_in destaddr;
public:
  UDP(char *myAddress, char *destAddress, int port, char *state);  // constructor
  int sendData(char *data, int byteCount);  // send data to destaddr
  int receiveData(char *data, int byteCount); // listen for data
  int peek(char *data, int byteCount);  // peek at buffer
  void setTp(double tp);  // set the throughput
  double getTp(); // get the through
  void kill();
  bool checkPulse();
};

#endif
