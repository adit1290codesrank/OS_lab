#include "types.h"
#include "spinlock.h"
#include "x86.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"

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
  if(growproc(n) < 0)
    return -1;
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

void* shm_get_addr(void);

int
sys_shm_get(void)
{
  return (int)shm_get_addr();
}

struct sem {
  struct spinlock lock;
  int count;
};
struct sem sems[10];
int sems_initialized = 0;

int
sys_sem_init(void)
{
  int id, val;
  if(argint(0, &id) < 0 || argint(1, &val) < 0)
    return -1;
  if(!sems_initialized) {
    for(int i=0; i<10; i++) initlock(&sems[i].lock, "sem");
    sems_initialized = 1;
  }
  sems[id].count = val;
  return 0;
}

int
sys_sem_wait(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  acquire(&sems[id].lock);
  while(sems[id].count <= 0){
    sleep(&sems[id], &sems[id].lock);
  }
  sems[id].count--;
  release(&sems[id].lock);
  return 0;
}

int
sys_sem_post(void)
{
  int id;
  if(argint(0, &id) < 0)
    return -1;
  acquire(&sems[id].lock);
  sems[id].count++;
  wakeup(&sems[id]);
  release(&sems[id].lock);
  return 0;
}
