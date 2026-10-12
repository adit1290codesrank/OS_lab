#include "types.h"
#include "stat.h"
#include "user.h"

#define NTHREADS 3
#define ITERS 100000
#define PGSIZE 4096

volatile int counter = 0;
volatile int go = 0;
char stacks[NTHREADS][PGSIZE] __attribute__((aligned(PGSIZE)));

void
worker(void *arg)
{
  int i;
  while(!go)
    ;
  for(i = 0; i < ITERS; i++)
    counter++;
  exit();
}

int
main(int argc, char *argv[])
{
  int i, pid;

  printf(1, "threadtest: parent pid %d, %d threads x %d increments, no synchronization\n", getpid(), NTHREADS, ITERS);
  for(i = 0; i < NTHREADS; i++){
    pid = clone(worker, (void*)i, stacks[i]);
    if(pid < 0){
      printf(1, "clone failed\n");
      exit();
    }
    printf(1, "created thread %d (pid %d)\n", i, pid);
  }
  go = 1;
  for(i = 0; i < NTHREADS; i++)
    printf(1, "joined thread pid %d\n", join());
  printf(1, "final counter = %d (expected %d)\n", counter, NTHREADS * ITERS);
  if(counter != NTHREADS * ITERS)
    printf(1, "race condition: lost %d updates\n", NTHREADS * ITERS - counter);
  exit();
}
