#	NAME: Jeffrey Xu
#	EMAIL: jeffreyhxu@gmail.com
#	ID: 404768745

all: lab0

lab0: lab0.c
	gcc -g lab0.c -o lab0
check: lab0
	echo "Errors 1, 2, 3, and 4 in sequence are expected."
	-./lab0 --badoption
	rm -f testin.txt
	-./lab0 --input="testin.txt"
	-./lab0 --output=""
	-./lab0 --segfault --catch
	echo -e "Lorem ipsum dolor sit amet\nLine 2" >testin.txt
	./lab0 --input=testin.txt --output=testout.txt || echo "Failure to accurately copy text between files"
clean:
	rm -f lab0 testin.txt testout.txt lab0-404768745.tar.gz
dist: lab0-404768745.tar.gz

lab0-404768745.tar.gz: lab0.c Makefile backtrace.png breakpoint.png README
	tar -czf lab0-404768745.tar.gz lab0.c Makefile backtrace.png breakpoint.png README
