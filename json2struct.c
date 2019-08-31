#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

struct thread_args {
	int tile_num;
};

struct GOP {
  int values[4][64];
};

int main(int argc, char const *argv[]) {
  int i = 0, j = 0, k = 0, val = 0;
  char x;
  char s[1024];
  int gop_count = 1, gop_opt_count = 1, tile_count = 64;
  FILE *fp = NULL;
  struct GOP gop[10];

  fp = fopen("inpt", "r");

  while (!feof(fp)) {
    fscanf(fp, "%s", s);
    printf("%s\n", s);
  }

/*
  //while (!feof(fp)) {
    for (i = 0; i < gop_count; i++) {
      for (j = 0; j < gop_opt_count; j++) {
        fscanf(fp, "%d", &val);
        printf("%d\n", val);
        for (k = 0; k < tile_count; k++) {
          fscanf(fp, "%d", &val);
          printf("%d, ", val);
          //gop[i].values[gop_opt_count][tile_count] = val;
        }
      }
    }
//  }
*/
  return 0;
}
