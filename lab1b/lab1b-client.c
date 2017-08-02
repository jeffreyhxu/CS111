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
#include <mcrypt.h>
#include <sys/stat.h>
#include <fcntl.h>

int shutdown_flag = 0;
struct termios old_term;
FILE *logfile;
MCRYPT encrypt, decrypt;

void flagger(int sig){
  shutdown_flag = 1;
  //fprintf(stderr, "SIGPIPE flagger caused shutdown\r\n");
}

void term_restore(){
  tcsetattr(0, TCSAFLUSH, &old_term);
}

void log_closer(){
  fclose(logfile);
}

void crypt_closer(){
  mcrypt_generic_deinit(encrypt);
  mcrypt_module_close(encrypt);
  mcrypt_generic_deinit(decrypt);
  mcrypt_module_close(decrypt);
}

int main(int argc, char **argv){
  int port_flag = 0;
  int log_flag = 0;
  struct option lopts[5] =
    {
      {"port", required_argument, &port_flag, 1},
      {"log", required_argument, &log_flag, 1},
      {"encrypt", required_argument, NULL, 2},
      {"host", required_argument, NULL, 1},
      {0, 0, 0, 0}
    };
  int opti;
  int port;
  char *log;
  char *keyfile = NULL;
  char *host = "lnxsrv09.seas.ucla.edu";
  for(;;){
    int opt_stat = getopt_long(argc, argv, "", lopts, &opti);
    if(opt_stat == -1)
      break;
    else if(opt_stat == 1)
      host = optarg;
    else if(opt_stat == 2)
      keyfile = optarg;
    else if(opt_stat != 0){
      fprintf(stderr, "Usage: lab1a --port=port# [--log=filename] [--encrypt=filename] [--host=name]\r\n");
      exit(1);
    }
    else if(opti == 0 && optarg != NULL)
      port = atoi(optarg);
    else if(opti == 1)
      log = optarg;
  }
  if(!port_flag){
    fprintf(stderr, "Usage: lab1a --port=port# [--log=filename] [--encrypt=filename] [--host=name]\r\n");
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
  //fprintf(stderr, "Name: %s, Address: %s\r\n", server->h_name, server->h_addr);
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  strcpy((char *)&serv_addr.sin_addr.s_addr, (char *)server->h_addr);
  //fprintf(stderr, "Address: %i\r\n", serv_addr.sin_addr.s_addr);
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

  if(log_flag){
    logfile = fopen(log, "w");
    if(logfile == NULL){
      fprintf(stderr, "Error using fopen to open log file: %s\r\n", strerror(errno));
      exit(1);
    }
    atexit(log_closer);
  }

  if(keyfile != NULL){
    encrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    if(encrypt == MCRYPT_FAILED){
      fprintf(stderr, "Error using mcrypt_module_open to open encryption session");
      exit(1);
    }
    int keysize = mcrypt_enc_get_key_size(encrypt);
    char *key = malloc(keysize);
    int keyfd = open(keyfile, O_RDONLY);
    if(keyfd == -1){
      fprintf(stderr, "Error using open to open key file: %s\r\n", strerror(errno));
      exit(1);
    }
    keysize = read(keyfd, key, keysize);
    if(keysize == -1){
      fprintf(stderr, "Error using read to read key from file: %s\r\n", strerror(errno));
      exit(1);
    }
    if(close(keyfd) == -1){
      fprintf(stderr, "Error using close to close key file: %s\r\n", strerror(errno));
      exit(1);
    }
    //printf("Key: %s, Size: %i, sizeof: %i, Max: %i\r\n", key, keysize, sizeof(key), mcrypt_enc_get_key_size(encrypt));
    char *eniv = malloc(mcrypt_enc_get_iv_size(encrypt));
    memset(eniv, 0, sizeof(char) * mcrypt_enc_get_iv_size(encrypt));
    int en_stat = mcrypt_generic_init(encrypt, (void *)key, sizeof(key), (void *)eniv);
    if(en_stat < 0){
      fprintf(stderr, "Error using mcrypt_generic_init to initialize encryption session: %s/r/n", mcrypt_strerror(en_stat));
      exit(1);
    }
    decrypt = mcrypt_module_open("twofish", NULL, "cfb", NULL);
    if(decrypt == MCRYPT_FAILED){
      fprintf(stderr, "Error using mcrypt_module_open to open decryption session");
      exit(1);
    }
    char *deiv = malloc(mcrypt_enc_get_iv_size(decrypt));
    memset(deiv, 0, sizeof(char) * mcrypt_enc_get_iv_size(decrypt));
    int de_stat = mcrypt_generic_init(decrypt, (void *)key, sizeof(key), (void *)deiv);
    if(de_stat < 0){
      fprintf(stderr, "Error using mcrypt_generic_init to initialize decryption session: %s/r/n", mcrypt_strerror(de_stat));
      exit(1);
    }
    
    atexit(crypt_closer);
  }
  
  int in_closed = 0;
  for(;;){
    if(poll(pollfds, 2, 0) == -1){
      fprintf(stderr, "Error using poll to get input: %s\r\n", strerror(errno));
      exit(1);
    }
    if(pollfds[0].revents & POLLIN){
      char buf[256];
      int bytes = read(0, buf, 255);
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
      if(keyfile != NULL){
	if(mcrypt_generic(encrypt, buf, bytes) != 0){
	  fprintf(stderr, "Error using mcrypt_generic to encrypt input\r\n");
	  exit(1);
	}
      }
      if(write(sockfd, buf, bytes) == -1 && !shutdown_flag){
	fprintf(stderr, "Error using write to write to socket: %s\r\n", strerror(errno));
	exit(1);
      }
      if(log_flag && !shutdown_flag){
	buf[bytes] = 0;
	if(fprintf(logfile, "SENT %i bytes: %s\n", bytes, buf) < 0){
	  fprintf(stderr, "Error using fprintf to write sent bytes to log file: %s\r\n", strerror(errno));
	  exit(1);
	}
      }
    }
    if(pollfds[1].revents & POLLIN){
      char buf[256];
      int bytes = read(sockfd, buf, 255);
      if(bytes == -1){
	fprintf(stderr, "Error using read to read from socket: %s\r\n", strerror(errno));
	exit(1);
      }
      if(log_flag && bytes > 0){
	buf[bytes] = 0;
	if(fprintf(logfile, "RECEIVED %i bytes: %s\n", bytes, buf) < 0){
	  fprintf(stderr, "Error using fprintf to write received bytes to log file: %s\r\n", strerror(errno));
	  exit(1);
	}
      }
      if(keyfile != NULL){
	if(mdecrypt_generic(decrypt, buf, bytes) != 0){
	  fprintf(stderr, "Error using mdecrypt_generic to decrypt output\r\n");
	  exit(1);
	}
      }
      char ech_buf[512]; // in case every character in the buffer is <lf>
      int ech_bytes = 0;
      for(int i = 0; i < bytes; i++){
	if(buf[i] == 0x04){ // end of file
	  ech_buf[ech_bytes] = buf[i];
	  shutdown_flag = 1;
	  //fprintf(stderr, "EOF caused shutdown\r\n");
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
      /*if(pollfds[1].revents & POLLHUP)
	fprintf(stderr, "POLLHUP caused shutdown, flag: %i\r\n", shutdown_flag);*/
      break;
    }
  }

  exit(0);
}
