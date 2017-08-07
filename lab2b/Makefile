#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745

all: lab2_list

lab2_list: lab2_list.o SortedList.o
	gcc -g lab2_list.o SortedList.o -o lab2_list -pthread
lab2_list.o: lab2_list.c SortedList.h
	gcc -g -c lab2_list.c
SortedList.o: SortedList.c SortedList.h
	gcc -g -c SortedList.c
tests: lab2b_list.csv

lab2b_list.csv: lab2_list
	rm -f lab2b_list.csv
	for threads in 1 2 4 8 12 16 24 ; do \
		./lab2_list --threads=$$threads --iterations=1000 --sync=m >>lab2b_list.csv ; \
		./lab2_list --threads=$$threads --iterations=1000 --sync=s >>lab2b_list.csv ; \
	done
	-for threads in 1 4 8 12 16 ; do \
		for iterations in 1 2 4 8 16 ; do \
			./lab2_list --threads=$$threads --iterations=$$iterations --lists=4 --yield=id >>lab2b_list.csv ; \
		done ; \
	done
	for threads in 1 4 8 12 16 ; do \
		for iterations in 10 20 40 80 ; do \
			./lab2_list --threads=$$threads --iterations=$$iterations --lists=4 --yield=id --sync=s >>lab2b_list.csv ; \
			./lab2_list --threads=$$threads --iterations=$$iterations --lists=4 --yield=id --sync=m >>lab2b_list.csv ; \
		done ; \
	done
	for threads in 1 2 4 8 12 ; do \
		for lists in 4 8 16 ; do \
			./lab2_list --threads=$$threads --iterations=1000 --lists=$$lists --sync=s >>lab2b_list.csv ; \
			./lab2_list --threads=$$threads --iterations=1000 --lists=$$lists --sync=m >>lab2b_list.csv ; \
		done ; \
	done #No 1 list test; the first set of tests takes care of that.
profile: lab2_list
	LD_PRELOAD=~/pprof/lib/libprofiler.so.0 CPUPROFILE=rawprof ./lab2_list --threads=12 --iterations=1000 --sync=s
	~/pprof/bin/pprof --text lab2_list rawprof >profile.out
	~/pprof/bin/pprof --list=spinlock lab2_list rawprof >>profile.out
graphs: tests
	./lab2b.gp
dist: SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile graphs lab2b.gp README
	tar -czf lab2b-404768745.tar.gz SortedList.h SortedList.c lab2_list.c Makefile lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b.gp README
clean:
	rm -f SortedList.o lab2_list.o lab2_list lab2b_list.csv profile.out lab2b_1.png lab2b_2.png lab2b_3.png lab2b_4.png lab2b_5.png lab2b-404768745.tar.gz
