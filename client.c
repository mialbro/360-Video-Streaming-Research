#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define PORT 8080
#define ROWS 8
#define COLUMNS 8
#define SPF 1.07
#define TILE_COUNT 64
#define GOP_COUNT 1
#define BW_COUNT 4
#define BUFFER_SIZE 64000

struct GOP {
  int *bw_vals;
  int **tile_vals;
};

struct thread_args {
	int tile_num;
	int *bw;
	struct GOP *gop;
};

struct GOP* setGOPStruct() {
  int i = 0, j = 0, k = 0;
  FILE *fp = NULL;
  // start reading gop file
  fp = fopen("./gop/gop_data", "r");
  struct GOP *gop = malloc(sizeof(struct GOP) * (GOP_COUNT+1));
  // loop through the gops -> new struct for each one
  for (i = 0; i < GOP_COUNT; i++) {
    // allocate space for gop values
    gop[i].bw_vals = malloc(BW_COUNT * sizeof(int));
    // allocate space for the array of pointers that will each point to a
    // different set (w/ 64 values) of tile values
    gop[i].tile_vals = malloc(BW_COUNT * sizeof(int));
    // loop through the different bandwidth sets in gop i
    for (j = 0; j < BW_COUNT; j++) {
      // allocate space for this row of data which represents the tile values (64)
      gop[i].tile_vals[j] = malloc(TILE_COUNT * (sizeof(int)));
      // get the bandwidth for the corresponding tile set
      fscanf(fp, "%d", &gop[i].bw_vals[j]);
      // loop through the tiles and store the values
      for (k = 0; k < TILE_COUNT; k++) {
        fscanf(fp, "%d", &gop[i].tile_vals[j][k]);
      }
    }
  }
	/*
	for (i = 0; i < GOP_COUNT; i++) {
		for (j = 0; j < BW_COUNT; j++) {
			printf("%d  ", gop[i].bw_vals[j]);
			for (k = 0; k < TILE_COUNT; k++) {
				printf("%d ",gop[i].tile_vals[j][k]);
			}
			printf("\n");
		}
	}
	*/
  return gop;
}

/*
	currently hardcoded since i need to wait until the instruction file is written
*/
int getStatus(char *status, int gop_num, struct thread_args *args) {
	int tile_num = args->tile_num;
	int bw = 0;

	struct GOP *gop = args->gop;
	//printf("\n%d\n", args->gop[gop].tile_vals[0][args->tile_num]);
	int tile_val = gop[gop_num].tile_vals[0][tile_num];
	//printf("%d ", tile_val);
	sprintf(status, "%d", tile_val);
}

int setRowCol(char *row, char *column, int tile_num) {
	// row
	sprintf(row, "%d", (int)((tile_num) / COLUMNS) + 1);
	// column
	sprintf(column, "%d", (tile_num % COLUMNS) + 1);
}


// filename = getFilename(row, column, gop_num, status);
char *getFilename(char *filename, char *row, char *column, char *gop_num, char *status) {
	memset(filename, 0, sizeof(filename));
	strcat(filename, "./video_files/gop");
	strcat(filename, gop_num);
	strcat(filename, "/");
	strcat(filename, "AngelSplit");
	strcat(filename, row);
	strcat(filename, "-");
	strcat(filename, column);
	strcat(filename, "/qp");
	strcat(filename, status);
	strcat(filename, "/str.bin");
}

int sendGOP(struct sockaddr_in servaddr, int client_sock, int tile_num, char *row, char *column, char *gop_num, char *status) {
	int packet_size = 0, bytes = 0, file_size = 0, len = 0;
	time_t start_time;
	char filename[1024], buffer[BUFFER_SIZE], ack_buffer[1024];
	FILE *fp = 0;

	struct sockaddr_in cliaddr;

	getFilename(filename, row, column, gop_num, status);

	memset(buffer, 0, sizeof(buffer));
	/* user is not looking at this tile so
		 do not send it. just send empty packet
	*/
	if (strcmp(status, "100") == 0) {
		sendto(client_sock, buffer, 0, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		return 0;
	}
	/* open file to send */
	fp = fopen(filename, "rb");
	if (fp == NULL) {
		return 0;
	}
	// get the file size
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* read in the file and send it to the server */
	start_time = time(NULL);
	while(fread(buffer, 1, sizeof(buffer), fp) > 0 ) {
		// calculate the size of the packet to be sent
		if (file_size - bytes > sizeof(buffer))
			packet_size = sizeof(buffer);
		else
			packet_size = file_size - bytes;
		// send the packet and store the number of bytes that have been sent
		bytes += sendto(client_sock, buffer, packet_size, 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
		// clear the buffer
		memset(buffer, 0, sizeof(buffer));
		// make sure we don't take too long sending packet
		// if we can't send the file quick enough (in 1.07 seconds)
		// quit early!
		if ((double)(time(NULL) - start_time) >= SPF) {
			break;
		}
	}
	fclose(fp);
	printf("\nClosed file\n");
}

/* read the file to send and send it to server  */
void *sendThread(void *arguments) {
	int file_size = 0, client_sock = 0;
	time_t start_time = 0;
	char row[5], column[5], status[5], gop_num[5];
	struct sockaddr_in servaddr;
	struct thread_args *args = arguments;
	/* create client socket */
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("\n Socket creation error\n");
			return 0;
	}
	/* configure socket */
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT + args->tile_num);
	servaddr.sin_addr.s_addr = inet_addr("192.168.0.1");
	// get the corresponding tile's row and column
	setRowCol(row, column, args->tile_num);
	/*
		use this port to listen for additional frames.
		each time we will receive a tile from this row / column.
		this represents a tile from every frame of the video.
	*/
	for (int i = 0; i < GOP_COUNT; i++) {
		// read file to get the status (quality) of the tile to be selected
		getStatus(status, i, arguments);
		start_time = time(NULL);
		// don't send tile if the status is set to 100 -> user not looking in that location
		sprintf(gop_num, "%d", i);
		sendGOP(servaddr, client_sock, args->tile_num, row, column, gop_num, status);
		// sleep until next frame needs to be sent
		// don't send additional tiles untill the current frame ends -> 1.07 seconds
		//printf("\nsleep: %lf\n", SPF-(double)(time(NULL)-start_time));
		if (SPF-(double)(time(NULL)-start_time) <= 0)
			return 0;
		else
			sleep(SPF - (double)(time(NULL) - start_time));
	}
	return 0;
}

int main(int argc, char const *argv[]) {
		int i = 0;
		pthread_t thread_array[TILE_COUNT];
		struct thread_args args[TILE_COUNT];
		struct GOP *gop = NULL;

		gop = setGOPStruct();

		for (i = 0; i < TILE_COUNT; i++) {
			args[i].tile_num = i;
			args[i].gop = gop;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &sendThread, (void *)&args[i]);
		}
		/* wait for threads to end */
		for ( i = 0; i < TILE_COUNT; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
