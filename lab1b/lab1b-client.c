/* NAME: Jeffrey Xu
 * EMAIL: jeffreyhxu@gmail.com
 * ID: 404768745
 */
#include <unistd.h>
#include <stdlib.h>
#include <getopt.h>
#include <sys/wait.h>
#include <termios.h>
#include <signal.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <netdb.h>

int shutdown_flag = 0;
struct termios old_term;

void flagger(int sig){
  shutdown_flag = 1;
}

void term_restore(){
  tcsetattr(0, TCSAFLUSH, &old_term);
}

int main(int argc, char **argv){
  int port_flag = 0;
  int log_flag = 0;
  struct option lopts[4] =
    {
      {"port", required_argument, &port_flag, 1},
      {"log", required_argument, &log_flag, 1},
      {"host", required_argument, NULL, 1},
      {0, 0, 0, 0}
    };
  int opti;
  int port;
  char *log;
  char *host = "lnxsrv09.seas.ucla.edu";
  gethostname(host, 255);
  for(;;){
    int opt_stat = getopt_long(argc, argv, "", lopts, &opti);
    if(opt_stat == -1)
      break;
    else if(opt_stat == 1)
      host = optarg;
    else if(!strcmp(lopts[opti].name, "port"))
      port = atoi(optarg);
    else if(!strcmp(lopts[opti].name, "log"))
      log = optarg;
    else if(opt_stat != 0){
      fprintf(stderr, "Usage: lab1a --port=port# [--log=filename]\r\n");
      exit(1);
    }
  }
  if(!port_flag){
    fprintf(stderr, "Usage: lab1a --port=port# [--log=filename]\r\n");
    exit(1);
  }
  
  tcgetattr(0, &old_term);
  struct termios new_term = old_term;
  new_term.c_iflag = ISTRIP;
  new_term.c_oflag = 0;
  new_term.c_lflag = 0;
  if(tcsetattr(0, TCSAFLUSH, &new_term) == -1){
    fprintf(stderr, "Error using tcsetattr to set character-at-a-time, no-echo mode: %s\r\n", strerror(errno));
    exit(1);
  }
  atexit(term_restore);

  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if(sockfd == -1){
    fprintf(stderr, "Error using socket to create a socket: %s\r\n", strerror(errno));
    exit(1);
  }
  struct hostent *server = gethostbyname(host);
  if(server == NULL){
    fprintf(stderr, "Error with provided hostname: %s\r\n", hstrerror(h_errno));
    exit(1);
  }
  fprintf(stderr, "Name: %s, Address: %s\r\n", server->h_name, server->h_addr);
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  strcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr);
  fprintf(stderr, "Address: %i\r\n", serv_addr.sin_addr.s_addr);
  if(connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
    fprintf(stderr, "Error using connect to connect to server: %s\r\n", strerror(errno));
    exit(1);
  }
  
  struct pollfd pollfds[2];
  pollfds[0].fd = 0;
  pollfds[0].events = POLLIN;
  pollfds[1].fd = sockfd;
  pollfds[1].events = POLLIN;

  signal(SIGPIPE, flagger);
  
  int in_closed = 0;
  for(;;){
    if(poll(pollfds, 2, 0) == -1){
      fprintf(stderr, "Error using poll to get input: %s\r\n", strerror(errno));
      exit(1);
    }
    if(pollfds[0].revents & POLLIN){
      char buf[256];
      int bytes = read(0, buf, 256);
      if(bytes == -1){
	fprintf(stderr, "Error using read to read from keyboard: %s\r\n", strerror(errno));
	exit(1);
      }
      char ech_buf[512]; // in case every character in the buffer is <lf>
      int ech_bytes = 0;
      for(int i = 0; i < bytes; i++){
	if(buf[i] == '\r' || buf[i] == '\n'){
	  ech_buf[ech_bytes] = '\r';
	  ech_bytes++;
	  ech_buf[ech_bytes] = '\n';
	}
	else if(buf[i] == 0x04){ // end of file
	  ech_buf[ech_bytes] = '^';
	  ech_bytes++;
	  ech_buf[ech_bytes] = 'D';
	}
	else if(buf[i] == 0x03){ // interrupt
	  ech_buf[ech_bytes] = '^';
	  ech_bytes++;
	  ech_buf[ech_bytes] = 'C';
	}
	else{
	  ech_buf[ech_bytes] = buf[i];
	}
	ech_bytes++;
      }
      if(write(1, ech_buf, ech_bytes) == -1){
	fprintf(stderr, "Error using write to write to display: %s\r\n", strerror(errno));
	exit(1);
      }
      if(write(sockfd, buf, bytes) == -1 && !shutdown_flag){
	fprintf(stderr, "Error using write to write socket: %s\r\n", strerror(errno));
	exit(1);
      }
    }
    if(pollfds[1].revents & POLLIN){
      char buf[256];
      int bytes = read(sockfd, buf, 256);
      if(bytes == -1){
	fprintf(stderr, "Error using read to read from socket: %s\r\n", strerror(errno));
	exit(1);
      }
      char ech_buf[512]; // in case every character in the buffer is <lf>
      int ech_bytes = 0;
      for(int i = 0; i < bytes; i++){
	if(buf[i] == '\r' || buf[i] == '\n'){
	  ech_buf[ech_bytes] = '\r';
	  ech_bytes++;
	  ech_buf[ech_bytes] = '\n';
	}
	else if(buf[i] == 0x04){ // end of file
	  ech_buf[ech_bytes] = buf[i];
	  shutdown_flag = 1;
	}
	else{
	  ech_buf[ech_bytes] = buf[i];
	}
	ech_bytes++;
      }
      if(write(1, ech_buf, ech_bytes) == -1){
	fprintf(stderr, "Error using write to write to display: %s\r\n", strerror(errno));
	exit(1);
      }
    }
    if((pollfds[1].revents & POLLHUP) || shutdown_flag){
      break;
    }
  }

  exit(0);
}
