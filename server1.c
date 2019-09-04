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
#define TILE_COUNT 2
#define GOP_COUNT 2
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

int getGOP(int server_sock, char *tile_num) {
	char header[] = {"\x00\x00\x00\x01@\x01\x0c\x01\xff\xff\x01`\x00\x00\x03\x00\x90\x00\x00\x03\x00\x00\x03\x00Z\x95\x98\t\x00\x00\x00\x01B\x01\x01\x01`\x00\x00\x03\x00\x90\x00\x00\x03\x00\x00\x03"};
	char gop_num[5];
	int len = 0, bytes = 0, i = 0;
	char filename[1024], buffer[BUFFER_SIZE];
	FILE *fp = NULL;

	struct timeval timeout;
	struct sockaddr_in cliaddr;

	bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);	// block untill we get the first packet
	// set the file name
	sprintf(gop_num, "%d", i);
	setFilename(filename, gop_num, tile_num);
	fp = fopen(filename, "wb");

	timeout.tv_usec = USPF;
	setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout));

	while (bytes > 0) {
		// write the received packet to a file
		fwrite(buffer, 1, bytes, fp);
		// clear the buffer
		memset(buffer, 0, sizeof(buffer));
		bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);
		if (strstr(header, buffer) != NULL) {
			fclose(fp);
			i += 1;
			// set the file name
			sprintf(gop_num, "%d", i);
			setFilename(filename, gop_num, tile_num);
			if (bytes > 0)
				fp = fopen(filename, "wb");
		}
	}
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
		sprintf(tile_num, "%d", args->tile_num);
		// listen for messages at specified port
		getGOP(server_sock, tile_num);
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
