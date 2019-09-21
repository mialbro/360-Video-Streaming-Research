#include <stdio.h>
#include <string.h>
#include <stdlib.h>

struct GOP {
  int *bw_vals;
  int **tile_vals;
};

int main(int argc, char const *argv[]) {
  int bw_count = 0, tile_count = 0, gop_count = 0;
  int i = 0, j = 0, k = 0;
  FILE *fp = NULL;
  // start reading gop file
  fp = fopen("gop_data", "r");
  fscanf(fp, "%d", &bw_count);
  fscanf(fp, "%d", &tile_count);
  fscanf(fp, "%d", &gop_count);
  struct GOP gop[gop_count];
  // loop through the gops -> new struct for each one
  for (i = 0; i < gop_count; i++) {
    // allocate space for gop values
    gop[i].bw_vals = malloc(bw_count * sizeof(int));
    // allocate space for the array of pointers that will each point to a
    // different set (w/ 64 values) of tile values
    gop[i].tile_vals = malloc(bw_count * sizeof(int));
    // loop through the different bandwidth sets in gop i
    for (j = 0; j < bw_count; j++) {
      // allocate space for this row of data which represents the tile values (64)
      gop[i].tile_vals[j] = malloc(tile_count * (sizeof(int)));
      // get the bandwidth for the corresponding tile set
      fscanf(fp, "%d", &gop[i].bw_vals[j]);
      // loop through the tiles and store the values
      for (k = 0; k < tile_count; k++) {
        fscanf(fp, "%d", &gop[i].tile_vals[j][k]);
      }
    }
  }
  for (i = 0; i < gop_count; i++) {
    for (j = 0; j < bw_count; j++) {
      printf("%d  ",gop[i].bw_vals[j]);
      for (k = 0; k < tile_count; k++) {
        printf("%d ", gop[i].tile_vals[j][k]);
      }
      printf("\n");
    }
  }
  return 0;
}
