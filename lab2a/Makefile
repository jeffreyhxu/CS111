#	Name: Jeffrey Xu
#	Email: jeffreyhx@gmail.com
#	ID: 404768745
all: lab2_add lab2_list

build: all

lab2_add: lab2_add.c
	gcc -g lab2_add.c -o lab2_add -pthread
lab2_list: lab2_list.o SortedList.o
	gcc -g lab2_list.o SortedList.o -o lab2_list -pthread
lab2_list.o: lab2_list.c SortedList.h
	gcc -g -c lab2_list.c
SortedList.o: SortedList.c SortedList.h
	gcc -g -c SortedList.c
tests: lab2_add.csv lab2_list.csv

lab2_add.csv: lab2_add
	rm -f lab2_add.csv
	for threads in 1 2 4 8 12 ; do \
		for iterations in 10 20 40 80 100 200 400 1000 2000 4000 10000 20000 40000 100000 ; do \
			./lab2_add --threads=$$threads --iterations=$$iterations >>lab2_add.csv ; \
			./lab2_add --threads=$$threads --iterations=$$iterations --yield >>lab2_add.csv ; \
		done ; \
	done
	for threads in 2 4 8 12 ; do \
		./lab2_add --threads=$$threads --iterations=10000 --yield --sync=m >>lab2_add.csv ; \
		./lab2_add --threads=$$threads --iterations=1000 --yield --sync=s >>lab2_add.csv ; \
		./lab2_add --threads=$$threads --iterations=10000 --yield --sync=c >>lab2_add.csv ; \
	done
	for threads in 1 2 4 8 12 ; do \
		./lab2_add --threads=$$threads --iterations=10000 --sync=m >>lab2_add.csv ; \
		./lab2_add --threads=$$threads --iterations=10000 --sync=s >>lab2_add.csv ; \
		./lab2_add --threads=$$threads --iterations=10000 --sync=c >>lab2_add.csv ; \
	done
lab2_list.csv: lab2_list
	rm -f lab2_list.csv
	for iterations in 10 100 1000 10000 20000 ; do \
		./lab2_list --iterations=$$iterations >>lab2_list.csv ; \
	done
	-for threads in 2 4 8 12 ; do \
		for iterations in 1 10 100 1000 ; do \
		./lab2_list --threads=$$threads --iterations=$$iterations >>lab2_list.csv ; \
		done ; \
		for iterations in 1 2 4 8 16 32 ; do \
		./lab2_list --threads=$$threads --iterations=$$iterations --yield=i >>lab2_list.csv ; \
		./lab2_list --threads=$$threads --iterations=$$iterations --yield=d >>lab2_list.csv ; \
		./lab2_list --threads=$$threads --iterations=$$iterations --yield=il >>lab2_list.csv ; \
		./lab2_list --threads=$$threads --iterations=$$iterations --yield=dl >>lab2_list.csv ; \
		done ; \
	done
	for yield in i d il dl ; do \
		./lab2_list --threads=12 --iterations=32 --yield=$$yield --sync=m >>lab2_list.csv ; \
		./lab2_list --threads=12 --iterations=32 --yield=$$yield --sync=s >>lab2_list.csv ; \
	done
	for threads in 1 2 4 8 12 16 24 ; do \
		./lab2_list --threads=$$threads --iterations=1000 --sync=m >>lab2_list.csv ; \
		./lab2_list --threads=$$threads --iterations=1000 --sync=s >>lab2_list.csv ; \
	done
graphs: lab2_add_graphs lab2_list_graphs

lab2_add_graphs: lab2_add.csv
	./lab2_add.gp
lab2_list_graphs: lab2_list.csv
	./lab2_list.gp
dist: lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv graphs lab2_add.gp lab2_list.gp README
	tar -czf lab2a-404768745.tar.gz lab2_add.c SortedList.h SortedList.c lab2_list.c Makefile lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add.gp lab2_list.gp README
clean:
	rm -f lab2_add.csv lab2_list.csv lab2_add-1.png lab2_add-2.png lab2_add-3.png lab2_add-4.png lab2_add-5.png lab2_list-1.png lab2_list-2.png lab2_list-3.png lab2_list-4.png lab2_add lab2_list lab2a-404768745.tar.gz lab2_list.o SortedList.o
