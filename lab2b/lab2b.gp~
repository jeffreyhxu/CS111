#! /usr/local/cs/bin/gnuplot
#
# plot total number of operations per second for mutex and spin-lock
#
# input: lab2b_list.csv
#	1. test name
#	2. threads
#	3. iterations
#	4. lists
#	5. total operations
#	6. total run time
#	7. average time per operation
#	8. average wait for lock, not included in original lab2_list
#
# output:
#	lab2b_1.png ... total operations per second vs threads (with protection)
#	lab2b_2.png ... average time per lock and operation vs threads with mutexes
#	lab2b_3.png ... threads and iterations that run without failure with 4 lists and yielding
#	lab2b_4.png ... throughput vs number of threads for mutex synchronized partitioned lists.
#	lab2b_5.png ... throughput vs number of threads for spin-lock-synchronized partitioned lists.
#

set terminal png
set datafile separator ","

set title "List-1: Operations per Second vs Threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Operations per Second"
set logscale y 10
set output 'lab2b_1.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title 'mutex' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title 'spin-lock' with linespoints lc rgb 'blue'

set title "List-2: Average Time for Lock and Operation vs Number of Mutexed Threads"
set xlabel "Threads"
set logscale x 2
set ylabel "Average Time (ns)"
set logscale y 10
set output 'lab2b_2.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($8) \
     	title 'wait-for-lock time' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):($7) \
     	title 'time per operation' with linespoints lc rgb 'blue'

set title "List-3: Successful Threads and Iterations with Four Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Successful Iterations"
set logscale y 2
set output 'lab2b_3.png'

plot \
     "< grep 'list-id-none,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
     	title 'unprotected' with points lc rgb 'red', \
     "< grep 'list-id-s,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
     	title 'spin-lock' with points lc rgb 'green', \
     "< grep 'list-id-m,[0-9]*,[0-9]*,4,' lab2b_list.csv" using ($2):($3) \
     	title 'mutex' with points lc rgb 'blue'

set title "List-4: Throughput vs Number of Threads for Mutexed Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Operations per Second"
set logscale y 10
set output 'lab2b_4.png'

plot \
     "< grep 'list-none-m,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-m,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-m,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '8 list' with linespoints lc rgb 'blue', \
     "< grep 'list-none-m,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '16 lists' with linespoints lc rgb 'pink'

set title "List-5: Throughput vs Number of Threads for Spin-Locked Partitioned Lists"
set xlabel "Threads"
set logscale x 2
set ylabel "Operations per Second"
set logscale y 10
set output 'lab2b_5.png'

plot \
     "< grep 'list-none-s,[0-9]*,1000,1,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '1 list' with linespoints lc rgb 'red', \
     "< grep 'list-none-s,[0-9]*,1000,4,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '4 lists' with linespoints lc rgb 'green', \
     "< grep 'list-none-s,[0-9]*,1000,8,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '8 list' with linespoints lc rgb 'blue', \
     "< grep 'list-none-s,[0-9]*,1000,16,' lab2b_list.csv" using ($2):(1000000000/($7)) \
     	title '16 lists' with linespoints lc rgb 'pink'