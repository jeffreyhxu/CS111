#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745
all: lab3b

lab3b: lab3b.cpp
	g++ -g -Wall lab3b.cpp -o lab3b
dist: lab3b.cpp Makefile README
	tar -czf lab3b-404768745.tar.gz lab3b.cpp Makefile README
clean:
	rm -f lab3b lab3b-404768745.tar.gz
