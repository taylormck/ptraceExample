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

#ifdef __x86_64__
#define SC_NUMBER  (8 * ORIG_RAX)
#define SC_RETCODE (8 * RAX)
#else
#define SC_NUMBER  (4 * ORIG_EAX)
#define SC_RETCODE (4 * EAX)
#endif

int main(int argc, char** argv){
  // pid_t for using fork
  pid_t pid;

  // Assert that the arguments are correctly passed
  if(argc <= 0){
    printf("Incorrect usage.  Must provide a program to trace.\n");
    return 0;
  }

  // Fork
  if((pid = fork()) < 0){
    perror("fork()");
    exit(EXIT_FAILURE);
  }

  // Child process
  if(pid == 0) {
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP);
    execvp(argv[1], &argv[1]);
  }
  //------------------------------------------------------------
  // Parent process
  //------------------------------------------------------------
  int wStatus = 0;
  long call, rc, newRC = 0;
  struct user_regs_struct uregs;
  /*
  wait(&wStatus);
  ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);
  ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
  */
  // Wait for the child process to stop
  while(1) {
    wait(&wStatus);

    // Check if the child process has finished
    if(WIFEXITED(wStatus)){
      printf("child exited\n");
      return 0;
    }

    // Check to make sure the child process has been stopped
    if(!WIFSTOPPED(wStatus)){
      perror("wait(&wStatus)");
      exit(0);
    }
    // Stopped by our ptrace call
    if(WSTOPSIG(wStatus) == (SIGTRAP/* | 0x80*/)){
      ptrace(PTRACE_GETREGS, pid, NULL, &uregs);
      call = uregs.orig_rax;
      rc = uregs.rax;
      if(rc != 32){
        printf("syscall:  %ld rc=%ld --> %ld\n", call, rc, newRC);
        if(call == 20){
          uregs.rax = newRC;
          newRC = 0;
          ptrace(PTRACE_SETREGS, pid, NULL, &uregs);
        }
      }
    }

    // Stopped for some other reason
    else{
      printf("child stopped but not for system call.\n");
    }
    fflush(stdout);
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
  }
  return 0;
}
