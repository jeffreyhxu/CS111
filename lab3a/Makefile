#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745
all: lab3a

lab3a: lab3a.c ext2_fs.h
	gcc -g -Wall lab3a.c -o lab3a
dist: lab3a.c Makefile README
	tar -czf lab3a-404768745.tar.gz lab3a.c Makefile README
clean:
	rm -f lab3a lab3a-404768745.tar.gz
