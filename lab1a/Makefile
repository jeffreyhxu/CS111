#	NAME: Jeffrey Xu
#	EMAIL: jeffreyhxu@gmail.com
#	ID: 404768745
all: lab1a

lab1a: lab1a.c
	gcc -g lab1a.c -o lab1a
clean:
	rm -f lab1a lab1a-404768745.tar.gz
dist: lab1a-404768745.tar.gz

lab1a-404768745.tar.gz: lab1a.c Makefile README
	tar -czf lab1a-404768745.tar.gz lab1a.c Makefile README
