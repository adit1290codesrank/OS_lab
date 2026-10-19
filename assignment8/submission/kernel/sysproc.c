#include "types.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "meminfo.h"

int
sys_fork(void)
{
  return fork();
}

int
sys_exit(void)
{
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  return wait();
}

int
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  return myproc()->pid;
}

int
sys_sbrk(void)
{
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = myproc()->sz;
  if(n < 0){
    if(growproc(n) < 0)
      return -1;
  } else {
    if(addr + n >= KERNBASE || addr + n < addr)
      return -1;
    myproc()->sz += n;
  }
  return addr;
}

int
sys_sleep(void)
{
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(myproc()->killed){
      release(&tickslock);
      return -1;
    }
    sleep(&ticks, &tickslock);
  }
  release(&tickslock);
  return 0;
}

// return how many clock tick interrupts have occurred
// since start.
int
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

int
sys_va2pa(void)
{
  int va;
  pte_t *pte;
  uint pa;

  if(argint(0, &va) < 0)
    return -1;
  pte = walkpgdir(myproc()->pgdir, (void*)va, 0);
  if(pte == 0 || !(*pte & PTE_P) || !(*pte & PTE_U)){
    cprintf("[kernel] va2pa: va 0x%x pdx %d ptx %d off 0x%x -> not mapped / not user\n",
            va, PDX(va), PTX(va), va & 0xFFF);
    return -1;
  }
  pa = PTE_ADDR(*pte) | (va & 0xFFF);
  cprintf("[kernel] va2pa: va 0x%x pdx %d ptx %d off 0x%x -> pa 0x%x flags P=%d W=%d U=%d\n",
          va, PDX(va), PTX(va), va & 0xFFF, pa,
          (*pte & PTE_P) != 0, (*pte & PTE_W) != 0, (*pte & PTE_U) != 0);
  return pa;
}

int
sys_getmeminfo(void)
{
  struct meminfo *mi;
  struct proc *p = myproc();

  if(argptr(0, (void*)&mi, sizeof(*mi)) < 0)
    return -1;
  kmemstat(&mi->total_pages, &mi->free_pages);
  mi->proc_pages = p->sz / PGSIZE;
  mi->pt_pages = ptpages(p->pgdir);
  return 0;
}

int
sys_vmprint(void)
{
  vmprint(myproc()->pgdir, myproc()->sz);
  return 0;
}
