//------------------------------------------------------------
// Controller source file
//------------------------------------------------------------
// Author: Taylor McKinney
// Date Last Modified: 9/17/2012
//------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ptrace.h>
#include <sys/reg.h>
#include <signal.h>
#include <sys/user.h>

// These defines allow us to compile and run for both 32 and 64 bit
// versions of x86.
// Note that the user_regs_struct has members for both rax and eax,
// but 64 and 32 bit systems require these members respectively.
// 39 is the value stored in rax for getpid calls in 64 bit systems, and
// 20 is the value stored in eax in 32 bit systems
#ifdef __x86_64__
#define ORIG_REG orig_rax
#define REG rax
#define GETPID_NUM 39
#else
#define ORIG_REG orig_eax
#define REG eax
#define GETPID_NUM 20
#endif

int main(int argc, char** argv){
  // pid_t for using fork
  pid_t pid;

  // Assert that the arguments are correctly passed
  if(argc <= 0){
    printf("Incorrect usage.  Must provide a program to trace.\n");
    return 0;
  }

  // Fork and make sure it worked
  if((pid = fork()) < 0){
    perror("fork()");
    exit(EXIT_FAILURE);
  }

  // Check to see if we are the child
  // If so, prepare to be traced, wait for the parent, and execute
  if(pid == 0) {
    ptrace(PTRACE_TRACEME);
    kill(getpid(), SIGSTOP); // Won't be traced
    execvp(argv[1], &argv[1]); // Will be traced
  }
  
  //------------------------------------------------------------
  // Parent process
  //------------------------------------------------------------
  int wStatus = 0;
  long call, rc, newRC = 5;
  struct user_regs_struct uregs;
  
  // Wait for the kill statement to execute
  wait(&wStatus); 

  // Now set our options
  ptrace(PTRACE_SETOPTIONS, pid, NULL, PTRACE_O_TRACESYSGOOD);
  ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
  
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
    if(WSTOPSIG(wStatus) == (SIGTRAP | 0x80)){
      // We are now entering a system call
      ptrace(PTRACE_GETREGS, pid, NULL, &uregs);
      call = uregs.ORIG_REG;
      printf("syscall:  %4ld ", call);

      // Wait until we're exiting the system call
      ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
      wait(&wStatus);
      // If wait fails here, the computer has probably exploded
      // or whatever.

      ptrace(PTRACE_GETREGS, pid, NULL, &uregs);
      rc = uregs.REG;
      printf("rc = %ld", rc);

      // Modify the return value of the getpid system call
      if(call == GETPID_NUM){
	uregs.REG = newRC;
	ptrace(PTRACE_SETREGS, pid, NULL, &uregs);
	printf(" -> %ld\n", newRC);
	newRC--;
      }
      else
	printf("\n");
    } 

    // Stopped for some other reason
    else{
      printf("child stopped but not for system call.\n");
    }
    fflush(stdout); // flush the output
    ptrace(PTRACE_SYSCALL, pid, NULL, NULL);
  }
  return 0;
}
