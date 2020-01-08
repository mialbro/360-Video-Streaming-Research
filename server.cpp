/*
  Author: Mark Robinson
  Last Updated: 01/08/20
*/

#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>
#include <sstream>
#include <iomanip>
#include <math.h>
#include <functional>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>

using namespace std;

void setupServer(char *clientAddress, char *serverAddress, int *sockfd, int port, struct sockaddr_in *servaddr) {
  if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }
  servaddr->sin_family = AF_INET;
  servaddr->sin_addr.s_addr = INADDR_ANY;//inet_addr(serverAddress);
  servaddr->sin_port = htons(port);

  if (bind(*sockfd, (struct sockaddr *)servaddr, sizeof(*servaddr)) < 0) {
    perror("bind failed");
  }
}

int receiveData(int sockfd, struct sockaddr_in *cliaddr, char *buffer, int bytesToRead) {
  int bytes = 0;
  socklen_t len = sizeof(*cliaddr);
  bytes = recvfrom(sockfd, (char *)buffer, bytesToRead, 0, (struct sockaddr *)cliaddr, &len);
  buffer[bytes] = '\n';
  return bytes;
}

void sendData(int sockfd, struct sockaddr_in cliaddr, char *data, int bytesToSend) {
  int bytes = 0;
  socklen_t len;
  bytes = sendto(sockfd, (const char*)data, bytesToSend, 0, (const struct sockaddr *)&cliaddr, len);
}

void readHeader(char *buffer, char *headerBuffer, char *nameBuffer, char *sizeBuffer, int hI) {
  copy(&buffer[hI], &buffer[hI + 16], &headerBuffer[0]);
  copy(&buffer[hI], &buffer[hI + 6], &nameBuffer[0]); // extract the filename from the header (gop-row-column)
  copy(&buffer[hI + 8], &buffer[hI + 16], &sizeBuffer[0]); // extract the filesize from the header
  return;
}

void receiveGops(int sockfd, struct sockaddr_in servaddr, struct sockaddr_in cliaddr) {
  char buffer[64000], nameBuffer[12], sizeBuffer[12], headerBuffer[16];
  char resp[] = "1";
  int bytesRec = 0, fileSize = 0, packetSize = 0, hI = 64000 - 16;
  string filename, header;
  // receive data
  while (1) {
    packetSize = 64000;
    bytesRec = receiveData(sockfd, &cliaddr, buffer, 64000);
    readHeader(buffer, headerBuffer, nameBuffer, sizeBuffer, hI);
    sscanf(sizeBuffer, "%d", &fileSize);  // convert filesize to string
    string str(nameBuffer);
    filename = "./received/" + str + ".bin";  // convert filename to string
    cout << "_" << headerBuffer << "_" << endl;
    ofstream file (filename, ios::out | ios::binary); // open file to begin writing
    file.write(buffer, packetSize - 16);  // write the first packet - header, to the file
    sendData(sockfd, cliaddr, resp, strlen(resp));
    bytesRec -= 16;
    // read in rest of the packets
    while (bytesRec < fileSize) {
      packetSize = 64000;
      if (fileSize - bytesRec < 64000)
        packetSize = fileSize - bytesRec;
      bytesRec += receiveData(sockfd, &cliaddr, buffer, packetSize);
      file.write(buffer, packetSize); // write packet to the file
      memset(buffer, 0, 64000); // clear the buffer
      sendData(sockfd, cliaddr, resp, strlen(resp));
    }
    file.close(); // close current file
    memset(nameBuffer, 0, 12); // clear name buffer
    memset(sizeBuffer, 0, 12);  // clear file size buffer
    memset(headerBuffer, 0, 12);
  }
  return;
}

int main() {
  char serverAddress[] = "127.0.0.1";
  char clientAddress[] = "127.0.0.1";
  int sockfd = 0, port = 8081;
  struct sockaddr_in servaddr, cliaddr;

  memset(&servaddr, 0, sizeof(servaddr));
  memset(&cliaddr, 0, sizeof(cliaddr));
  setupServer(clientAddress, serverAddress, &sockfd, port, &servaddr);
  receiveGops(sockfd, servaddr, cliaddr);

  return 0;
}
