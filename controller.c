//------------------------------------------------------------
// Controller source file
//

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <signal.h>
#include <sys/user.h>

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
  if(pid == 0) {
    ptrace(PTRACE_TRACEME);
    execvp(argv[1], &argv[2]);
  }
  //------------------------------------------------------------
  // Parent process
  //------------------------------------------------------------
  siginfo_t sig;
  int wStatus = 0;
  waitpid(pid, &wStatus, 0);
  ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);
  
  // Wait for the child process to stop
  while(1){

    // Check if the child process has finished
    if(WIFEXITED(wStatus)){
      printf("child exited\n");
      break;
    }

    // Check to make sure the child process has been stopped by us
    else if(WIFSTOPPED(wStatus) && WSTOPSIG(wStatus) == SIGTRAP){
      ptrace(PTRACE_GETSIGINFO, pid, NULL, &sig);
      printf("syscall:  %d rc=%d\n", sig.si_signo, wStatus);
      ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }
    else {
      ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
    }

    waitpid(pid, &wStatus, 0);
  }
  
  return 0;
}
