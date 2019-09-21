client: client.o
    gcc -o client -lpthread client.o

server: server.o fileio.o
		$(CC) -o server -lpthread server.o fileio.o

clean:
		rm -f server client fileio
