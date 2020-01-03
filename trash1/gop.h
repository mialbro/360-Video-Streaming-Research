#ifndef GOP_H_
#define GOP_H_

#include <string>
using namespace std;

class GOP {
private:
  int bwVals[4], tiles[2][4][64], currBw, row[4][64], column[4][64], gop;
  int filesizes[4][64];
  string filenames[4][64], headers[4][64];

public:
  GOP();
  void setBw(int i, int value); // set the row throughput header
  void setTile(int row, int column, int position, int value); // initialize the tile
  void displayTiles();  // display all of the tiles in the gop
  void sortTiles(); // sort the tiles by the tile value (increasing order)
  void setRowCol(); // set the row and column values
  void setGop(int i); // set the gop number
  int getRow(int index, int gopRow);  // get the corresponding time row
  int getColumn(int index, int gopRow); // get the corresponding tile column
  int getValue(int index, int gopRow);  // get the corresponding vile value
  int selGopRow(double throughput); // pick row of gops based on the throughput
  void displayRowSize();
  void setFilenames();
};

#endif
