#include "types.h"
#include "stat.h"
#include "user.h"
#include "meminfo.h"

#define NPAGES 16

void
show(char *label)
{
  struct meminfo mi;

  getmeminfo(&mi);
  printf(1, "%s: total=%d free=%d proc=%d pt=%d\n",
         label, mi.total_pages, mi.free_pages, mi.proc_pages, mi.pt_pages);
}

int
freepages(void)
{
  struct meminfo mi;

  getmeminfo(&mi);
  return mi.free_pages;
}

int
main(void)
{
  int start, beforefork, afterwait, aftershrink;
  char *p;

  show("start              ");
  start = freepages();

  p = sbrk(NPAGES * 4096);
  memset(p, 'x', NPAGES * 4096);
  show("after sbrk(16 pages)");
  printf(1, "  free pages dropped by %d\n", start - freepages());

  beforefork = freepages();
  if(fork() == 0){
    show("in child            ");
    printf(1, "  child costs %d pages (user copy + page tables + kstack)\n",
           beforefork - freepages());
    exit();
  }
  wait();
  show("after child reaped  ");
  afterwait = freepages();
  printf(1, "  before fork %d, after wait %d -> %s\n", beforefork, afterwait,
         beforefork == afterwait ? "no leak" : "LEAK");

  sbrk(-NPAGES * 4096);
  show("after sbrk(-16 pages)");
  aftershrink = freepages();
  printf(1, "  start %d, after shrink %d -> %s\n", start, aftershrink,
         start == aftershrink ? "back to original" : "DIFFERENT");
  exit();
}
