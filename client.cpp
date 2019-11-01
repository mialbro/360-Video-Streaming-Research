#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>
#include <sstream>
#include <iomanip>

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
  char buffer[64000];
  int bytesRead = 0, packetSize = 0;
  ifstream inFile(filename, ios::in | ios::binary); // open file to read
  //ofstream myFile ("data.bin", ios::out | ios::binary);
  // read the file
  while (bytesRead < fileSize) {
    fill_n(buffer, 64000, 0); // clear buffer
    packetSize = 64000;
    if (fileSize - bytesRead < 64000) // if ear the end of the fil-e
      packetSize = fileSize - bytesRead;
    // add header to buffer
    if (bytesRead == 0) {
      inFile.read(buffer, packetSize - header.length()); // make space for header
      appendHeader(buffer, header, packetSize - (header.length())); // add header
    }
    else {
      inFile.read(buffer, packetSize); // make space for header
    }
    // send packet to server
    //myFile.write(buffer, packetSize);
    bytesRead += packetSize;
  }
  return;
}

// get the filename
string getFilename(string filename, int gop, int row, int column, int value) {
  // set the file name
  ostringstream oss;
  oss << "./video_files/gop" << gop << "/AngelSplit" << row << "-" << column << "/qp" << value << "/str.bin";
  filename = oss.str();
  return filename;
}

string getHeader(string header, int gop, int row, int column, int fileSize) {
  ostringstream oss;
  oss << setw(2) << setfill('0') << gop << "-"  << row << "-" << column << "-" << setw(9) << setfill('0') << fileSize;
  header = oss.str();
  return header;
}

void sendGops(UDP udp, GOP gop[]) {
  double tp = 0.0;
  int gopRow = 0, tileValue = 0, tileColumn = 0, tileRow = 0, fileSize = 0;
  string filename, header;
  for (int i = 0; i < 1; i++) {
    for (int j = 0; j < 1; j++) {
      tp = udp.getTp(); // get the throughput value
      gopRow = gop[i].selGopRow(tp);  // select which tile row to send (corresponds to throughput value)
      tileValue = gop[i].getValue(j, gopRow); // get the tile's value
      if (tileValue != 100) {
        tileRow = gop[i].getRow(j, gopRow); // get the row value for the tile
        tileColumn = gop[i].getColumn(j, gopRow); // get the column value
        filename = getFilename(filename, i, tileRow, tileColumn, tileValue);  // get filename
        fileSize = getFileSize(filename); // get the size of the file
        header = getHeader(header, i, tileRow, tileColumn, fileSize);
        cout << header << endl;
        fileSize += header.length(); // add header length to the file size
        sendFile(udp, filename, header, fileSize);  // read in the file and send it to the server
      }
      // Don't send rest of the tiles. Move on to next gop
      else {
        break;
      }
    }
  }
}

int main() {
  GOP gop[10];

  // store instruction data in classes
  getInstr("./gop/gop_data", gop);
  UDP client = UDP("192.168.0.2", "192.168.0.1", 0, 0);
  sendGops(client, gop);
  //sendFile("./video_files/gop0/AngelSplit1-1/qp1/str.bin",header);
  // D:\reps\360-video\video_files\gop0\AngelSplit1-1\qp1
  //UDP client = UDP("192.168.0.2", "192.168.0.1", 0, 0);
  return 0;
}
