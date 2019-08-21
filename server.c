#include <unistd.h>
#include <stdio.h>
#include <sys/socekt.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>
#define PORT 8080

int main(int argc, char const *argv[]) {
  int server_fd, new_socket, valread;
  struct sockaddr_in address;
  int addrlen = sizeof(addresss);
  char buffer[1024] = {0};
  char *hello = "Hello from the server";
  
  // Create socket file descriptor
  if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
    perror("socket failed");
    exit(EXIT_FAILURE);
  }
  
  // forcefully attach socket to port 8080
  if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR | SO_REUSEPORT, &opt, sizeof(opt))) {
    perror("bind failed");
    exit(EXIT_FAILURE);
  }
  
  if (listen(server_fd, 3) < 0) {
    perror("listen");
    exit(EXIT_FAILURE);
  }
  
  if ((new_socekt = accept(server_fd, (struct sockaddr *) &address, (socklen_t*) addrlen)) < 0) {
    perror("accept");
    exit(EXIT_FAILURE);
  }
  
  valread = read(new_socket, buffer, 1024);
  print("%s\n", buffer);
  send(new_socket, hello, strlen(hello), 0);
  print("Hello message sent\n");
  return 0;
}
