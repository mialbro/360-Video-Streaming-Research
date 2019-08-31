#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>

#define PORT 8080
#define ROWS 8
#define COLUMNS 8

struct thread_args {
	int tile_num;
};

int getRow(int tile_num) {
	return floor(tile_num / COLUMNS);
}

int getColumn(int tile_num) {
	return tile_num % COLUMNS;
}

char *int2String(int value) {
	char res[5];
	sprintf(res, "%d", value);
	return res;
}


char *getFilename(int row, int column, int gop_num, int status) {
	char filename[1024];

	filename = "./files2/gop";
	strcat(filename, int2char(gop_num));
	strcat(filename, "/");
	strcat(filename, "AngelSplit");
	strcat(filename, int2char(row));
	strcat(filename, "-");
	strcat(filename, int2char(column));
	strcat(filename, "/qp");
	strcat(filename, int2char(status));
	strcat(filename, "/str.bin");

	return filename;
}




int sendGOP(struct sockaddr_in servaddr, int client_sock, char *buffer, int tile_num, int row, int column, int status) {
	int packet_size = 0, bytes = 0, file_size = 0;
	char *filename;
	FILE *fp = 0;

	filename = getFilename(row, column, gop_num, status);

	/* open file to send */
	fp = fopen(filename, "r");
	if (fp == NULL)
		return 0;

	// get the file size
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	/* read in the file and send it to the server */
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
	}
	fclose(fp);
}

/* read the file to send and send it to server  */
void *sendThread(void *arguments) {
	int buffer_size = 64000, file_size = 0, client_sock = 0, rwo = 0, column = 0, status = 0;
	char buffer[buffer_size];
	struct sockaddr_in servaddr;

	struct thread_args *args = arguments;

	/* create client socket */
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("\n Socket creation error \n");
			return 0;
	}

	/* configure socket */
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT + args->tile_num);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	// int sendGOP(struct sockaddr_in servaddr, int client_sock, char *buffer, int file_size, FILE *fp)
	row = getRow(args->tile_num);
	column = getColumn(args->tile_num);

	for (int i = 0; i < gop_count; i++) {
		// read file to get the status (quality) of the tile to be selected
		status = getStatus(i, args->tile_num);
		sendGOP(servaddr, client_sock, buffer, args->tile_num, row, column, status);
	}
	return 0;
}

int main(int argc, char const *argv[]) {
		int tile_count = 60, i = 0;
		pthread_t thread_array[tile_count];
		struct thread_args args[tile_count];

		///////////////   LOOP   ///////////////////////////////////
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
