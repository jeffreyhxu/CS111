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
#include <signal.h>
#include "SortedList.h"

int opt_yield = 0;
int opt_sync = 0;
#define MUTE_SYNC	1
#define SPIN_SYNC	2

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

struct thread_args{
  SortedListElement_t *elements;
  int ecount;
  SortedList_t *list;
};

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
void *thread_list(void *arg){
  struct thread_args *a = (struct thread_args *)arg;
  for(int i = 0; i < a->ecount; i++){
    if(opt_sync == MUTE_SYNC)
      pthread_mutex_lock(&mutex);
    else if(opt_sync == SPIN_SYNC)
      spinlock();
    SortedList_insert(a->list, &(a->elements[i]));
    if(opt_sync == MUTE_SYNC)
      pthread_mutex_unlock(&mutex);
    else if(opt_sync == SPIN_SYNC)
      spinunlock();
  }
  if(opt_sync == MUTE_SYNC)
    pthread_mutex_lock(&mutex);
  else if(opt_sync == SPIN_SYNC)
    spinlock();
  if(SortedList_length(a->list) == -1){
    fprintf(stderr, "List corruption discovered by thread %i using SortedList_length\n", pthread_self());
    exit(2);
  }
  if(opt_sync == MUTE_SYNC)
    pthread_mutex_unlock(&mutex);
  else if(opt_sync == SPIN_SYNC)
    spinunlock();
  for(int i = 0; i < a->ecount; i++){
    if(opt_sync == MUTE_SYNC)
      pthread_mutex_lock(&mutex);
    else if(opt_sync == SPIN_SYNC)
      spinlock();
    SortedListElement_t *found = SortedList_lookup(a->list, a->elements[i].key);
    if(opt_sync == MUTE_SYNC)
      pthread_mutex_unlock(&mutex);
    else if(opt_sync == SPIN_SYNC)
      spinunlock();
    if(found == NULL){
      fprintf(stderr, "List inconsistency discovered by thread %i using SortedList_lookup to find key %s\n", pthread_self(), a->elements[i].key);
      exit(2);
    }
    if(opt_sync == MUTE_SYNC)
      pthread_mutex_lock(&mutex);
    else if(opt_sync == SPIN_SYNC)
      spinlock();
    if(SortedList_delete(found) == 1){
      fprintf(stderr, "List corruption discovered by thread %i using SortedList_delete to delete element with key %s\n", pthread_self(), found->key);
      exit(2);
    }
    if(opt_sync == MUTE_SYNC)
      pthread_mutex_unlock(&mutex);
    else if(opt_sync == SPIN_SYNC)
      spinunlock();
  }
  pthread_exit(NULL);
}

void segcatcher(int sig){
  fprintf(stderr, "Segmentation fault caught\n");
  exit(2);
}

int main(int argc, char **argv){
  struct option lopts[5] =
    {
      {"threads", required_argument, NULL, 1},
      {"iterations", required_argument, NULL, 2},
      {"yield", required_argument, NULL, 3},
      {"sync", required_argument, NULL, 4},
      {0,0,0,0}
    };
  int opti, threads = 1, iterations = 1;
  char *ystring = "";
  for(;;){
    int opt_stat = getopt_long(argc, argv, "", lopts, &opti);
    if(opt_stat == -1)
      break;
    else if(opt_stat == 1 && optarg != NULL)
      threads = atoi(optarg);
    else if(opt_stat == 2 && optarg != NULL)
      iterations = atoi(optarg);
    else if(opt_stat == 3 && optarg != NULL)
      ystring = optarg;
    else if(opt_stat == 4 && optarg != NULL){
      if(!strcmp(optarg, "m"))
	opt_sync = MUTE_SYNC;
      else if(!strcmp(optarg, "s"))
	opt_sync = SPIN_SYNC;
      else{
	fprintf(stderr, "Usage: lab2_list [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=(m|s)]\n");
	exit(1);
      }
    }
    else{
      fprintf(stderr, "Usage: lab2_list [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=(m|s)]\n");
      exit(1);
    }
  }
  for(int i = 0; i < strlen(ystring); i++){
    switch(ystring[i]){
    case 'i':
      opt_yield |= INSERT_YIELD;
      break;
    case 'd':
      opt_yield |= DELETE_YIELD;
      break;
    case 'l':
      opt_yield |= LOOKUP_YIELD;
      break;
    default:
      fprintf(stderr, "Usage: lab2_list [--threads=#] [--iterations=#] [--yield=[idl]] [--sync=(m|s)]\n");
      exit(1);
      break;
    }
  }
  SortedList_t list;
  list.prev = NULL;
  list.next = NULL;
  list.key = NULL;
  SortedListElement_t *elements = calloc(threads * iterations, sizeof(SortedListElement_t));
  if(elements == NULL){
    fprintf(stderr, "Error using calloc to create array of list elements: %s\n", strerror(errno));
    exit(1);
  }
  for(int i = 0; i < threads * iterations; i++){
    int randkey = rand();
    elements[i].key = malloc(33);
    if(sprintf((char *)elements[i].key, "%i", randkey) < 0){
      fprintf(stderr, "Error using sprintf to write key to list element: %s\n", strerror(errno));
      exit(1);
    }
  }
  struct thread_args *args = calloc(threads, sizeof(struct thread_args));
  if(args == NULL){
    fprintf(stderr, "Error using calloc to create array of thread arguments: %s\n", strerror(errno));
    exit(1);
  }
  for(int i = 0; i < threads; i++){
    args[i].elements = elements + (i * iterations);
    args[i].ecount = iterations;
    args[i].list = &list;
  }
  signal(SIGSEGV, segcatcher);
  struct timespec start_time;
  if(clock_gettime(CLOCK_MONOTONIC, &start_time) == -1){
    fprintf(stderr, "Error using clock_gettime to get monotonic start time: %s\n", strerror(errno));
    exit(1);
  }
  pthread_t *thread_ids = malloc(sizeof(pthread_t) * threads);
  if(thread_ids == NULL){
    fprintf(stderr, "Error using malloc to create array of thread ids: %s\n", strerror(errno));
    exit(1);
  }
  for(int i = 0; i < threads; i++){
    int t_status = pthread_create(thread_ids + i, NULL, thread_list, args + i);
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
  free(args);
  free(elements);
  int end_length = SortedList_length(&list);
  if(end_length == -1){
    fprintf(stderr, "List corruption discovered by main thread using SortedList_length\n");
    exit(2);
  }
  else if(end_length != 0){
    fprintf(stderr, "List inconsistency discovered by main thread using SortedList_length\n");
    exit(2);
  }
  char *test_name = malloc(6);
  if(test_name == NULL){
    fprintf(stderr, "Error using malloc to create string for test name: %s\n", strerror(errno));
    exit(1);
  }
  test_name[0] = 0;
  strcat(test_name, "list-");
  if(opt_yield & INSERT_YIELD){
    test_name = realloc(test_name, strlen(test_name) + 2);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "i");
  }
  if(opt_yield & DELETE_YIELD){
    test_name = realloc(test_name, strlen(test_name) + 2);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "d");
  }
  if(opt_yield & LOOKUP_YIELD){
    test_name = realloc(test_name, strlen(test_name) + 2);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "l");
  }
  if(strlen(test_name) == 5){
    test_name = realloc(test_name, strlen(test_name) + 5);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "none");
  }
  //sync options
  if(opt_sync == MUTE_SYNC){
    test_name = realloc(test_name, strlen(test_name) + 3);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "-m");
  }
  else if(opt_sync == SPIN_SYNC){
    test_name = realloc(test_name, strlen(test_name) + 3);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "-s");
  }
  else{
    test_name = realloc(test_name, strlen(test_name) + 6);
    if(test_name == NULL){
      fprintf(stderr, "Error using realloc to expand string for test name: %s\n", strerror(errno));
      exit(1);
    }
    strcat(test_name, "-none");
  }
  int lists = 1;
  int total_ops = threads * iterations * 3;
  long long secston = (end_time.tv_sec - start_time.tv_sec) * 1000000000;
  long long nsec_adj = end_time.tv_nsec + secston;
  long long run_time = nsec_adj - start_time.tv_nsec;
  long long ave_time = run_time / total_ops;
  //fprintf(stderr, "Start: sec: %i, nsec: %i\nEnd: sec: %i, nsec: %i\nsecston: %lli, nsec_adg: %lli\n", start_time.tv_sec, start_time.tv_nsec, end_time.tv_sec, end_time.tv_nsec, secston, nsec_adj);
  printf("%s,%i,%i,%i,%i,%lli,%lli\n", test_name, threads, iterations, lists, total_ops, run_time, ave_time);
  free(test_name);
  exit(0);
}
