#include "types.h"
#include "stat.h"
#include "user.h"

#define PGSIZE 4096

int
main(void)
{
  int pages[5] = {0, 7, 15, 23, 31};   // 5 different pages of the 32-page region
  int i, pid;

  printf(1, "faults at start: %d\n", getfaults());

  char *p = sbrk(32 * PGSIZE);          // lazy: only sz grows, no frames yet
  printf(1, "sbrk(32 * 4096) -> 0x%x, faults after sbrk: %d\n", p, getfaults());

  for(i = 0; i < 5; i++){               // first write to each page -> 1 fault each
    p[pages[i] * PGSIZE] = 'A' + i;
    printf(1, "wrote page %d (addr 0x%x), faults = %d\n",
           pages[i], p + pages[i] * PGSIZE, getfaults());
  }

  p[0] = 'Z';                           // pages already mapped -> no new faults
  p[pages[4] * PGSIZE + 100] = 'Z';
  printf(1, "rewrote 2 already-touched pages, faults = %d\n", getfaults());
  printf(1, "page faults handled: %d (expected 5)\n", getfaults());

  pid = fork();
  if(pid == 0){                         // child: access above sz must kill only the child
    char *bad = sbrk(0) + PGSIZE;
    printf(1, "child %d: writing above sz at 0x%x\n", getpid(), bad);
    *bad = 1;
    printf(1, "child: ERROR, still alive\n");
    exit();
  }
  wait();
  printf(1, "parent %d: child was killed, parent still alive, faults = %d\n",
         getpid(), getfaults());
  exit();
}
