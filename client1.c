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

int hardcodedqp[] = {1 , 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 1, 1, 1, 100, 100, 100, 100, 100, 1, 1, 1, 1, 100, 100, 100, 1, 1, 1, 1, 1, 100, 100, 100, 1, 1, 1, 1, 1, 100, 100, 100, 100, 1, 1, 1, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

struct thread_args {
	int tile_num;
};

/*
	currently hardcoded since i need to wait until the instruction file is written
*/
int getStatus(char *status, int gop, int tile_num) {
	sprintf(status, "%d", hardcodedqp[tile_num]);
}

int setRowCol(char *row, char *column, int tile_num) {
	// row
	sprintf(row, "%d", tile_num / COLUMNS);
	// column
	sprintf(column, "%d", tile_num % COLUMNS);
}


// filename = getFilename(row, column, gop_num, status);
char *getFilename(char *filename, char *row, char *column, char *gop_num, char *status) {
	char gop_str[5], row_str[5], column_str[5], status_str[5];

	memset(filename, 0, sizeof(filename));
	strcat(filename, "./files2/gop");
	strcat(filename, gop_num);
	strcat(filename, "/");
	strcat(filename, "AngelSplit");
	strcat(filename, row);
	strcat(filename, "-");
	strcat(filename, column);
	strcat(filename, "/qp");
	strcat(filename, status);
	strcat(filename, "/str.bin");
	printf("__%s__\n", filename);
}



// 		sendGOP(servaddr, client_sock, args->tile_num, row, column, status);
int sendGOP(struct sockaddr_in servaddr, int client_sock, int tile_num, char *row, char *column, char *gop_num, char *status) {
	int packet_size = 0, bytes = 0, file_size = 0, buffer_size = 64000;
	time_t start_time;

	char filename[1024];
	char *buffer[buffer_size];
	FILE *fp = 0;

	getFilename(filename, row, column, gop_num, status);

	/* open file to send */
	fp = fopen(filename, "r");
	if (fp == NULL)
		return 0;

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
		// make sure we don't get behind
		// one frame = 1.07 seconds
		if (time(NULL) - start_time >= 1.07)
			break;
	}
	// don't send additional tiles untill the current frame ends -> 1.07 seconds
	if (time(NULL) - start_time < 1.07)
		sleep(time(NULL) - start_time);
	fclose(fp);
}

/* read the file to send and send it to server  */
void *sendThread(void *arguments) {
	int file_size = 0, client_sock = 0, gop_count = 10;
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
	servaddr.sin_addr.s_addr = INADDR_ANY;

	// get the corresponding tile's row and column
	setRowCol(row, column, args->tile_num);

	/*
		use this port to listen for additional frames.
		each time we will receive a tile from this row / column.
		this represents a tile from every frame of the video.
	*/
	for (int i = 0; i < gop_count; i++) {
		// read file to get the status (quality) of the tile to be selected
		getStatus(status, i, args->tile_num);
		// don't send tile if the status is set to 100
		if (strcmp("100", status) != 0) {
			sprintf(gop_num, "%d", i);
			sendGOP(servaddr, client_sock, args->tile_num, row, column, gop_num, status);
		}
		// sleep until next frame needs to be sent
		else
			sleep(1.07);
	}
	return 0;
}


int main(int argc, char const *argv[]) {
		int tile_count = 64, i = 0;
		pthread_t thread_array[tile_count];
		struct thread_args args[tile_count];

		for (i = 0; i < tile_count; i++) {
			args[i].tile_num = i;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &sendThread, (void *)&args[i]);
		}
		/* wait for threads to end */
		for ( i = 0; i < tile_count; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
