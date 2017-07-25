#include <unistd.h>
#include <stdio.h>

int main(int argc, char **argv){
  pid_t child = fork();
  if(child == 0){
    if(execl("/bin/bash", "/bin/bash", (char *)0) == -1)
      printf("Nope");
  }
}
