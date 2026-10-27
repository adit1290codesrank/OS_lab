#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
  printf(1, "=== page table at start-up ===\n");
  vmprint();

  char *p = sbrk(4 * 4096);
  for(int i = 0; i < 4; i++)   // lazy sbrk: touch each page so the page fault maps it
    p[i * 4096] = 1;
  printf(1, "\n=== after sbrk(4 * 4096), new region 0x%x - 0x%x ===\n", p, p + 4 * 4096);
  vmprint();

  exit();
}
