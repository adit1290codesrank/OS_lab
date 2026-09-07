#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int *shm = (int *)shm_get();
  volatile int *flag = (volatile int *)shm;
  volatile int *turn = (volatile int *)(shm + 2);
  volatile int *counter = (volatile int *)(shm + 3);
  
  flag[0] = 0;
  flag[1] = 0;
  *turn = 0;
  *counter = 0;

  int pid = fork();
  if (pid < 0)
    exit();
  if (pid == 0)
    shm_get();

  int id = (pid == 0) ? 1 : 0;
  int other = 1 - id;

  for (int i = 0; i < 10; i++) {
    flag[id] = 1;
    *turn = other;
    while (flag[other] == 1 && *turn == other)
      ;

    (*counter)++;
    printf(1, "Process %d in CS, counter = %d\n", id, *counter);

    flag[id] = 0;

    for (int j = 0; j < 10000; j++)
      ;
  }

  if (pid > 0) {
    wait();
    printf(1, "Final counter = %d\n", *counter);
  }

  exit();
}
