#include "types.h"
#include "stat.h"
#include "user.h"

int
main(int argc, char *argv[])
{
  int *shm = (int *)shm_get();
  volatile int *shared_data = (volatile int *)shm;
  volatile int *read_count = (volatile int *)(shm + 1);
  
  *shared_data = 0;
  *read_count = 0;

  sem_init(3, 1);
  sem_init(4, 1);
  sem_init(5, 1);

  for (int i = 0; i < 3; i++) {
    if (fork() == 0) {
      shm_get();
      for(int j=0; j<3; j++) {
        sem_wait(5);
        sem_wait(3);
        (*read_count)++;
        if (*read_count == 1)
          sem_wait(4);
        sem_post(3);
        sem_post(5);

        printf(1, "Reader %d reads data = %d, active readers = %d\n", getpid(), *shared_data, *read_count);
        sleep(10);

        sem_wait(3);
        (*read_count)--;
        if (*read_count == 0)
          sem_post(4);
        sem_post(3);

        sleep(10);
      }
      exit();
    }
  }

  for (int i = 0; i < 2; i++) {
    if (fork() == 0) {
      shm_get();
      for(int j=0; j<3; j++) {
        sem_wait(5);
        sem_wait(4);
        
        (*shared_data)++;
        printf(1, "Writer %d writes data = %d\n", getpid(), *shared_data);
        sleep(10);
        
        sem_post(4);
        sem_post(5);

        sleep(20);
      }
      exit();
    }
  }

  for (int i = 0; i < 5; i++)
    wait();

  exit();
}
