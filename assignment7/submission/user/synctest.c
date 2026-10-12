#include "types.h"
#include "stat.h"
#include "user.h"

#define NTHREADS 3
#define ITERS 100000
#define PGSIZE 4096
#define BUFSZ 5
#define NITEMS 20

#define MUTEX 0
#define EMPTY 1
#define FULL 2
#define BUFMUTEX 3

volatile int counter = 0;
volatile int go = 0;
char stacks[NTHREADS][PGSIZE] __attribute__((aligned(PGSIZE)));

int buffer[BUFSZ];
volatile int in = 0, out = 0, count = 0;
volatile int overflow = 0, underflow = 0;

void
worker(void *arg)
{
  int i;
  while(!go)
    ;
  for(i = 0; i < ITERS; i++){
    sem_wait(MUTEX);
    counter++;
    sem_signal(MUTEX);
  }
  exit();
}

void
producer(void *arg)
{
  int i;
  for(i = 1; i <= NITEMS; i++){
    sem_wait(EMPTY);
    sem_wait(BUFMUTEX);
    buffer[in] = i;
    in = (in + 1) % BUFSZ;
    count++;
    if(count > BUFSZ)
      overflow = 1;
    printf(1, "producer: produced %d  [buffer count=%d]\n", i, count);
    sem_signal(BUFMUTEX);
    sem_signal(FULL);
  }
  exit();
}

void
consumer(void *arg)
{
  int i, item;
  for(i = 1; i <= NITEMS; i++){
    sem_wait(FULL);
    sem_wait(BUFMUTEX);
    item = buffer[out];
    out = (out + 1) % BUFSZ;
    count--;
    if(count < 0)
      underflow = 1;
    printf(1, "consumer: consumed %d  [buffer count=%d]\n", item, count);
    sem_signal(BUFMUTEX);
    sem_signal(EMPTY);
  }
  exit();
}

int
main(int argc, char *argv[])
{
  int i;

  printf(1, "synctest part 1: %d threads x %d increments, guarded by semaphore\n", NTHREADS, ITERS);
  sem_init(MUTEX, 1);
  for(i = 0; i < NTHREADS; i++)
    clone(worker, 0, stacks[i]);
  go = 1;
  for(i = 0; i < NTHREADS; i++)
    join();
  printf(1, "final counter = %d (expected %d) -> %s\n", counter, NTHREADS * ITERS,
         counter == NTHREADS * ITERS ? "CORRECT" : "WRONG");

  printf(1, "\nsynctest part 2: producer-consumer, buffer size %d, %d items\n", BUFSZ, NITEMS);
  sem_init(EMPTY, BUFSZ);
  sem_init(FULL, 0);
  sem_init(BUFMUTEX, 1);
  clone(producer, 0, stacks[0]);
  clone(consumer, 0, stacks[1]);
  join();
  join();
  printf(1, "producer-consumer done: final count=%d, overflow=%s, underflow=%s\n",
         count, overflow ? "YES" : "no", underflow ? "YES" : "no");
  exit();
}
