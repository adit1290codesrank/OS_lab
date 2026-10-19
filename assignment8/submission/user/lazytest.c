#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"
#include "meminfo.h"

#define MB (1024 * 1024)
#define REGION (32 * MB)

struct meminfo mi;

void
show(char *label)
{
  getmeminfo(&mi);
  printf(1, "%s: free=%d proc=%d pt=%d\n", label, mi.free_pages, mi.proc_pages, mi.pt_pages);
}

int
freepages(void)
{
  getmeminfo(&mi);
  return mi.free_pages;
}

void
badchild(char *what, char *addr)
{
  printf(1, "\n-- child touches %s (0x%x) --\n", what, (uint)addr);
  if(fork() == 0){
    *addr = 1;
    printf(1, "ERROR: child survived\n");
    exit();
  }
  wait();
  printf(1, "child killed, parent still alive\n");
}

int
main(void)
{
  int f0, f1, i, fd, n, pfd[2], local;
  char *p, *q;
  int offs[] = { 0, 8 * MB, 16 * MB, REGION - 1 };

  printf(1, "=== Q4(d): sbrk(32 MB) without touching ===\n");
  show("before sbrk");
  f0 = freepages();
  p = sbrk(REGION);
  show("after sbrk ");
  printf(1, "sbrk returned 0x%x, frames used by 32 MB sbrk: %d\n", (uint)p, f0 - freepages());

  printf(1, "\n=== touching 4 widely spaced pages ===\n");
  f1 = freepages();
  for(i = 0; i < 4; i++){
    if(p[offs[i]] != 0)
      printf(1, "ERROR: page not zeroed\n");
    p[offs[i]] = 'A' + i;
  }
  show("after touch");
  printf(1, "frames used by touching 4 pages: %d (4 data + page tables)\n", f1 - freepages());

  printf(1, "\n=== system calls with untouched buffers ===\n");
  f1 = freepages();
  fd = open("README", O_RDONLY);
  q = p + 4 * MB + 100;
  n = read(fd, q, 30);
  close(fd);
  q[n] = 0;
  printf(1, "read() into untouched page returned %d: \"%s\"\n", n, q);
  pipe(pfd);
  n = write(pfd[1], p + 12 * MB, 16);
  printf(1, "write() from untouched page returned %d\n", n);
  n = read(pfd[0], p + 20 * MB, 16);
  printf(1, "read() back %d bytes, first byte = %d\n", n, p[20 * MB]);
  close(pfd[0]);
  close(pfd[1]);
  printf(1, "frames faulted in by the kernel: %d\n", f1 - freepages());

  printf(1, "\n=== fork + exit with a lazy heap ===\n");
  f1 = freepages();
  if(fork() == 0){
    printf(1, "child sees touched pages: %c %c %c %c\n", p[offs[0]], p[offs[1]], p[offs[2]], p[offs[3]]);
    p[24 * MB] = 'Z';
    show("child      ");
    exit();
  }
  wait();
  show("parent after wait");
  printf(1, "free before fork %d, after wait %d -> %s\n", f1, freepages(),
         f1 == freepages() ? "no leak" : "LEAK");

  printf(1, "\n=== invalid accesses ===");
  badchild("address beyond sz", p + REGION + 3 * 4096);
  badchild("stack guard page", (char*)(((uint)&local & ~0xFFF) - 4096));
  badchild("kernel address", (char*)0x80100000);

  printf(1, "\n-- child sbrk's 400 MB and touches every page (out of memory) --\n");
  f1 = freepages();
  if(fork() == 0){
    q = sbrk(400 * MB);
    for(i = 0; i < 400 * MB; i += 4096)
      q[i] = 1;
    printf(1, "ERROR: child survived\n");
    exit();
  }
  wait();
  printf(1, "child killed, parent alive. free before %d, after %d -> %s\n", f1, freepages(),
         f1 == freepages() ? "no leak" : "LEAK");

  printf(1, "\n=== shrinking the lazy heap ===\n");
  sbrk(-REGION);
  show("after sbrk(-32 MB)");
  printf(1, "free at start %d, now %d (page tables are kept until exit)\n", f0, freepages());
  exit();
}
