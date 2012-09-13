//------------------------------------------------------------
// Worker source file
//


#include <stdio.h>
#include <unistd.h>
#include <sys/syscall.h>

int main(){

  long rc;
  while (1){
    sleep(1);
    rc = syscall(SYS_getpid);
    if(rc == 0)
      return 0;
    else
      printf("Worker now working on task %ld.\n", rc);
  }
  
  return 0;
}
