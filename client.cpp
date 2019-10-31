#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>

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
void appendHeader(char *buffer, char *header, int packetSize) {
  for (int i = 0; i < strlen(header); i++) {
    buffer[packetSize + i] = header[i];
  }
}

// read in binary file
void sendFile(string filename, string header, int fileSize) {
  char buffer[64000];
  int bytesRead = 0, packetSize = 0;
  fill_n(buffer, 64000, 0); // clear buffer
  ifstream inFile(filename, ios::in | ios::binary); // open file to read
  // read the file
  while (bytesRead < fileSize) {
    packetSize = 64000;
    if (fileSize - bytesRead < 64000) // if ear the end of the fil-e
      packetSize = fileSize - bytesRead;
    // add header to buffer
    if (bytesRead == 0) {
      inFile.read(buffer, packetSize - strlen(header)); // make space for header
      appendHeader(buffer, header, packetSize - strlen(header)); // add header
    }
    // send packet to server
    bytesRead += packetSize;
  }
  return;
}

// get the filename
string getFilename(string filename, int row, int column, int value, int gop) {
  // get the througput info
  gopRow = gop.getGopRow(tp);
  // set the file name
  filename = "./video_files/gop" << gop << "/AngelSplit" << row;
  filename += "-" << column << "/qp" << value < "/str.bin";
  return filename;
}

string getHeader(string header, int gopCount, int row, int column, int size) {
  header = setfill('0') << setw(2) << gopCount;
  header += "-" << row;
  header += "-" << column;
  header += "-" << setfill('0') << setw(9) << filesize;
  return header;
}

void sendGops(UDP udp, GOP gop[]) {
  double tp = 0.0;
  int gopRow = 0, tileValue = 0, tileColumn = 0, fileSize = 0;
  string filename, header;
  for (int i = 0; i < GOP_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      tp = udp.getTp();
      gopRow = gop[i].getGopRow(tp);
      tileValue = gop[i].getValue(j, gopRow);
      if (tileValue != 100) {
        tileRow = gop[i].getRow(j, gopRow);
        tileColumn = gop[i].getColumn(j, gopRow);
        filename = getFilename(filename, tileRow, tileColumn, tileValue, i);
        fileSize = getFileSize(filename);
        header = getHeader(header, i, tileRow, tileColumn, fileSize);
        fileSize += strlen(header);
        sendFile(udp, filename, header, fileSize);
      }
      else {
        break;
      }
    }
  }
}

int main() {
  GOP gop[10];
  char string;

  // store instruction data in classes
  getInstr("./gop/gop_data", gop);

  //sendFile("./video_files/gop0/AngelSplit1-1/qp1/str.bin",header);
  // D:\reps\360-video\video_files\gop0\AngelSplit1-1\qp1
  //UDP client = UDP("192.168.0.2", "192.168.0.1", 0, 0);
  return 0;
}
