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

int period = 1, sample = 0;
void ringer(int sig){
  sample = 1;
  alarm(period);
}

int running = 1;
void interrupter(int sig){
  running = 0;
}

int main(int argc, char **argv){
  struct option lopts[4] =
    {
      {"period", required_argument, NULL, 1},
      {"scale", required_argument, NULL, 2},
      {"log", required_argument, NULL, 3},
      {0,0,0,0}
    };
  int opti, scale = SCALE_F;
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
  
  signal(SIGALRM, ringer);
  ringer(SIGALRM);

  signal(SIGINT, interrupter);
  
  while(running){
    if(sample){
      int rawtemp = mraa_aio_read(adc_a0);
      if(rawtemp == -1){
	fprintf(stderr, "Error using mraa_aio_read to get temperature reading\n");
	exit(1);
      }
      double R = 1023.0 / rawtemp - 1.0;
      double temperature = 1.0 / (log(R) / B + 1 / 298.15) - 273.15;
      if(scale == SCALE_F)
	temperature = temperature * 1.8 + 32;
      struct timespec now;
      if(clock_gettime(CLOCK_REALTIME, &now) == -1){
	fprintf(stderr, "Error using clock_gettime to get time: %s\n", strerror(errno));
	exit(1);
      }
      printf("%s %.1f\n", ctime(&(now.tv_sec)), temperature);
      if(logfile != NULL)
	fprintf(logfile, "%s %.1f\n", ctime(&(now.tv_sec)), temperature);
      sample = 0;
    }

    int button = mraa_gpio_read(gpio_d3);
    if(button == -1){
      fprintf(stderr, "Error using mraa_gpio_read to get button reading\n");
      exit(1);
    }
    else if(button == 1){
      struct timespec now;
      if(clock_gettime(CLOCK_REALTIME, &now) == -1){
	fprintf(stderr, "Error using clock_gettime to get time: %s\n", strerror(errno));
	exit(1);
      }
      printf("%s SHUTDOWN", ctime(&(now.tv_sec)));
      if(logfile != NULL)
	fprintf(logfile, "%s SHUTDOWN", ctime(&(now.tv_sec)));
      break;
    }
  }
  if(logfile != NULL)
    fclose(logfile);
  exit(0);
}
