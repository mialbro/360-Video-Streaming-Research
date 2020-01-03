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

#include "udp.h"
#include "gop.h"

#define GOP_COUNT 10
#define TILE_COUNT 64
#define BW_COUNT 4

using namespace std;

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
  char headerArr[header.length() + 1];
  header.copy(headerArr, header.length(), 0);
  for (int i = 0; i < header.length(); i++) {
    buffer[packetSize + i] = headerArr[i];
  }
}

// read in binary file
void sendFile(UDP udp, string filename, string header, int fileSize, double *throughput) {
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
    }
    else {
      inFile.read(dataBuffer, packetSize); // make space for header
    }
    udp.sendData(dataBuffer, packetSize); // send the data to the server
    udp.receiveData(respBuffer, strlen(resp)); // wait for acknowledgement that server got data before moving on
    bytesRead += packetSize;
    //cout << respBuffer << endl;
  }
  //cout << endl << endl << endl;
  inFile.close(); // close the file
  endTime = time(NULL);
  sendTime = endTime - startTime;
  if (sendTime > 0)
    *throughput = (fileSize * 8 * pow(10.0, -6.0)) / (sendTime / 2);
  else
    *throughput = 0;
  //cout << *throughput << endl;
  return;
}


void sendGops(UDP& udp, GOP gop[]) {
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
        filesize = gop[i].getFilesize(j, gopRow) + header.length();
        sendFile(udp, filename, header, filesize, &throughput);  // read in the file and send it to the server
      }
      // Don't send rest of the tiles. Move on to next gop
    }
  }
}

int main() {
  GOP gop[10];
  char clientaddr[] = "192.168.0.2";
  char serveraddr[] = "192.168.1.228";
  char state[] = "c";

  // store instruction data in classes
  getInstr("./gop/gop_data", gop);

  UDP client = UDP(clientaddr, serveraddr, 8080, state);
  // send file thread
  sendGops(client, gop);
  return 0;
}
