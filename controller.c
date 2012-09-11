//------------------------------------------------------------
// Controller source file
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <linux/user.h>

int main(int argc, char** argv){
  // pid_t for using fork
  pid_t pid;
  long ret;

  // Assert that the arguments are correctly passed
  if(argc <= 0){
    printf("Incorrect usage.  Must provide a program to trace.\n");
    return 0;
  }

  // Fork
  if((pid = vfork()) < 0){
    perror("vfork()");
    ret = ptrace(PTRACE_TRACEME, 0, NULL, NULL);
    exit(EXIT_FAILURE);
  }

  // Child process
  if(pid == 0) {
    printf("Executing child process.\n");
    //ptrace(PTRACE_TRACEME);
    execvp(argv[1], &argv[2]);
  }

  // Parent process
  int wStatus = 0;
  waitpid(pid, &wStatus, 0);
  orig_eax = ptrace(PTRACE_PEEKUSER, child, 4* ORIG_EAX, NULL);
  printf("syscall:  %d rc=%d", orig_eax, wStatus);
  
  printf("Child finished processing.\n");

  return 0;
}
