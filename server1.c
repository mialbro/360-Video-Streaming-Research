#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#define PORT 8080

struct thread_args {
	int tile_num;
};

int getGOP(int server_sock, char *buffer, int buffer_size, int port) {
	int len = 0, bytes = 0;
	FILE *fp = 0;
	fp = fopen(port, "w");
	struct sockaddr_in cliaddr;

	// wait to get first packet of data (block)
	bytes = recvfrom(server_sock, buffer, buffer_size, 0, (struct sockaddr*)&cliaddr, &len);
	/*	set timeout	after initial read  */
	struct timeval timeout;
	timeout.tv_sec = 0;
	timeout.tv_usec = 100000;
	if (setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
		perror("Error");
		return -1;
	}
	// get additional packets
	do {
		// write the received packet to a file
		fwrite(buffer, 1, bytes, fp);
		// clear the buffer
		memset(buffer, 0, sizeof(buffer));
		// get additional packets (don't block)
	} while ((bytes = recvfrom(server_sock, buffer, buffer_size, 0, (struct sockaddr*)&cliaddr, &len)) > 0);
	fclose(fp);
	return 0;
}

void *receiveThread(void *arguments) {
		int server_sock = 0;
		struct sockaddr_in servaddr;
		int buffer_size = 64000;
		char buffer[buffer_size];

		struct thread_args *args = arguments;
		// Creating socket file descriptor
		if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
				perror("socket failed");
				exit(EXIT_FAILURE);
		}
		// configure server socket
		servaddr.sin_family = AF_INET;
		servaddr.sin_addr.s_addr = INADDR_ANY;
		servaddr.sin_port = htons(PORT + args->tile_num);
		// bind the socket to the specified port
		if (bind(server_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
				perror("bind failed");
				exit(EXIT_FAILURE);
		}
		// listen for messages at specified port
		getGOP(server_sock, buffer, buffer_size, PORT + args->tile_num);
}

// fuser -TERM 8080/udp

int main(int argc, char const *argv[]) {
		int tile_count = 60, i = 0;
		pthread_t thread_array[tile_count];		// array for each thread
		struct thread_args args[tile_count];	// array for argument structs

		/* create udp threads all listening for input */
		for (i = 0; i < tile_count; i++) {
			args[i].tile_num = i;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &receiveThread, (void *)&args[i]);
		}
		/* wait for threads to end */
		for ( i = 0; i < tile_count; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
