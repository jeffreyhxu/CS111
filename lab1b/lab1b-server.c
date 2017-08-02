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
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <mcrypt.h>
#include <sys/stat.h>
#include <fcntl.h>

int shutdown_flag = 0;
struct termios old_term;
MCRYPT encrypt, decrypt;

void flagger(int sig){
  shutdown_flag = 1; // The shutdown handling will be done in the poll loop.
}

void term_restore(){
  tcsetattr(0, TCSAFLUSH, &old_term);
}

void crypt_closer(){
  mcrypt_generic_deinit(encrypt);
  mcrypt_module_close(encrypt);
  mcrypt_generic_deinit(decrypt);
  mcrypt_module_close(decrypt);
}

int main(int argc, char **argv){
  int port_flag = 0;
  struct option lopts[3] =
    {
      {"port", required_argument, &port_flag, 1},
      {"encrypt", required_argument, NULL, 1},
      {0, 0, 0, 0}
    };
  int opti;
  int port;
  char *keyfile = NULL;
  for(;;){
    int opt_stat = getopt_long(argc, argv, "", lopts, &opti);
    if(opt_stat == -1)
      break;
    else if(opt_stat == 1)
      keyfile = optarg;
    else if(opt_stat != 0){
      fprintf(stderr, "Usage: lab1a --port=port# [--encrypt=filename]\r\n");
      exit(1);
    }
    else if(opti == 0 && optarg != NULL)
      port = atoi(optarg);
  }
  if(!port_flag){
    fprintf(stderr, "Usage: lab1a --port=port# [--encrypt=filename]\r\n");
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
  if(sockfd < 0){
    fprintf(stderr, "Error using socket to create a socket: %s\r\n", strerror(errno));
    exit(1);
  }
  struct sockaddr_in serv_addr;
  memset(&serv_addr, 0, sizeof(struct sockaddr_in));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_port = htons(port);
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  if(bind(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1){
    fprintf(stderr, "Error using bind to bind socket to address: %s\r\n", strerror(errno));
    exit(1);
  }
  listen(sockfd, 5);
  struct sockaddr_in cli_addr;
  int clen = sizeof(struct sockaddr_in);
  int truefd = accept(sockfd, (struct sockaddr *)&cli_addr, &clen);
  if(truefd == -1){
    fprintf(stderr, "Error using accept to accept connection: %s\r\n", strerror(errno));
    exit(1);
  }
  
  int in_pipe[2];
  if(pipe(in_pipe) == -1){
    fprintf(stderr, "Error using pipe to create a pipe to input: %s\r\n", strerror(errno));
    exit(1);
  }
  int out_pipe[2];
  if(pipe(out_pipe) == -1){
    fprintf(stderr, "Error using pipe to create a pipe from output: %s\r\n", strerror(errno));
    exit(1);
  }
  
  pid_t child = fork();
  if(child == -1){
    fprintf(stderr, "Error using fork to create a new process: %s\r\n", strerror(errno));
    exit(1);
  }
  if(child == 0){
    close(0);
    dup(in_pipe[0]);
    close(in_pipe[0]);

    close(in_pipe[1]);
    
    close(1);
    dup(out_pipe[1]);
    close(2);
    dup(out_pipe[1]);
    close(out_pipe[1]);

    close(out_pipe[0]);
    
    if(execl("/bin/bash", "/bin/bash", (char *)0) == -1){
      fprintf(stderr, "Error using execl to execute /bin/bash: %s\r\n", strerror(errno));
      exit(1);
    }
  }
  close(in_pipe[0]);
  close(out_pipe[1]);

  struct pollfd pollfds[2];
  pollfds[0].fd = truefd;
  pollfds[0].events = POLLIN;
  pollfds[1].fd = out_pipe[0];
  pollfds[1].events = POLLIN;

  signal(SIGPIPE, flagger);

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
      int bytes = read(truefd, buf, 255);
      if(bytes == -1){
	fprintf(stderr, "Error using read to read from socket: %s\r\n", strerror(errno));
	exit(1);
      }
      if(keyfile != NULL){
	if(mdecrypt_generic(decrypt, buf, bytes) != 0){
	  fprintf(stderr, "Error using mdecrypt_generic to decrypt input\r\n");
	  exit(1);
	}
      }
      char shell_buf[256];
      int eof = 0;
      int kill_flag = 0;
      for(int i = 0; i < bytes; i++){
	if(buf[i] == '\r' || buf[i] == '\n'){
	  shell_buf[i] = '\n';
	}
	else if(buf[i] == 0x04){ // end of file
	  eof = 1;
	  shell_buf[i] = ' ';
	}
	else if(buf[i] == 0x03){ // interrupt
	  shell_buf[i] = ' ';
	  kill_flag = 1;
	}
	else{
	  shell_buf[i] = buf[i];
	}
      }
      if(!in_closed && write(in_pipe[1], shell_buf, bytes) == -1 && !shutdown_flag){
	fprintf(stderr, "Error using write to write to pipe to shell input: %s\r\n", strerror(errno));
	exit(1);
      }
      if(eof && !in_closed){
	if(close(in_pipe[1]) == -1){
	  fprintf(stderr, "Error using close to close pipe to shell input: %s\r\n", strerror(errno));
	  exit(1);
	}
	in_closed = 1;
      }
      if(kill_flag){
	kill(child, SIGINT);
      }
    }
    if(pollfds[1].revents & POLLIN){
      char buf[256];
      int bytes = read(out_pipe[0], buf, 255);
      if(bytes == -1){
	fprintf(stderr, "Error using read to read from pipe from shell output: %s\r\n", strerror(errno));
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
      if(keyfile != NULL){
	if(mcrypt_generic(encrypt, ech_buf, ech_bytes) != 0){
	  fprintf(stderr, "Error using mcrypt_generic to encrypt output\r\n");
	  exit(1);
	}
      }
      if(write(truefd, ech_buf, ech_bytes) == -1){
	fprintf(stderr, "Error using write to write to socket: %s\r\n", strerror(errno));
	exit(1);
      }
    }
    if((pollfds[1].revents & POLLHUP) || shutdown_flag){
      if(!in_closed){
	if(close(in_pipe[1]) == -1){
	  fprintf(stderr, "Error using close to close pipe to shell input: %s\r\n", strerror(errno));
	  exit(1);
	}
      }
      int exit_status;
      if(waitpid(child, &exit_status, 0) == -1){
	fprintf(stderr, "Error using waitpid to get exit status of shell: %s\r\n", strerror(errno));
	exit(1);
      }
      fprintf(stderr, "SHELL EXIT SIGNAL=%d STATUS=%d\r\n", WTERMSIG(exit_status), WEXITSTATUS(exit_status));
      break;
    }
  }
  
  exit(0);
}
