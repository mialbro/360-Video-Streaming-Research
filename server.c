#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>
#include <stdlib.h>
#include <math.h>
#include <time.h>

#define HOST 127.0.0.1
#define PORT 8080
#define ROWS 8
#define COLUMNS 8
#define USPF 1070000
#define SPF 1.07
#define TILE_COUNT 64
#define GOP_COUNT 10
#define BUFFER_SIZE 64000

struct thread_args {
	int tile_num;
};
		
		

void *ackThread() {
	int server_sock = 0, client_sock = 0, len = 0;
	struct sockaddr_in servaddr, cliaddr;
	char buffer[BUFFER_SIZE];
	
	// create socket file descriptor
	server_sock = socket(AF_INET, SOCK_DGRAM, 0);
	client_sock = socket(AF_INET, SOCK_DGRAM, 0);
	
	// configure server socket
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT + 100);
	servaddr.sin_addr.s_addr = inet_addr("192.168.0.2");
	
	// configure server socket
	cliaddr.sin_family = AF_INET;
	cliaddr.sin_port = htons(PORT + 100);
	cliaddr.sin_addr.s_addr = inet_addr("192.168.0.1");
	
	// bind the socket to the specified port
	if (bind(server_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	
	while (1) {
		recvfrom(server_sock, buffer, BUFFER_SIZE, MSG_WAITALL, (struct sockaddr*)&cliaddr, &len);
		printf("\n1\n");
		sendto(client_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, sizeof(servaddr));
		printf("\n1\n");
		memset(buffer, 1, sizeof(buffer));
	}
}

/* 
 * calculate the row / column value based on the tile number
 * and store value as a string
 */
void setRowCol(char *row, char *col, int tile_num) {
	sprintf(row, "%d", (int)((tile_num) / COLUMNS) + 1);
	sprintf(col, "%d", (tile_num % COLUMNS) + 1);
}

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

char *setFilename(char *filename, char *gop_num, char *tile_num, char *row, char *col) {
	memset(filename, 0, sizeof(filename));
	strcat(filename, "./received/gop");
	strcat(filename, gop_num);
	strcat(filename, "_tile");
	strcat(filename, tile_num);
	strcat(filename, "_");
	strcat(filename, row);
	strcat(filename, "-");
	strcat(filename, col);
	strcat(filename, ".bin");
}


int getGOP(int server_sock, char *tile_num, char *row, char *col) {
	char header[] = {
		0x00, 0x00, 0x00, 0x01, 0x40, 0x01, 0x0c, 0x01, 0xff, 0xff, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00,
		0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03, 0x00, 0x5a, 0x95, 0x98, 0x09, 0x00, 0x00, 0x00, 0x01,
		0x42, 0x01, 0x01, 0x01, 0x60, 0x00, 0x00, 0x03, 0x00, 0x90, 0x00, 0x00, 0x03, 0x00, 0x00, 0x03,
		0x00, 0x5a, 0xa0, 0x07, 0x82, 0x01, 0xe1, 0x65, 0x95, 0x9a, 0x49, 0x32, 0xb8, 0x04, 0x00, 0x00
	};

	char gop_num[5];
	int len = 0, bytes = 0, curr_gop = 0, total = 0, fileOpen = 0;
	char filename[1024], buffer[BUFFER_SIZE];
	FILE *fp = NULL;

	struct sockaddr_in cliaddr;
	struct timeval timeout;
	
	
	timeout.tv_sec = 1;
	timeout.tv_usec = 70000;
	setsockopt(server_sock, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout));
	
	// read in the given file for every frame
	while (curr_gop <= GOP_COUNT) {
		memset(buffer, 0, sizeof(buffer));
		// read in data
		bytes = recvfrom(server_sock, buffer, BUFFER_SIZE, 0, (struct sockaddr*)&cliaddr, &len);
		// we just read in a new file
		if (memmem(buffer, sizeof(buffer), header, sizeof(header)) != NULL) {
			if (fileOpen == 1) {
				fclose(fp);
			}
			curr_gop += 1;
			// create new file to begin saving to it
			sprintf(gop_num, "%d", curr_gop-1);
			setFilename(filename, gop_num, tile_num, row, col);
			//printf("\n%s\n", filename);
			fp = fopen(filename, "wb");
			fileOpen = 1;
			fwrite(buffer, 1, bytes, fp);
		}
		else if (strcmp(buffer, "100") == 0) {
			curr_gop += 1;
			if (fileOpen == 1) {
				fclose(fp);
				fileOpen = 0;
			}
		}
		// continue saving to current file
		else if (bytes > 0) {
			fwrite(buffer, 1, bytes, fp);
		}
		else if (bytes == -1 && curr_gop > 0) {
			if (fileOpen == 1) {
				fclose(fp);
				fileOpen = 0;
			}
			break;
		}
	}
	return 0;
}

void *receiveThread(void *arguments) {
		int server_sock = 0, i = 0;
		struct sockaddr_in servaddr;
		char gop_num[5], tile_num[5], row[5], col[5];
		struct thread_args *args = arguments;
		// Creating socket file descriptor
		if ((server_sock = socket(AF_INET, SOCK_DGRAM, 0)) == 0) {
				perror("socket failed");
				exit(EXIT_FAILURE);
		}
		// configure server socket
		servaddr.sin_family = AF_INET;
		servaddr.sin_port = htons(PORT + args->tile_num);
		servaddr.sin_addr.s_addr = inet_addr("192.168.0.2");
		// bind the socket to the specified port
		if (bind(server_sock, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
				perror("bind failed");
				exit(EXIT_FAILURE);
		}
		/* listens for each successive frame
			 in 1.07 second intervals
		*/
		// set the row / column values
		setRowCol(row, col, args->tile_num);
		sprintf(tile_num, "%d", args->tile_num);
		// listen for messages at specified port
		getGOP(server_sock, tile_num, row, col);
}

int main(int argc, char const *argv[]) {
		int i = 0;
		pthread_t thread_array[TILE_COUNT], ack;		// array for each thread
		struct thread_args args[TILE_COUNT];	// array for argument structs

		/* create udp threads for listening at each port for the corresponding tile */
		for (i = 0; i < TILE_COUNT; i++) {
			args[i].tile_num = i;
			/* create thread to send videos */
			pthread_create(&thread_array[i], NULL, &receiveThread, (void *)&args[i]);
		}
		pthread_create(&ack, NULL, ackThread, NULL);
		/* wait for threads to end */
		for ( i = 0; i < TILE_COUNT; i++) {
			pthread_join(thread_array[i], NULL);
			pthread_join(thread_array[i], NULL);
		}
		pthread_join(ack, NULL);
		return 0;
}
