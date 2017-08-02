#	Name: Jeffrey Xu
#	Email: jeffreyhxu@gmail.com
#	ID: 404768745
LD_PRELOAD = ~/pprof/lib/libprofiler.so.0
CPUPROFILE

all: lab2_list

lab2_list: lab2_list.o SortedList.o
	gcc -g lab2_list.o SortedList.o -o lab2_list -pthread
lab2_list.o: lab2_list.c SortedList.h
	gcc -g -c lab2_list.c
SortedList.o: SortedList.c SortedList.h
	gcc -g -c SortedList.c
