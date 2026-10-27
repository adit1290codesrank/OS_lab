#include "types.h"
#include "defs.h"
#include "param.h"
#include "memlayout.h"
#include "mmu.h"
#include "proc.h"
#include "x86.h"
#include "traps.h"
#include "spinlock.h"

// Interrupt descriptor table (shared by all CPUs).
struct gatedesc idt[256];
extern uint vectors[];  // in vectors.S: array of 256 entry pointers
struct spinlock tickslock;
uint ticks;

void
tvinit(void)
{
  int i;

  for(i = 0; i < 256; i++)
    SETGATE(idt[i], 0, SEG_KCODE<<3, vectors[i], 0);
  SETGATE(idt[T_SYSCALL], 1, SEG_KCODE<<3, vectors[T_SYSCALL], DPL_USER);

  initlock(&tickslock, "time");
}

void
idtinit(void)
{
  lidt(idt, sizeof(idt));
}

// A9 Q2: handle a page fault on a lazily allocated heap page.
// Returns 0 if a zeroed page was mapped, -1 if the fault is invalid.
static int
lazyfault(struct trapframe *tf)
{
  struct proc *p = myproc();
  uint va = rcr2();            // faulting virtual address
  char *mem;

  if(tf->err & 1)              // page was present: protection fault (e.g. guard page), not lazy
    return -1;
  if(va >= p->sz)              // above the process size: invalid access
    return -1;
  if((mem = kalloc()) == 0)    // out of physical memory
    return -1;
  memset(mem, 0, PGSIZE);      // zero-fill so no old data leaks
  if(mappages(p->pgdir, (char*)PGROUNDDOWN(va), PGSIZE, V2P(mem), PTE_W|PTE_U) < 0){
    kfree(mem);
    return -1;
  }
  p->faults++;                 // count the handled fault
  return 0;                    // return from trap re-executes the faulting instruction
}

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  if(tf->trapno == T_SYSCALL){
    if(myproc()->killed)
      exit();
    myproc()->tf = tf;
    syscall();
    if(myproc()->killed)
      exit();
    return;
  }

  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpuid() == 0){
      acquire(&tickslock);
      ticks++;
      wakeup(&ticks);
      release(&tickslock);
    }
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE:
    ideintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_IDE+1:
    // Bochs generates spurious IDE1 interrupts.
    break;
  case T_IRQ0 + IRQ_KBD:
    kbdintr();
    lapiceoi();
    break;
  case T_IRQ0 + IRQ_COM1:
    uartintr();
    lapiceoi();
    break;
  case T_IRQ0 + 7:
  case T_IRQ0 + IRQ_SPURIOUS:
    cprintf("cpu%d: spurious interrupt at %x:%x\n",
            cpuid(), tf->cs, tf->eip);
    lapiceoi();
    break;

  // A9 Q2: page fault - try lazy allocation, otherwise kill the process
  case T_PGFLT:
    if(myproc() != 0 && lazyfault(tf) == 0)
      break;
    if(myproc() != 0 && (tf->cs&3) == DPL_USER){
      cprintf("pid %d %s: invalid page fault at addr 0x%x (sz 0x%x)--kill proc\n",
              myproc()->pid, myproc()->name, rcr2(), myproc()->sz);
      myproc()->killed = 1;
      break;
    }
    // fault inside the kernel on a bad address: fall through to panic

  //PAGEBREAK: 13
  default:
    if(myproc() == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpuid(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            myproc()->pid, myproc()->name, tf->trapno,
            tf->err, cpuid(), tf->eip, rcr2());
    myproc()->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(myproc() && myproc()->state == RUNNING &&
     tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(myproc() && myproc()->killed && (tf->cs&3) == DPL_USER)
    exit();
}
