/* NAME: Jeffrey Xu
 * EMAIL: jeffreyhxu@gmail.com
 * ID: 404768745
 */

#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <stdio.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <signal.h>

void catcher(int sig){
  fprintf(stderr, "Error from argument --segfault: Segfault successfully created\n");
  exit(4);
}

void faulter(){
  char *c = NULL;
  c[0] = 0;
}

int main(int argc, char **argv){
  int seg_flag = 0, cat_flag = 0;
  struct option lopts[5] =
    {
      {"input", required_argument, NULL, 1},
      {"output", required_argument, NULL, 2},
      {"segfault", no_argument, &seg_flag, 1},
      {"catch", no_argument, &cat_flag, 1},
      {0, 0, 0, 0}
    };
  int opti;
  char *in = NULL, *out = NULL;
  for(;;){
    int status = getopt_long(argc, argv, "", lopts, &opti);
    if(status == -1)
      break;
    else if(status == 1)
      in = optarg;
    else if(status == 2)
      out = optarg;
    else if(status != 0){
      fprintf(stderr, "Usage: lab0 [--input=filename] [--output=filename] [--segfault] [--catch]\n");
      exit(1);
    }
  }
  if(in != NULL){
    int fdin = open(in, O_RDONLY);
    if(fdin == -1){
      fprintf(stderr, "Error with argument --input: Could not open file %s: %s\n", in, strerror(errno));
      exit(2);
    }
    close(0);
    dup(fdin);
    close(fdin);
  }
  if(out != NULL){
    int fdout = creat(out, 0666);
    if(fdout == -1){
      fprintf(stderr, "Error with argument --output: Could not create file %s: %s\n", out, strerror(errno));
      exit(3);
    }
    close(1);
    dup(fdout);
    close(fdout);
  }
  if(cat_flag){
    signal(SIGSEGV, catcher);
  }
  if(seg_flag){
    faulter();
  }
  for(;;){
    char c[1];
    if(!read(0, c, 1))
      break;
    write(1, c, 1);
  }
  exit(0);
}

