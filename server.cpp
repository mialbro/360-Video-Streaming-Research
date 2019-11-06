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

void tp(UDP& ack, UDP& server) {
  char buffer[100];
  while (server.checkPulse() == true) {
    ack.receiveData(buffer, sizeof(buffer)); // wait for synchronous signal
    ack.sendData(buffer, sizeof(buffer));  // send acknowledgement
  }
}

void receiveGops(UDP& server) {
  char buffer[64000], nameBuffer[12], sizeBuffer[12], headerBuffer[16];
  char resp[] = "1";
  int bytesRec = 0, fileSize = 0, packetSize = 0, hI = 64000 - 16;
  string filename, header;
  // receive data
  while (1) {
    bytesRec = server.receiveData(buffer, 64000); // get the first packet
    copy(&buffer[hI], &buffer[hI+16], &headerBuffer[0]);
    copy(&buffer[hI], &buffer[hI + 6], &nameBuffer[0]); // extract the filename from the header (gop-row-column)
    copy(&buffer[hI + 8], &buffer[hI + 16], &sizeBuffer[0]); // extract the filesize from the header
    filename = nameBuffer;  // convert filename to string
    header = headerBuffer;
    filename = "./received/" + filename + ".bin";
    sscanf(sizeBuffer, "%d", &fileSize);  // convert filesize to string
    cout << "_" << header << "_" << endl;
    ofstream file (filename, ios::out | ios::binary); // open file to begin writing
    file.write(buffer, packetSize - 16);  // write the first packet - header, to the file
    // read in rest of the packets
    while (bytesRec < fileSize) {
      packetSize = 64000;
      if (fileSize - bytesRec < 64000)
        packetSize = fileSize - bytesRec;
      bytesRec += server.receiveData(buffer, packetSize);
      server.sendData(resp, 1);
      file.write(buffer, packetSize); // write packet to the file
      memset(buffer, 0, 64000); // clear the buffer
    }
    memset(nameBuffer, 0, 12); // clear name buffer
    memset(sizeBuffer, 0, 12);  // clear file size buffer
    file.close(); // close current file
  }
}

int main() {
  char addr[] = "xxx.xxx.x.x";
  char dest[] = "192.168.0.2";

  UDP server = UDP(addr, dest, 0, 0);
  UDP ack = UDP(addr, dest, 2, 2);
  thread gopThread(receiveGops, ref(server));
  thread tpThread(tp, ref(ack), ref(server));
  gopThread.join();
  tpThread.join();

  return 0;
}
