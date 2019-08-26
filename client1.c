#include <stdio.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <pthread.h>

#define PORT 8080

struct thread_args {
	char *filename;
}

/* read the file to send and send it to server  */
void *sendGOP(void *arguments) {
	int buffer_size = 64000, file_size = 0, packet_size = 0, bytes = 0, client_sock = 0;
	char buffer[buffer_size];
	struct sockaddr_in servaddr;
	FILE *fp = NULL;

	struct arg_struct *args = (struct arg_struct *)arguments;

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
	fp = fopen(args->filename, "r");
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

int main(int argc, char const *argv[]) {
		char *filename;
		struct thread_args args;

		///////////////   LOOP   ///////////////////////////////////
		args.filename = 'x';
		/* create thread to send videos */
		pthread_t thread;
		pthread_create(&thread, NULL, &sendGOP, (void *)&args);
		/* wait for thread to return */
		pthread_join(thread);

		return 0;
}
