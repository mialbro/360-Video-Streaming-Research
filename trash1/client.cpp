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
    gop[i].setFilenames();
  }
}

// get the size of the current file
int getFileSize(string filename) {
  ifstream inFile(filename, ifstream::ate | ifstream::binary); // open file to read
  return inFile.tellg();  // get the file size
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
void sendFile(UDP udp, string filename, string header, int fileSize) {
  //cout << header << endl;
  char buffer[64000], resp[2];
  int bytesRead = 0, packetSize = 0;
  ifstream inFile(filename, ios::in | ios::binary); // open file to read
  // read the file
  while (bytesRead < fileSize) {
    memset(buffer, 0, 64000); // clear buffer
    packetSize = 64000;
    if (fileSize - bytesRead < 64000) // if near the end of the file
      packetSize = fileSize - bytesRead;
    // add header to buffer
    if (bytesRead == 0) {
      inFile.read(buffer, packetSize - header.length()); // make space for header
      appendHeader(buffer, header, packetSize - header.length()); // add header
    }
    else {
      inFile.read(buffer, packetSize); // make space for header
    }
    udp.sendData(buffer, packetSize); // send the data to the server
    cout << "1" << endl;
    udp.receiveData(resp, 1); // wait for acknowledgement that server got data before moving on
    cout << "2" << endl;
    bytesRead += packetSize;
  }
  inFile.close(); // close the file
  return;
}

string getHeader(string header, int gop, int row, int column, int fileSize) {
  ostringstream oss;
  oss << setw(2) << setfill('0') << gop << "-"  << row << "-" << column << "-" << setw(9) << setfill('0') << fileSize;
  header = oss.str();
  return header;
}

// get the filename
string getFilename(string filename, int gop, int row, int column, int value) {
  // set the file name
  ostringstream oss;
  oss << "./video_files/gop" << gop << "/AngelSplit" << row << "-" << column << "/qp" << value << "/str.bin";
  filename = oss.str();
  return filename;
}


void sendGops(UDP& udp, GOP gop[]) {
  double tp = 0.0;
  int gopRow = 0, tileValue = 0, tileColumn = 0, tileRow = 0, fileSize = 0;
  string filename, header;
  for (int i = 0; i < GOP_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      tp = udp.getTp(); // get the throughput value
      gopRow = gop[i].selGopRow(tp);  // select which tile row to send (corresponds to throughput value)
      tileValue = gop[i].getValue(j, gopRow); // get the tile's value
      if (tileValue != 100) {
        tileRow = gop[i].getRow(j, gopRow); // get the row value for the tile
        tileColumn = gop[i].getColumn(j, gopRow); // get the column value
        filename = getFilename(filename, i, tileRow, tileColumn, tileValue);  // get filename
        fileSize = getFileSize(filename); // get the size of the file
        header = getHeader(header, i, tileRow, tileColumn, fileSize);
        fileSize += header.length(); // add header length to the file size
        sendFile(udp, filename, header, fileSize);  // read in the file and send it to the server
      }
      // Don't send rest of the tiles. Move on to next gop
    }
  }
  udp.kill();
}

void tp(UDP& ack, UDP &client) {
  char buffer[64000];
  memset(buffer, 'x', 64000);
  clock_t t;
  double tp = 0.0, elapsed = 0.0;
  while (ack.checkPulse() == true) {
    t = clock();
    ack.sendData(buffer, sizeof(buffer));
    ack.receiveData(buffer, sizeof(buffer));
    t = clock() - t;
    elapsed = ((float)t)/CLOCKS_PER_SEC;
    if (elapsed > 0) {
      tp = (sizeof(buffer) / elapsed) * pow(8.0, -6.0);
      client.setTp(tp);
    }
    this_thread::sleep_for(chrono::milliseconds(5));
  }
}

int main() {
  GOP gop[10];
  char myAddr[] = "localhost";
  char destAddr[] = "localhost";

  // store instruction data in classes
  getInstr("./gop/gop_data", gop);
  for (int i = 0; i < 10; i++) {
    gop[i].displayRowSize();
  }
  /*
  UDP client = UDP(addr, dest, 8083, 8083);
  UDP ack = UDP(addr, dest, 8084, 8084);

  // send file thread
  thread gopThread(sendGops, ref(client), gop);
  thread tpThread(tp, ref(ack), ref(client));
  gopThread.join();
  tpThread.join();
  */
  return 0;
}
