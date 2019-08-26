#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#define PORT 8080

struct thread_args {
	char filename[40];
};

int sendGOP(struct sockaddr_in servaddr, int client_sock, char *buffer, int file_size, FILE *fp) {
	int packet_size = 0, bytes = 0;
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
	int buffer_size = 64000, file_size = 0, client_sock = 0;
	char buffer[buffer_size];
	struct sockaddr_in servaddr;
	FILE *fp = NULL;

	struct thread_args *args = arguments;

	/* create client socket */
	if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
			printf("\n Socket creation error \n");
			return 0;
	}

	/* configure socket */
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = INADDR_ANY;

	/* open file to send */
	fp = fopen(args->filename, "r");
	if (fp == NULL)
		return 0;

	// get the file size
	fseek(fp, 0, SEEK_END);
	file_size = ftell(fp);
	fseek(fp, 0, SEEK_SET);

	// int sendGOP(struct sockaddr_in servaddr, int client_sock, char *buffer, int file_size, FILE *fp)
	sendGOP(servaddr, client_sock, buffer, file_size, fp);

	return 0;
}

int main(int argc, char const *argv[]) {
		char *filename;
		int tile_count = 0, i = 0;
		pthread_t thread_array[tile_count];
		struct thread_args args;

		///////////////   LOOP   ///////////////////////////////////
		for (i = 0; i < tile_count; i++) {
			sprintf(args.filename, "%d", i);
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &sendThread, (void *)&args);
		}
		/* wait for threads to end */
		for ( i = 0; i < tile_count; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
