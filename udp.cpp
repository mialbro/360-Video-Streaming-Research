#include <iostream>
#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include "udp.h"

using namespace std;

UDP::UDP(char *myAddress, char *destAddress, int myPort, int destPort) {
  if ((fd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("cannot create socket");
  }
  // set device info
  memset((char *)&myaddr, 0, sizeof(myaddr));
  myaddr.sin_family = AF_INET;
  myaddr.sin_addr.s_addr = inet_addr(myAddress);
  myaddr.sin_port = htons(myPort);
  if (bind(fd, (struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
    perror("bind failed");
  }
  // set destination info
  destaddr.sin_family = AF_INET;
  destaddr.sin_addr.s_addr = inet_addr(destAddress);
  destaddr.sin_port = htons(destPort);
  throughput = 0.0;
  printf("client created\n");
}

int UDP::sendData(char *data, int byteCount) {
  int bytes = sendto(fd, data, byteCount, 0, (struct sockaddr*)&destaddr, sizeof(destaddr));
  return bytes;
}

int UDP::receiveData(char *data, int byteCount) {
  struct sockaddr_in cliaddr;
  int bytes = 0;
  socklen_t len = 0;
  //bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr * ) & cliaddr, & len);
  bytes = recvfrom(fd, data, byteCount, 0, (struct sockaddr*)&cliaddr, &len);
  return bytes;
}

int UDP::peek(char *data, int byteCount) {
  struct sockaddr_in cliaddr;
  int bytes = 0;
  socklen_t len = 0;
  bytes = recvfrom(fd, data, byteCount, MSG_PEEK, (struct sockaddr*)&cliaddr, &len);
  return bytes;
}
