/* Name: Jeffrey Xu
 * Email: jeffreyhxu@gmail.com
 * ID: 404768745
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <getopt.h>
#include <errno.h>
#include <time.h>
#include <pthread.h>
#include <sched.h>
#include <string.h>

struct add_args{
  long long *counter;
  int iterations;
};

int spinlocker = 0;
void spinlock(){
  int locked;
  do{
    locked = __sync_lock_test_and_set(&spinlocker, 1);
  } while(locked);
}

void spinunlock(){
  __sync_lock_release(&spinlocker);
}

int opt_yield = 0, opt_sync = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void add(long long *pointer, long long value){
  if(opt_sync == 1)
    pthread_mutex_lock(&mutex);
  else if(opt_sync == 2)
    spinlock();
  long long sum = *pointer + value;
  if(opt_yield)
    sched_yield();
  *pointer = sum;
  if(opt_sync == 1)
    pthread_mutex_unlock(&mutex);
  else if(opt_sync == 2)
    spinunlock();
}

void casadd(long long *pointer, long long value){
  long long old, success;
  do{
    old = *pointer;
    long long new = old + value;
    success = __sync_val_compare_and_swap(pointer, old, new);
  } while(success != old);
}

void *thread_add(void *arg){
  long long *counter = ((struct add_args *)arg)->counter;
  int its = ((struct add_args *)arg)->iterations;
  for(int i = 0; i < its; i++){
    if(opt_sync == 3)
      casadd(counter, 1);
    else
      add(counter, 1);
  }
  for(int i = 0; i < its; i++){
    if(opt_sync == 3)
      casadd(counter, -1);
    else
      add(counter, -1);
  }
  pthread_exit(NULL);
}

int main(int argc, char **argv){
  struct option lopts[5] =
    {
      {"threads", required_argument, NULL, 1},
      {"iterations", required_argument, NULL, 2},
      {"yield", no_argument, &opt_yield, 1},
      {"sync", required_argument, NULL, 3},
      {0,0,0,0}
    };
  int opti, threads = 1, iterations = 1;
  for(;;){
    int opt_stat = getopt_long(argc, argv, "", lopts, &opti);
    if(opt_stat == -1)
      break;
    else if(opt_stat == 1 && optarg != NULL)
      threads = atoi(optarg);
    else if(opt_stat == 2 && optarg != NULL)
      iterations = atoi(optarg);
    else if(opt_stat == 3 && optarg != NULL){
      if(!strcmp(optarg, "m"))
	opt_sync = 1;
      else if(!strcmp(optarg, "s"))
	opt_sync = 2;
      else if(!strcmp(optarg, "c"))
	opt_sync = 3;
      else{
	fprintf(stderr, "Usage: lab2_add [--threads=#] [--iterations=#] [--yield] [--sync=(m|s|c)]");
	exit(1);
      }
    }
    else if(opt_stat != 0){
      fprintf(stderr, "Usage: lab2_add [--threads=#] [--iterations=#] [--yield] [--sync=(m|s|c)]");
      exit(1);
    }
  }
  long long counter = 0;
  pthread_t *thread_ids = malloc(sizeof(pthread_t) * threads);
  if(thread_ids == NULL){
    fprintf(stderr, "Error using malloc to create array of thread ids: %s\n", strerror(errno));
    exit(1);
  }
  struct add_args args;
  args.counter = &counter;
  args.iterations = iterations;
  struct timespec start_time;
  if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1){
    fprintf(stderr, "Error using clock_gettime to get monotonic start time: %s\n", strerror(errno));
    exit(1);
  }
  for(int i = 0; i < threads; i++){
    int t_status = pthread_create(thread_ids + i, NULL, thread_add, &args);
    if(t_status != 0){
      fprintf(stderr, "Error using pthread_create to create a thread: %s\n", strerror(t_status));
      exit(1);
    }
  }
  for(int i = 0; i < threads; i++){
    int t_status = pthread_join(thread_ids[i], NULL);
    if(t_status != 0){
      fprintf(stderr, "Error using pthread_join to join threads: %s\n", strerror(t_status));
      exit(1);
    }
  }
  struct timespec end_time;
  if(clock_gettime(CLOCK_MONOTONIC, &end_time) == -1){
    fprintf(stderr, "Error using clock_gettime to get monotonic end time: %s\n", strerror(errno));
    exit(1);
  }
  free(thread_ids);
  if(opt_sync == 1)
    pthread_mutex_destroy(&mutex);
  char *test_name = malloc(4);
  if(test_name == NULL){
    fprintf(stderr, "Error using malloc to create string for test name: %s\n", strerror(errno));
    exit(1);
  }
  test_name[0] = 0;
  test_name = strcat(test_name, "add");
  if(opt_yield){
    test_name = realloc(test_name, strlen(test_name) + 7);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    test_name = strcat(test_name, "-yield");
  }
  if(opt_sync == 1){
    test_name = realloc(test_name, strlen(test_name) + 3);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    test_name = strcat(test_name, "-m");
  }
  else if(opt_sync == 2){
    test_name = realloc(test_name, strlen(test_name) + 3);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    test_name = strcat(test_name, "-s");
  }
  else if(opt_sync == 3){
    test_name = realloc(test_name, strlen(test_name) + 3);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    test_name = strcat(test_name, "-c");
  }
  else{
    test_name = realloc(test_name, strlen(test_name) + 6);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    test_name = strcat(test_name, "-none");
  }
  long total_ops = threads * iterations * 2;
  long long secston = (end_time.tv_sec - start_time.tv_sec) * 1000000000;
  long long nsec_adj = end_time.tv_nsec + secston;
  long long run_time = nsec_adj - start_time.tv_nsec;
  long ave_time = run_time / total_ops;
  printf("%s,%i,%i,%i,%i,%i,%i\n", test_name, threads, iterations, total_ops, run_time, ave_time, counter);
  free(test_name);
  exit(0);
}
