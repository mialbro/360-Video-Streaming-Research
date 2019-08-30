#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#include <pthread.h>
#define PORT 8080

int main(int argc, char const *argv[]) {
		int tile_count = 3, i = 0;
		int server_sock[2];
		pthread_t thread_array[tile_count];
		struct sockaddr_in servaddr[2];

		/* configure server socket */
		servaddr[0].sin_family = AF_INET;
		servaddr[0].sin_addr.s_addr = INADDR_ANY;
		servaddr[0].sin_port = htons(PORT);

		// Creating socket file descriptor
		if ((server_sock[0] = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
				perror("socket failed");
				exit(EXIT_FAILURE);
		}

		int optval = 1;
		int set = setsockopt(server_sock[0], SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
		printf("SET: %d\n", set);

		// SO_REUSEPORT

		// bind the socket to port 8080
		if (bind(server_sock[0], (struct sockaddr *)&servaddr[0], sizeof(servaddr[0])) < 0) {
				perror("bind failed");
				exit(EXIT_FAILURE);
		}
		else {
			printf("Socket binded\n");
		}

		/* configure server socket */
		servaddr[1].sin_family = AF_INET;
		servaddr[1].sin_addr.s_addr = INADDR_ANY;
		servaddr[1].sin_port = htons(PORT);

		// Creating socket file descriptor
		if ((server_sock[1] = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
				perror("socket failed");
				exit(EXIT_FAILURE);
		}

		int optval1 = 1;
		set = setsockopt(server_sock[1], SOL_SOCKET, SO_REUSEPORT, &optval1, sizeof(optval1));
		printf("SET: %d\n", set);

		// SO_REUSEPORT

		// bind the socket to port 8080
		if (bind(server_sock[1], (struct sockaddr *)&servaddr[1], sizeof(servaddr[0])) < 0) {
				perror("bind failed");
				exit(EXIT_FAILURE);
		}
		else {
			printf("Socket binded\n");
		}

		return 0;
}
