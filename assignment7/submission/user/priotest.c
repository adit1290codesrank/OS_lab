#include "types.h"
#include "stat.h"
#include "user.h"

#define NCHILD 4
#define CHUNKS 2000
#define CHUNK 400000

int prios[NCHILD] = {2, 6, 12, 20};

void
work(int start, int initprio)
{
  volatile int x = 0;
  int c, i;

  for(c = 0; c < CHUNKS; c++)
    for(i = 0; i < CHUNK; i++)
      x += i;
  printf(1, "  pid %d (start prio %d) FINISHED at tick %d, final prio %d\n",
         getpid(), initprio, uptime() - start, get_priority(getpid()));
}

void
monitor(int pid, int start)
{
  int pr, last = 20, lastchange = 0, t;
  while((pr = get_priority(pid)) > 0){
    if(pr != last){
      t = uptime() - start;
      printf(1, "  [monitor] pid %d (start prio 20): priority %d -> %d at tick %d (waited %d ticks)\n",
             pid, last, pr, t, t - lastchange);
      last = pr;
      lastchange = t;
    }
    sleep(2);
  }
}

int
main(int argc, char *argv[])
{
  int pids[NCHILD], order[NCHILD], done[NCHILD];
  int i, j, pid, start, mon;

  set_priority(getpid(), 1);
  start = uptime();
  printf(1, "priotest: forking %d CPU-bound children, aging every 30 waiting ticks\n", NCHILD);
  for(i = 0; i < NCHILD; i++){
    pid = fork();
    if(pid == 0){
      work(start, prios[i]);
      exit();
    }
    pids[i] = pid;
    set_priority(pid, prios[i]);
    printf(1, "  child pid %d -> priority %d\n", pid, prios[i]);
  }

  if((mon = fork()) == 0){
    monitor(pids[NCHILD - 1], start);
    exit();
  }
  set_priority(mon, 1);

  for(i = 0; i < NCHILD; i++){
    pid = wait();
    if(pid == mon){
      i--;
      continue;
    }
    for(j = 0; j < NCHILD; j++)
      if(pids[j] == pid)
        order[i] = j;
    done[i] = uptime() - start;
  }

  printf(1, "\nCompletion order:\n");
  printf(1, "  rank\tpid\tpriority\tcompletion(ticks)\n");
  for(i = 0; i < NCHILD; i++)
    printf(1, "  %d\t%d\t%d\t\t%d\n", i + 1, pids[order[i]], prios[order[i]], done[i]);
  wait();
  exit();
}
