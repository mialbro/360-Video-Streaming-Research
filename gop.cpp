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
