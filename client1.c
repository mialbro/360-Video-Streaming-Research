#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#define PORT 8080

int main(int argc, char const *argv[]) {
    int client_sock, bytes = 0, file_size, packet_size;
    struct sockaddr_in servaddr;
    int buffer_size = 1024;
    char buffer[buffer_size];
    FILE *fp = NULL;

		/* create client socket */
    if ((client_sock = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("\n Socket creation error \n");
        return -1;
    }

		/* configure socket */
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

		/* open file to send */
		fp = fopen("slow.mp4", "r");
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
		return 0;
}
