#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <sys/time.h>

#define START_PORT 8080
#define ROWS 8
#define COLUMNS 8
#define SPF 1.07
#define TILE_COUNT 64
#define GOP_COUNT 10
#define BW_COUNT 4
#define BUFFER_SIZE 64000

char header[] = {
  0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00,
  0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5a, 0x95, 0x98, 0x09, 0x00, 0x00, 0x00, 0x01,
  0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
  0x00, 0x5a, 0xa0, 0x07, 0x82, 0x01, 0xe1, 0x65, 0x95, 0x9a, 0x49, 0x32, 0xb8, 0x04, 0x00, 0x00
};

struct GOP {
  int *bw_vals;
  int **tile_vals;
};

struct thread_args {
  int tile_num;
  double *bandwidth;
  struct GOP *gop;
};

int selectTransferRate(double bandwidth, int bw_vals[]) {
  int i = 0, rateIndex = 0;
  double smallest_diff = 0.0, diff = 0.0;
  // make the first bw option be the value
  smallest_diff = bandwidth - bw_vals[0];
  // loop through the available bandwidths and select the one closes to our calculated value
  for (i = 0; i < BW_COUNT; i++) {
    diff = bandwidth - bw_vals[i];
    if ( (diff < smallest_diff) && (diff >= 0) ) {
      smallest_diff = diff;
      rateIndex = i;
    }
  }
  return rateIndex;
}

void *calculateBandwidth(void *arg) {
  char buffer[BUFFER_SIZE];
  double elapsed = 0.0, data = 0.0;
  double *bandwidth = NULL;
  struct timeval t0, t1;
  struct timeval timeout;
  struct sockaddr_in servaddr, cliaddr;
  int client_sock = 0, len = 0, bytes = 0, i = 0;

  // set timeout to 1.07 seconds
  timeout.tv_sec = 1;
  timeout.tv_usec = 70000;

  bandwidth = arg;

  /* create client socket */
  client_sock = socket(AF_INET, SOCK_DGRAM, 0);
 /* configure send socket -> provide the address of other device */
 servaddr.sin_family = AF_INET;
 servaddr.sin_port = htons(100 + START_PORT);
 servaddr.sin_addr.s_addr = inet_addr("192.168.0.2");

  if (connect(client_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
    printf("ERROR!\n");
  }
  setsockopt(client_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
  memset(buffer, '1', sizeof(buffer));

  // send packet to server then listen for response to calculate bandwidth
  while (1) {
    // get the start time
    gettimeofday(&t0, 0);
    // send syn to the server
    sendto(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)NULL, sizeof(servaddr));
    // wait for ack from the server
    bytes = recvfrom(client_sock, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)NULL, NULL);
    // check to make sure we got all our data back
    if (bytes == BUFFER_SIZE) {
      // get the end time
      gettimeofday(&t1, 0);
      // calculate the total time it took to send the data and get a response
      elapsed = ((t1.tv_sec-t0.tv_sec)*1000000.0 + t1.tv_usec-t0.tv_usec) / 1000000.0;
      elapsed = elapsed / 2.0;
      // calculate the amount of data we sent (in Megabits)
      data = (BUFFER_SIZE + 28.0) / 125000.0;
      // calculate the bandwidth (Megabits per second)
      *bandwidth = data / elapsed;
      // display the calculated bandwidth
    }
  }
}

/* read in gop instruction file and return a struct representation */
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
  return gop;
}

/* get the tile value to send for the current gop */
int getStatus(char *status, int gop_num, struct thread_args *args) {
  struct GOP *gop = NULL;
  int tile_num = 0, tile_val = 0, rateIndex = 0;
  double bandwidth = 0.0;

  gop = args->gop;
  // store the calculated bandwidth value (currently being calculated in other thread)
  bandwidth = *args->bandwidth;
  tile_num = args->tile_num;
  // set correct bandwidth if value has been calculated, set it to the lowest possible value otherwise
  if (bandwidth > 0)
    rateIndex = selectTransferRate(bandwidth, gop->bw_vals);
  else
    rateIndex = 0;
  // get the tile value given the calculated bandwidth
  tile_val = gop[gop_num].tile_vals[rateIndex][tile_num];
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
	strcat(filename, "/media/pi/12BF-9A5D/video_files/gop");
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

/* set the send timeout */
void setTimeout(int client_sock, double elapsed_time) {
	struct timeval timeout;
	if (elapsed_time <= SPF - 1) {
		timeout.tv_sec = 1;
    timeout.tv_usec = (int)(((SPF - 1) - elapsed_time) * 1000000);
	}
	else if (elapsed_time > SPF - 1) {
		timeout.tv_sec = 0;
    timeout.tv_usec  = (int)((SPF - elapsed_time) * 1000000);
	}
	setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));
}

int getFileSize(FILE *fp) {
  int file_size = 0;
  fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);
  return file_size;
}

void sendHeader(int client_sock, int start_time, struct sockaddr_in servaddr) {
  sendto(client_sock, header, sizeof(header), 0, (struct sockaddr*)&servaddr, sizeof(servaddr));
  double time_left = SPF - (time(NULL) - start_time);
  if (time_left > 0)
    sleep(time_left);
}

/*
sends the same tile for every frame
*/
int sendGOP(struct sockaddr_in servaddr, int client_sock, int tile_num, char *row, char *column, char *gop_num, char *status) {
	int packet_size = 0, bytes = 0, file_size = 0, len = 0;
  double time_left = 0.0, start_time = 0.0, elapsed_time = 0.0;
	char filename[1024], buffer[BUFFER_SIZE], ack_buffer[1024];
	FILE *fp = 0;

  struct timeval timeout;
  start_time = time(NULL);

  timeout.tv_sec = 1;
  timeout.tv_usec = 70000;
  // change to send timeout
  setsockopt(client_sock, SOL_SOCKET, SO_SNDTIMEO, (char*)&timeout, sizeof(timeout));

	struct sockaddr_in cliaddr;
	getFilename(filename, row, column, gop_num, status);
	memset(buffer, 0, sizeof(buffer));
	/* user is not looking at this tile so
		 do not send it. just send header
	*/
	if (strcmp(status, "100") == 0) {
    sendHeader(client_sock, start_time, servaddr);
    return 0;
	}

	/* open file to send */
	fp = fopen(filename, "rb");
	if (fp == NULL) {
    printf("could not find %s\n", filename);
		exit(0);
	}
	// get the file size
  file_size = getFileSize(fp);

	/* read in the file and send it to the server until the file is done or we run out of time */
	while ((fread(buffer, 1, sizeof(buffer), fp)) > 0 && (elapsed_time < SPF)) {
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
    time_left = SPF - (time(NULL) - start_time);
		if (time_left <= 0) {
			break;
		}
    elapsed_time = time(NULL) - start_time;
    setTimeout(client_sock, elapsed_time);
	}
	fclose(fp);
  // sleep if we have time left
  time_left = SPF - (time(NULL) - start_time);
  printf("time-left: %f\n", time_left);
  if (time_left > 0)
    sleep(time_left);
}

/* read the file to send and send it to server  */
void *sendThread(void *arguments) {
	int file_size = 0, client_sock = 0;
	time_t start_time = 0;
  double time_left = 0.0;
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
	servaddr.sin_port = htons(START_PORT + args->tile_num);
	servaddr.sin_addr.s_addr = inet_addr("192.168.0.2");
	// get the corresponding tile's row and column
	setRowCol(row, column, args->tile_num);
	/*
		use this port to listen for additional frames.
		each time we will receive a tile from this row / column.
		this represents the same tile from every frame of the video.
	*/
	for (int i = 0; i < GOP_COUNT; i++) {
		// read file to get the status (quality) of the tile to be selected
		getStatus(status, i, arguments);
		// don't send tile if the status is set to 100 -> user not looking in that location
		sprintf(gop_num, "%d", i);
		sendGOP(servaddr, client_sock, args->tile_num, row, column, gop_num, status);
	}
	return 0;
}

int main(int argc, char const *argv[]) {
		int i = 0;
    double *bandwidth = NULL;
		pthread_t thread_array[TILE_COUNT], bandwidth_thread;
		struct thread_args args[TILE_COUNT];
		struct GOP *gop = NULL;

		gop = setGOPStruct();
    bandwidth = (double*)malloc(1*sizeof(int));
    /* create thread to calculate bandwidth */
    pthread_create(&bandwidth_thread, NULL, &calculateBandwidth, (void *)bandwidth);
		for (i = 0; i < 1; i++) {
			args[i].tile_num = i;
			args[i].gop = gop;
      // store adress of bandwidth value
      args[i].bandwidth = bandwidth;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &sendThread, (void *)&args[i]);
		}
		/* wait for threads to end */
		for ( i = 0; i < 1; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
