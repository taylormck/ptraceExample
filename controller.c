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
    execvp(argv[1], &argv[1]);
  }
  //------------------------------------------------------------
  // Parent process
  //------------------------------------------------------------
  int wStatus = 0;
  long call, rc, newRC = 0;
  struct user_regs_struct uregs;
  waitpid(pid, &wStatus, 0);
  ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);
  ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
  
  // Wait for the child process to stop
  while(1) {
    waitpid(pid, &wStatus, 0);

    // Check if the child process has finished
    if(WIFEXITED(wStatus)){
      printf("child exited\n");
      return 0;
    }

    // Check to make sure the child process has been stopped
    else if(WIFSTOPPED(wStatus)){
      // Stopped by our ptrace call
      if(WSTOPSIG(wStatus) == (SIGTRAP | 0x80)){
        ptrace(PTRACE_GETREGS, pid, NULL, &uregs);
        call = uregs.orig_rax;
        rc = uregs.rax;
        printf("syscall:  %ld rc=%ld --> %ld\n", call, rc, newRC);
        if(call == 20){
          uregs.rax = newRC;
          newRC = 0;
          ptrace(PTRACE_SETREGS, pid, NULL, &uregs);
        }
        ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
      }

      // Stopped for some other reason
      else{
        printf("child stopped but not for system call");
      }
    }

    // Just in case waitpid() returns
    // If the child has already resumed running, this will
    // have no effect
    else
      ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
  }
  
  return 0;
}
