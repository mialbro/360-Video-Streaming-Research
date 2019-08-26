#include <unistd.h>
#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080


int main(int argc, char const *argv[]) {
    int server_sock, len, bytes;
    struct sockaddr_in servaddr, cliaddr;
    int buffer_size = 1024;
    char buffer[buffer_size];
    FILE *fp = NULL;

    // Creating socket file descriptor
    if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }

		/* configure server socket */
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // bind the socket to port 8080
    if (bind(server_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

		fp = fopen("a.mp4", "w");

		// wait to get first packet of data (block)
		bytes = recvfrom(server_sock, buffer, buffer_size, 0, (struct sockaddr*)&cliaddr, &len);

		/*	set timeout	after initial read  */
		struct timeval timeout;
		timeout.tv_sec = 0;
		timeout.tv_usec = 100000;
		if (setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char *)&timeout, sizeof(timeout)) < 0) {
			perror("Error");
			return 0;
		}

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
