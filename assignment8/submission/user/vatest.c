#include "types.h"
#include "stat.h"
#include "user.h"

int global = 42;

void
show(char *name, void *va)
{
  int pa = va2pa((int)va);
  printf(1, "%s: va 0x%x -> pa 0x%x\n\n", name, (uint)va, pa);
}

int
main(void)
{
  int local = 7;
  char *block = malloc(100);
  int pa, pid;

  printf(1, "=== Q1(c): VA -> PA of different variables ===\n");
  show("global", &global);
  show("local ", &local);
  show("malloc", block);

  pa = va2pa(0x7FFFFFFF);
  printf(1, "unmapped 0x7FFFFFFF: va2pa returned %d\n\n", pa);
  pa = va2pa(0x80100000);
  printf(1, "kernel   0x80100000: va2pa returned %d\n\n", pa);

  printf(1, "=== Q1(d): same VA after fork ===\n");
  global = 100;
  pid = fork();
  if(pid == 0){
    pa = va2pa((int)&global);
    printf(1, "child : &global = 0x%x pa = 0x%x global = %d\n\n", (uint)&global, pa, global);
    exit();
  }
  wait();
  pa = va2pa((int)&global);
  printf(1, "parent: &global = 0x%x pa = 0x%x global = %d\n", (uint)&global, pa, global);
  printf(1, "same virtual address, different physical frames (fork copies pages)\n");
  free(block);
  exit();
}
