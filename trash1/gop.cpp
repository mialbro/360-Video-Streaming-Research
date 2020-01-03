#include <fstream>
#include <iostream>
#include <string>
#include <cstdlib>
#include <fstream>
#include <algorithm>
#include <bits/stdc++.h>

#define GOP_COUNT 10
#define TILE_COUNT 64
#define BW_COUNT 4

#include "gop.h"

using namespace std;

GOP::GOP() {
  gop = 0;
}

// initialize the tile
void GOP::setTile(int row, int column, int position, int value) {
  tiles[0][row][column] = position;  // store the tile position
  tiles[1][row][column] = value;  // store the tile value
}

void GOP::setBw(int i, int value) {
  bwVals[i] = value;
}

// show the tile values
void GOP::displayTiles() {
  for (int i = 0; i < BW_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      cout << tiles[1][i][j] << " ";
    }
    cout << "\n\n\n";
  }
}

// sort the tiles by the tile values, in increasing order
void GOP::sortTiles() {
  for (int i = 0; i < BW_COUNT; i++) {
    pair<int, int> pairt[TILE_COUNT]; // create pairs for tile position and value
    for (int j = 0; j < TILE_COUNT; j++) {
      pairt[j].first = tiles[1][i][j];
      pairt[j].second = tiles[0][i][j];
    }
    // sort the pair array
    sort(pairt, pairt + TILE_COUNT);
    // modify original arrays
    for (int j = 0; j < TILE_COUNT; j++) {
      tiles[1][i][j] = pairt[j].first;
      tiles[0][i][j] = pairt[j].second;
    }
  }
}

void GOP::setFilenames() {
  int position = 0, gopValue = 0, gopRow = 0, gopColumn = 0, size = 0;

  for (int i = 0; i < BW_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      ostringstream nameOSS;
      ostringstream headerOSS;
      position = tiles[0][i][j];
      gopValue = tiles[1][i][j];
      if (gopValue != 100) {
        gopRow = row[i][j];
        gopColumn = column[i][j];
        nameOSS << "./video_files/gop" << gop << "/AngelSplit" << gopRow << "-" << gopColumn << "/qp" << gopValue << "/str.bin";
        filenames[i][j] = nameOSS.str();

        ifstream inFile(filenames[i][j], ifstream::ate | ifstream::binary); // open file to read
        size =  inFile.tellg();  // get the file size
        headerOSS << setw(2) << setfill('0') << gop << "-"  << gopRow << "-" << gopColumn << "-" << setw(9) << setfill('0') << size;
        headers[i][j] = headerOSS.str();
        filesizes[i][j] = size;
        cout << filenames[i][j] << " <-> " << headers[i][j] << endl;
      }
      else {
        filenames[i][j] = "";
        headers[i][j] = "";
        filesizes[i][j] = 0;
      }
    }
  }
}


// display the size of the files, that we will display, in the row
void GOP::displayRowSize() {
  int x = 0, value = 0;
  for (int i = 0; i < BW_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      value = tiles[1][i][j];
      if (value != 100)
        x += filesizes[i][j];
    }
    cout << i << " -> " << x << endl;
    x = 0;
  }
}

// set the row and column values
void GOP::setRowCol() {
  int position = 0;
  for (int i = 0; i < BW_COUNT; i++) {
    for (int j = 0; j < TILE_COUNT; j++) {
      position = tiles[0][i][j];
      row[i][j] = (int)((position) / 8) + 1;
      column[i][j] = (position % 8) + 1;
    }
  }
  return;
}
// set the group of picture number
void GOP::setGop(int i) {
  gop = i;
  return;
}

int GOP::getRow(int index, int gopRow) {
  return row[gopRow][index];
}

int GOP::getColumn(int index, int gopRow) {
  return column[gopRow][index];
}

int GOP::getValue(int index, int gopRow) {
  return tiles[1][gopRow][index];
}

int GOP::selGopRow(double throughput) {
  int i = 0, rateIndex = 0;
  double smallestDiff = 0.0, diff = 0.0;
  // make the first bw option be the value
  smallestDiff = throughput - bwVals[0];
  // loop through the available bandwidths
  // and select the one closes to our calculated value
  for (int i = 0; i < BW_COUNT; i++) {
    diff = throughput - bwVals[i];
    if ( (diff < smallestDiff) && (diff >= 0) ) {
      smallestDiff = diff;
      rateIndex = i;
    }
  }
  return rateIndex;
}
