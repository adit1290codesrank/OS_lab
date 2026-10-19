#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  char *p;
  int i;

  printf(1, "=== vmprint at start-up (sz = 0x%x) ===\n", (uint)sbrk(0));
  vmprint();

  p = sbrk(8 * 4096);
  for(i = 0; i < 8; i++)
    p[i * 4096] = i;
  printf(1, "\n=== vmprint after sbrk(8 * 4096) (sz = 0x%x) ===\n", (uint)sbrk(0));
  vmprint();

  sbrk(-4096);
  printf(1, "\n=== vmprint after sbrk(-4096) (sz = 0x%x) ===\n", (uint)sbrk(0));
  vmprint();
  exit();
}
