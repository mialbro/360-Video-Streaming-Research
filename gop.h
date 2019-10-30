#ifndef GOP_H_
#define GOP_H_

class GOP {
private:
  int bwVals[4], tiles[2][4][64], currBw, row[4][64], column[4][64], gop;
public:
  GOP();
  void setBw(int i, int value);
  void setTile(int row, int column, int position, int value);
  // show the tile values
  void displayTiles();
  // sort the tiles
  void sortTiles();
  // set the row and column values
  void setRowCol();
  // set the group of picture number
  void setGop(int i);
  void createFilenames();
};

#endif
