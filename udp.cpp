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
  pulse = true; // alive
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
  printf("started\n");
}

// send data to destaddr
int UDP::sendData(char *data, int byteCount) {
  int bytes = sendto(fd, data, byteCount, 0, (struct sockaddr*)&destaddr, sizeof(destaddr));
  return bytes;
}

// listen for data. Set destaddr
int UDP::receiveData(char *data, int byteCount) {
  int bytes = 0;
  socklen_t len = 0;
  bytes = recvfrom(fd, data, byteCount, 0, (struct sockaddr*)&destaddr, &len);
  return bytes;
}

// look at the buffer
int UDP::peek(char *data, int byteCount) {
  struct sockaddr_in cliaddr;
  int bytes = 0;
  socklen_t len = 0;
  bytes = recvfrom(fd, data, byteCount, MSG_PEEK, (struct sockaddr*)&cliaddr, &len);
  return bytes;
}

// set the throughput
void UDP::setTp(double tp) {
  throughput = tp;
}

// return the throughput
double UDP::getTp() {
  return throughput;
}

void UDP::kill() {
  pulse = false;
  return;
}

bool UDP::checkPulse() {
  return pulse;
}
