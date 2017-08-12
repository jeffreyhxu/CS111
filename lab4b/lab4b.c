/* Name: Jeffrey Xu
 * Email: jeffreyhxu@gmail.com
 * ID: 404768745
 */
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <math.h>
#include <time.h>
#include <fcntl.h>
#include <getopt.h>

#ifndef DUMMY
#include <mraa/aio.h>
#include <mraa/gpio.h>
#endif
#ifdef DUMMY
typedef int * mraa_aio_context;
typedef int * mraa_gpio_context;
mraa_aio_context mraa_aio_init(int a){
  return malloc(1);
}
mraa_gpio_context mraa_gpio_init(int a){
  return malloc(1);
}
int mraa_aio_read(mraa_aio_context a){
  return 512;
}
int mraa_gpio_read(mraa_aio_context a){
  return 0;
}
#endif

#define SCALE_C 0
#define SCALE_F 1
const int B = 4275;

int running = 1;
void interrupter(int sig){
  running = 0;
}

char *formatted_time(time_t now){
  struct tm *localnow = localtime(&now);
  if(localnow == NULL){
    fprintf(stderr, "Error using localtime to get local time: %s\n", strerror(errno));
    exit(1);
  }
  char *result = malloc(10);
  if(!strftime(result, 10, "%T", localnow)){
    fprintf(stderr, "Error using strftime to format time: %s\n", strerror(errno));
    exit(1);
  }
  return result;
}

int main(int argc, char **argv){
  struct option lopts[4] =
    {
      {"period", required_argument, NULL, 1},
      {"scale", required_argument, NULL, 2},
      {"log", required_argument, NULL, 3},
      {0,0,0,0}
    };
  int opti, scale = SCALE_F, period = 1;
  FILE *logfile = NULL;
  for(;;){
    int opt_stat = getopt_long(argc, argv, "", lopts, &opti);
    if(opt_stat == -1)
      break;
    else if(opt_stat == 1 && optarg != NULL){
      period = atoi(optarg);
      if(period < 1){
	fprintf(stderr, "Usage: lab4b [--period=#] [--scale=(C|F)] [--log=filename]\n");
	exit(1);
      }
    }
    else if(opt_stat == 2 && optarg != NULL){
      if(!strcmp(optarg, "C"))
	scale = SCALE_C;
      else if (!strcmp(optarg, "F"))
	scale = SCALE_F;
      else{
	fprintf(stderr, "Usage: lab4b [--period=#] [--scale=(C|F)] [--log=filename]\n");
	exit(1);
      }
    }
    else if(opt_stat == 3 && optarg != NULL){
      logfile = fopen(optarg, "w");
      if(logfile == NULL){
	fprintf(stderr, "Error using fopen to open logfile %s: %s\n", optarg, strerror(errno));
	exit(1);
      }
    }
    else{
      fprintf(stderr, "Usage: lab4b [--period=#] [--scale=(C|F)] [--log=filename]\n");
      exit(1);
    }
  }  

  mraa_aio_context adc_a0 = mraa_aio_init(0);
  if(adc_a0 == NULL){
    fprintf(stderr, "Error using mraa_aio_init to initialize context for temperature reading\n");
    exit(1);
  }

  mraa_gpio_context gpio_d3 = mraa_gpio_init(3);
  if(gpio_d3 == NULL){
    fprintf(stderr, "Error using mraa_gpio_init to initialize context for button reading\n");
    exit(1);
  }

  if(fcntl(0, F_SETFL, O_NONBLOCK) == -1){
    fprintf(stderr, "Error using fcntl to make stdin non-blocking: %s\n", strerror(errno));
    exit(1);
  }
  char *combuf = malloc(32);
  memset(combuf, 0, 32);
  int bufsize = 0;

  signal(SIGINT, interrupter);

  int unstopped = 1;
  int off = 0;
  time_t prev = 0;
  while(running){
    time_t now = time(NULL);
    if(now == (time_t) -1){
      fprintf(stderr, "Error using time to get time: %s\n", strerror(errno));
      exit(1);
    }
    if(now - prev >= period && unstopped){
      prev = now;
      int rawtemp = mraa_aio_read(adc_a0);
      if(rawtemp == -1){
	fprintf(stderr, "Error using mraa_aio_read to get temperature reading\n");
	exit(1);
      }
      double R = 1023.0 / rawtemp - 1.0;
      double temperature = 1.0 / (log(R) / B + 1 / 298.15) - 273.15;
      if(scale == SCALE_F)
	temperature = temperature * 1.8 + 32;
      char *time = formatted_time(now);
      printf("%s %.1f\n", time, temperature);
      if(logfile != NULL)
	fprintf(logfile, "%s %.1f\n", time, temperature);
      free(time);
    }

    int rsize = read(0, combuf + bufsize, 31 - bufsize);
    if(rsize == -1 && errno != EAGAIN){
      fprintf(stderr, "Error using read to get command from terminal: %s\n", strerror(errno));
      exit(1);
    }
    if(rsize > 0 || bufsize > 0){
      bufsize += rsize;
      //fprintf(stderr, "rsize: %i\ncombuf befor: %s\nbufsize: %i\n", rsize, combuf, bufsize);
      for(int i = 0; i < bufsize; i++){
	if(combuf[i] == '\n'){
	  combuf[i] = '\0';
	  if(!strncmp(combuf, "SCALE=", 6) && strlen(combuf) == 7){
	    if(combuf[6] == 'C'){
	      scale = SCALE_C;
	      if(logfile != NULL)
		fprintf(logfile, "SCALE=C\n");
	    }
	    else if(combuf[6] == 'F'){
	      scale = SCALE_F;
	      if(logfile != NULL)
		fprintf(logfile, "SCALE=F\n");
	    }
	    //fprintf(stderr, "new scale: %c\n", combuf[6]);
	  }
	  else if(!strncmp(combuf, "PERIOD=", 7)){
	    if(atoi(combuf + 7) > 0){
	      period = atoi(combuf + 7);
	      if(logfile != NULL)
		fprintf(logfile, "PERIOD=%d\n", atoi(combuf+7));
	    }
	    //fprintf(stderr, "new period: %i\n", atoi(combuf + 7));
	  }
	  else if(!strcmp(combuf, "STOP")){
	    unstopped = 0;
	    if(logfile != NULL)
	      fprintf(logfile, "STOP\n");
	  }
	  else if(!strcmp(combuf, "START")){
	    unstopped = 1;
	    if(logfile != NULL)
	      fprintf(logfile, "START\n");
	  }
	  else if(!strcmp(combuf, "OFF")){
	    off = 1;
	    if(logfile != NULL)
	      fprintf(logfile, "OFF\n");
	  }
	  char *newbuf = malloc(32);
	  memset(newbuf, 0, 32);
	  memcpy(newbuf, combuf + i + 1, 31 - i);
	  free(combuf);
	  combuf = newbuf;
	  bufsize = bufsize - i - 1;
	  break;
	}
      }
      //fprintf(stderr, "combuf after: %s\nbufsize: %i\n", combuf, bufsize);
      if(bufsize >= 31){
	memset(combuf, 0, 32);
	bufsize = 0;
      }
    }
      
    int button = mraa_gpio_read(gpio_d3);
    if(button == -1){
      fprintf(stderr, "Error using mraa_gpio_read to get button reading\n");
      exit(1);
    }
    else if(button || off){
      char *time = formatted_time(now);
      printf("%s SHUTDOWN\n", time);
      if(logfile != NULL)
	fprintf(logfile, "%s SHUTDOWN\n", time);
      free(time);
      running = 0;
    }
  }
  if(logfile != NULL)
    fclose(logfile);
  exit(0);
}
