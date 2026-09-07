#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int buf_size = 5;
  if(argc > 1) buf_size = atoi(argv[1]);
  if(buf_size > 20) buf_size = 20;

  int *shm = (int *)shm_get();
  volatile int *in = (volatile int *)shm;
  volatile int *out = (volatile int *)(shm + 1);
  volatile int *buffer = (volatile int *)(shm + 2);
  
  *in = 0;
  *out = 0;

  sem_init(0, buf_size);
  sem_init(1, 0);
  sem_init(2, 1);

  int pid = fork();
  if (pid < 0) exit();
  if (pid == 0) {
    shm_get();
    for(int i=0; i<20; i++){
      sem_wait(1);
      sem_wait(2);
      int item = buffer[*out];
      *out = (*out + 1) % buf_size;
      printf(1, "Consumed: %d\n", item);
      sem_post(2);
      sem_post(0);
      if (i < 10) sleep(20);
      else sleep(1);
    }
    exit();
  } else {
    for(int i=1; i<=20; i++){
      sem_wait(0);
      sem_wait(2);
      buffer[*in] = i;
      *in = (*in + 1) % buf_size;
      printf(1, "Produced: %d\n", i);
      sem_post(2);
      sem_post(1);
      if (i < 10) sleep(1);
      else sleep(20);
    }
    wait();
  }
  exit();
}
