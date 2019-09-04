#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>

#define PORT 8080
#define ROWS 8
#define COLUMNS 8
#define USPF 1070000
#define SPF 1.07
#define TILE_COUNT 1
#define GOP_COUNT 1
#define BUFFER_SIZE 64000

struct thread_args {
	int tile_num;
};

char *setFilename(char *filename, char *gop_num, char *tile_num) {
	memset(filename, 0, sizeof(filename));
	strcat(filename, "./received/gop");
	strcat(filename, gop_num);
	strcat(filename, "_tile");
	strcat(filename, tile_num);
	strcat(filename, ".bin");
}

int getGOP(int server_sock, char *tile_num, char *gop_num) {
	printf("\nNNNEEEWWW\n");
	int len = 0, bytes = 0;
	time_t start_time = 0;
	char filename[1024], buffer[BUFFER_SIZE];
	FILE *fp = NULL;
	struct timeval timeout;

	struct sockaddr_in cliaddr;
	// set the file name
	setFilename(filename, gop_num, tile_num);
	fp = fopen(filename, "wb");
	// wait to get first packet of data (block) if this is the first gop
	// if we've already began the process of receiving gops
	// then the initial read will also have a SPF timeout
	if (strcmp(gop_num, "0") != 0)
		start_time = time(NULL);
	bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);	// block untill we get the first packet
	// if we are receiving the first gop
	// then start the SPF timeout after we get
	// the first packet
	if (strcmp(gop_num, "0") == 0)
		start_time = time(NULL);
	// get additional packets
	int total = bytes;
	while (bytes > 0 && (SPF-((double)time(NULL)-start_time)) > 0) {
		// set timeout
		timeout.tv_sec = 0;
		timeout.tv_usec = 1000000 * (SPF-((double)time(NULL) - start_time));
		setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
		// write the received packet to a file
		fwrite(buffer, 1, bytes, fp);
		// clear the buffer
		memset(buffer, 0, sizeof(buffer));
		printf("\nA: timeout: %ld\n", timeout.tv_usec);
		bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);
		printf("\ntotal bytes: %d, time used: %f\n", bytes, (((double)time(NULL)-start_time)));
		printf("\nz\n");
		total += bytes;
		//printf("\ntotal bytes: %d -- bytes: %d -- time spent: %f\n", total, bytes, (double)(time(NULL) - start_time));
	}
	// write any remaining bytes to file before closing
	if (bytes > 0)
		fwrite(buffer, 1, bytes, fp);
	fclose(fp);
	timeout.tv_sec = 0;
	timeout.tv_usec = 1000000 * SPF;
	setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));
	return 0;
}

void *receiveThread(void *arguments) {
		int server_sock = 0, i = 0;
		struct sockaddr_in servaddr;
		char gop_num[5], tile_num[5];
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
		/* listens for each successive frame
			 in 1.07 second intervals
		*/
		for (i = 0; i < GOP_COUNT; i++) {
			sprintf(gop_num, "%d", i);
			sprintf(tile_num, "%d", args->tile_num);
			// listen for messages at specified port
			getGOP(server_sock, tile_num, gop_num);
		}
}

int main(int argc, char const *argv[]) {
		int i = 0;
		pthread_t thread_array[TILE_COUNT];		// array for each thread
		struct thread_args args[TILE_COUNT];	// array for argument structs

		/* create udp threads all listening for input */
		for (i = 0; i < TILE_COUNT; i++) {
			args[i].tile_num = i;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &receiveThread, (void *)&args[i]);
		}
		/* wait for threads to end */
		for ( i = 0; i < TILE_COUNT; i++) {
			pthread_join(thread_array[i], NULL);
		}
		return 0;
}
