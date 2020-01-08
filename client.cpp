#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>
#include <sstream>
#include <iomanip>
#include <thread>
#include <math.h>
#include <functional>
#include <time.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>

#include "gop.h"

#define GOP_COUNT 10
#define TILE_COUNT 64
#define BW_COUNT 4

using namespace std;

void setupClient(char *clientAddress, char *serverAddress, int *sockfd, int port, struct sockaddr_in *servaddr) {
  if ((*sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
    perror("socket creation failed");
    exit(EXIT_FAILURE);
  }
  servaddr->sin_family = AF_INET;
  servaddr->sin_port = htons(port);
  servaddr->sin_addr.s_addr = INADDR_ANY;//inet_addr(serverAddress);
}

void sendData(int sockfd, struct sockaddr_in servaddr, char *data, int bytesToSend) {
  sendto(sockfd, (const char*)data, bytesToSend, 0, (const struct sockaddr*)&servaddr, sizeof(servaddr));
  return;
}

int receiveData(int sockfd, struct sockaddr_in servaddr, char *buffer, int bytesToRead) {
  int bytes = 0;
  socklen_t len = 0;
  bytes = recvfrom(sockfd, (char *)buffer, bytesToRead, 0, (struct sockaddr *) &servaddr, &len);
  buffer[bytes] = '\n';
  return bytes;
}

void getInstr(string filename, GOP *gop) {
  int value = 0;
  ifstream inf (filename);
  for (int i = 0; i < GOP_COUNT; i++) {
    gop[i].setGop(i);
    for (int j = 0; j < BW_COUNT; j++) {
      inf >> value;
      gop[i].setBw(j, value);
      value = 0;
      for (int k = 0; k < TILE_COUNT; k++) {
        inf >> value;
        gop[i].setTile(j, k, k, value);
        value = 0;
      }
    }
    gop[i].sortTiles(); // sort the rows by importance
    gop[i].setRowCol(); // create row and column arrays
    gop[i].setFilenames();  // set files
  }
}


// add the header: gop#-row-colum to the end of the first packet
void appendHeader(char *buffer, string header, int packetSize) {
  char headerArr[header.length()];
  header.copy(headerArr, header.length(), 0);
  for (int i = 0; i < header.length(); i++) {
    buffer[packetSize + i] = headerArr[i];
  }
}

// read in binary file
void sendFile(int sockfd, struct sockaddr_in servaddr, string filename, string header, int fileSize, double *throughput) {
  char dataBuffer[64000], respBuffer[64000];
  char resp[] = "1";
  int bytesRead = 0, packetSize = 0;
  ifstream inFile(filename, ios::in | ios::binary); // open file to read
  time_t startTime, endTime;
  double sendTime = 0.0;
  startTime = time(NULL);
  // read the file
  while (bytesRead < fileSize) {
    memset(dataBuffer, 0, 64000); // clear buffer
    packetSize = 64000;
    if (fileSize - bytesRead < 64000) // if near the end of the file
      packetSize = fileSize - bytesRead;
    // add header to buffer
    if (bytesRead == 0) {
      inFile.read(dataBuffer, packetSize - header.length()); // make space for header
      appendHeader(dataBuffer, header, packetSize - header.length()); // add header
      bytesRead = packetSize - header.length();
    }
    else {
      inFile.read(dataBuffer, packetSize); // make space for header
      bytesRead += packetSize;
    }
    sendData(sockfd, servaddr, dataBuffer, packetSize);
    receiveData(sockfd, servaddr, respBuffer, strlen(resp));
  }
  inFile.close(); // close the file
  endTime = time(NULL);
  sendTime = difftime(endTime, startTime);
  if (sendTime > 0)
    *throughput = (fileSize * pow(8.0, -6.0)) / (sendTime);
  else
    *throughput = 0;
  return;
}

void sendGops(int sockfd, struct sockaddr_in servaddr, GOP gop[]) {
  double throughput = 0.0;
  int gopRow = 0, filesize = 0, tileValue = 0;
  string filename, header;
  for (int i = 0; i < GOP_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      gopRow = gop[i].selGopRow(throughput);  // select which tile row to send (corresponds to throughput value)
      tileValue = gop[i].getValue(j, gopRow); // get the tile's value
      if (tileValue != 100) {
        filename = gop[i].getFilename(j, gopRow);
        header = gop[i].getHeader(j, gopRow);
        filesize = gop[i].getFilesize(j, gopRow);
        sendFile(sockfd, servaddr, filename, header, filesize, &throughput);  // read in the file and send it to the server
      }
      // Don't send rest of the tiles. Move on to next gop
    }
  }
}

int main() {
  GOP gop[10];
  char clientAddress[] = "127.0.0.1";
  char serverAddress[] = "127.0.0.1";
  struct sockaddr_in servaddr;
  int sockfd = 0, port = 8081;
  memset(&servaddr, 0, sizeof(servaddr));
  setupClient(clientAddress, serverAddress, &sockfd, port, &servaddr);
  // store instruction data in classes
  getInstr("./gop/gop_data", gop);

  // send file thread
  sendGops(sockfd, servaddr, gop);
  return 0;
}
