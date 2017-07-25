all: client server

client: lab1b-client

lab1b-client: lab1b-client.c
	gcc -g lab1b-client.c -o lab1b-client
server: lab1b-server

lab1b-server: lab1b-server.c
	gcc -g lab1b-server.c -o lab1b-server
clean:
	rm -f lab1b-client lab1b-server lab1b-404768745.tar.gz
dist: lab1b-client.c lab1b-server.c Makefile my.key README
	tar -czf lab1b-404768745.tar.gz lab1b-client.c lab1b-server.c Makefile my.key README
