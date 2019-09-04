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
#define TILE_COUNT 1
#define GOP_COUNT 1
#define BUFFER_SIZE 64000

int hardcodedqp[] = {1 , 1, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100, 1, 1, 1, 100, 100, 100, 100, 100, 1, 1, 1, 1, 100, 100, 100, 1, 1, 1, 1, 1, 100, 100, 100, 1, 1, 1, 1, 1, 100, 100, 100, 100, 1, 1, 1, 100, 100, 100, 100, 100, 100, 100, 100, 100, 100};

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
	sprintf(row, "%d", (int)((tile_num) / COLUMNS) + 1);
	// column
	sprintf(column, "%d", (tile_num % COLUMNS) + 1);
}


// filename = getFilename(row, column, gop_num, status);
char *getFilename(char *filename, char *row, char *column, char *gop_num, char *status) {
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
}

int sendGOP(struct sockaddr_in servaddr, int client_sock, int tile_num, char *row, char *column, char *gop_num, char *status) {
	int packet_size = 0, bytes = 0, file_size = 0;
	time_t start_time;
	char filename[1024], buffer[BUFFER_SIZE];
	FILE *fp = 0;

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
		printf("\nbytes: %d\n", bytes);
		if ((double)(time(NULL) - start_time) >= SPF) {
			printf("\nLeaving Early\n");
			break;
		}
	}
	fclose(fp);
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
	servaddr.sin_addr.s_addr = INADDR_ANY;
	// get the corresponding tile's row and column
	setRowCol(row, column, args->tile_num);
	/*
		use this port to listen for additional frames.
		each time we will receive a tile from this row / column.
		this represents a tile from every frame of the video.
	*/
	for (int i = 0; i < GOP_COUNT; i++) {
		// read file to get the status (quality) of the tile to be selected
		getStatus(status, i, args->tile_num);
		start_time = time(NULL);
		// don't send tile if the status is set to 100 -> user not looking in that location
		sprintf(gop_num, "%d", i);
		sendGOP(servaddr, client_sock, args->tile_num, row, column, gop_num, status);
		// sleep until next frame needs to be sent
		// don't send additional tiles untill the current frame ends -> 1.07 seconds
		sleep(SPF - (double)(time(NULL) - start_time));
	}
	return 0;
}


int main(int argc, char const *argv[]) {
		int i = 0;
		pthread_t thread_array[TILE_COUNT];
		struct thread_args args[TILE_COUNT];

		for (i = 0; i < TILE_COUNT; i++) {
			args[i].tile_num = i;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &sendThread, (void *)&args[i]);
		}
		/* wait for threads to end */
		for ( i = 0; i < TILE_COUNT; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
