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

using namespace std;

void tp(UDP& server) {
  char buffer[100];
  while (server.checkPulse() == true) {
    server.receiveData(buffer, sizeof(buffer)); // wait for synchronous signal
    server.sendData(buffer, sizeof(buffer));  // send acknowledgement
  }
}

void receiveGops(UDP& server) {
  char buffer[64000], nameBuffer[12], sizeBuffer[12];
  int bytesRec = 0, fileSize = 0, packetSize = 0, hI = 64000 - 16;
  string filename;
  // receive data
  while (1) {
    bytesRec = server.receiveData(buffer, 64000); // get the first packet
    copy(&buffer[hI], &buffer[hI + 6], &nameBuffer[0]); // extract the filename from the header (gop-row-column)
    copy(&buffer[hI + 8], &buffer[hI + 16], &sizeBuffer[0]); // extract the filesize from the header
    filename = nameBuffer;  // convert filename to string
    filename = "/received/" + filename + ".bin";
    sscanf(sizeBuffer, "%d", &fileSize);  // convert filesize to string
    cout << fileSize << endl;
    ofstream file (filename, ios::out | ios::binary); // open file to begin writing
    file.write(buffer, packetSize - 16);  // write the first packet - header, to the file
    // read in rest of the packets
    while (bytesRec < fileSize) {
      packetSize = 64000;
      if (fileSize - bytesRec < 64000)
        packetSize = fileSize - bytesRec;
      bytesRec += server.receiveData(buffer, packetSize);
      file.write(buffer, packetSize); // write packet to the file
      memset(buffer, 0, 64000); // clear the buffer
    }
    memset(nameBuffer, 0, 12); // clear name buffer
    memset(sizeBuffer, 0, 12);  // clear file size buffer
    file.close(); // close current file
  }
}

int main() {
  char addr[] = "192.168.0.1";
  char dest[] = "192.168.0.2";

  UDP server = UDP(addr, dest, 0, 0);
  thread gopThread(receiveGops, ref(server));
  thread tpThread(tp, ref(server));
  gopThread.join();
  tpThread.join();

  return 0;
}
