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
#define GOP_COUNT 1
#define BUFFER_SIZE 64000

struct thread_args {
	int tile_num;
};

/*
 * The memmem() function finds the start of the first occurrence of the
 * substring 'needle' of length 'nlen' in the memory area 'haystack' of
 * length 'hlen'.
 *
 * The return value is a pointer to the beginning of the sub-string, or
 * NULL if the substring is not found.
 */
void *memmem(const void *haystack, size_t hlen, const void *needle, size_t nlen) {
    int needle_first;
    const void *p = haystack;
    size_t plen = hlen;

    if (!nlen)
        return NULL;

    needle_first = *(unsigned char *)needle;

    while (plen >= nlen && (p = memchr(p, needle_first, plen - nlen + 1)))
    {
        if (!memcmp(p, needle, nlen))
            return (void *)p;

        p++;
        plen = hlen - (p - haystack);
    }

    return NULL;
}

char *setFilename(char *filename, char *gop_num, char *tile_num) {
	memset(filename, 0, sizeof(filename));
	strcat(filename, "./received/gop");
	strcat(filename, gop_num);
	strcat(filename, "_tile");
	strcat(filename, tile_num);
	strcat(filename, ".bin");
}
// 0x00 0x00 0x00 0x01 0x40 0x01 0x0c 0x01 0xff 0xff 0x01 0x60 0x00 0x00 0x03 0x00
int getGOP(int server_sock, char *tile_num) {
	char header[] = {
		0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00,
		0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5a, 0x95, 0x98, 0x09, 0x00, 0x00, 0x00, 0x01,
		0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
		0x00, 0x5a, 0xa0, 0x07, 0x82, 0x01, 0xe1, 0x65, 0x95, 0x9a, 0x49, 0x32, 0xb8, 0x04, 0x00, 0x00
	};

	char gop_num[5];
	int len = 0, bytes = 0, i = 0, total = 0;
	char filename[1024], buffer[BUFFER_SIZE];
	FILE *fp = NULL;

	struct timeval timeout;
	struct sockaddr_in cliaddr;

	memset(buffer, 0, sizeof(buffer));
	bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);	// block untill we get the first packet
	sprintf(gop_num, "%d", i);
	setFilename(filename, gop_num, tile_num);
	fp = fopen(filename, "wb");

	//timeout.tv_sec = 2;
	//timeout.tv_usec = 0;
	//setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	total = bytes;
	do {
		// write the received packet to a file
		fwrite(buffer, 1, bytes, fp);
		// clear the buffer
		memset(buffer, 0, sizeof(buffer));
		bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);
		total += bytes;
		// memmem(buffer, bytes, header, sizeof(header))
		if (memmem(buffer, sizeof(buffer), header, sizeof(header)) != NULL) {
		//if (strstr(buffer, header) != NULL) {
			fclose(fp);
			i += 1;
			// set the file name
			sprintf(gop_num, "%d", i);
			setFilename(filename, gop_num, tile_num);
			fp = fopen(filename, "wb");
			total = bytes;
		}
	} while (bytes > 0);
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
