//------------------------------------------------------------
// Controller source file
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>


int main(int argc, char** argv){
  // pid_t for using fork
  pid_t pid;

  // Assert that the arguments are correctly passed
  if(argc <= 0){
    printf("Incorrect usage.  Must provide a program to trace.\n");
    return 0;
  }

  // Fork
  if((pid = vfork()) < 0){
    perror("vfork()");
    exit(EXIT_FAILURE);
  }

  // Child process
  if(pid == 0){
    execvp(argv[1], &argv[2]);
  }

  // Parent process
  else {
    printf("Got to end of controller\n");
    wait(0);
    printf("Child finished processing.\n");
  }

  return 0;
}
