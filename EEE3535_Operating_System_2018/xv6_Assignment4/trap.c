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

//PAGEBREAK: 41
void
trap(struct trapframe *tf)
{
  //////////////// changed part (added features) /////////////////
  char *mem;	
  uint a;		
  char *temp;	
  pte_t *pte;	
  pde_t *pde;	
  pte_t *pgtab;	
  ////////////////////////////////////////////////////////////////

  if(tf->trapno == T_SYSCALL){
    if(proc->killed)
      exit();
    proc->tf = tf;
    syscall();
    if(proc->killed)
      exit();
    return;
  }

  /* Check if the exception is page fault. */
  if(tf->trapno == T_PGFLT)
  {
	  /* CR2 register contains the faulty memory address. */
	  uint cr2 = rcr2();

	  /////////////////////////// changed part //////////////////////////
	
	  proc->page_used_lazy += 1;	// a page allocated by lazy paging!!

	  /* Allocate a page for the faulty memory address below. */
	  a = PGROUNDDOWN(cr2);		// find the ground of a page which memory address cr2 belongs to.
	  mem = kalloc();			// allocate a page of a physical memory.
	  
	  // if fails to allocate a page in a physical memory, return.
	  if(mem == 0)
	  {
		  cprintf("allocuvm out of memory\n");
		  return;
	  }
	  
	  memset(mem, 0, PGSIZE);	// initialize the allocated page with 0.

	  temp = (char*)(a);

	  pde = &(proc->pgdir)[PDX((void*)temp)];	// pde --> the address of page directory.
	  
	  if(*pde & PTE_P)				// if the corresponding page-table already exists,
	  {
		  pgtab = (pte_t*)P2V(PTE_ADDR(*pde));	// pgtab --> the address of page table.
												// PTE_ADDR extracts PFN from page directory entry(pde)
	  } 
	  else							// if not, allocate a new page table.
	  {
		  if((pgtab = (pte_t*)kalloc()) == 0)
			  return;

		  memset(pgtab, 0, PGSIZE);		// Make sure all those PTE_P bits are zero.
										// recall that the size of page table equals to page size!
		  
		  *pde = V2P(pgtab) | PTE_P | PTE_W | PTE_U;	// fill pde with appropriate PFN.
	  }

	  pte = &pgtab[PTX((void*)temp)];	// get the address of page table entry(pte) using PTE index from virtual memory(temp).

	  *pte = V2P(mem) | PTE_W | PTE_U | PTE_P; // fill page table entry(pte) with PFN.
	  
	  return;
	  ///////////////////////////////////////////////////////////////////
  }


  switch(tf->trapno){
  case T_IRQ0 + IRQ_TIMER:
    if(cpunum() == 0){
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
            cpunum(), tf->cs, tf->eip);
    lapiceoi();
    break;

  //PAGEBREAK: 13
  default:
    if(proc == 0 || (tf->cs&3) == 0){
      // In kernel, it must be our mistake.
      cprintf("unexpected trap %d from cpu %d eip %x (cr2=0x%x)\n",
              tf->trapno, cpunum(), tf->eip, rcr2());
      panic("trap");
    }
    // In user space, assume process misbehaved.
    cprintf("pid %d %s: trap %d err %d on cpu %d "
            "eip 0x%x addr 0x%x--kill proc\n",
            proc->pid, proc->name, tf->trapno, tf->err, cpunum(), tf->eip,
            rcr2());
    proc->killed = 1;
  }

  // Force process exit if it has been killed and is in user space.
  // (If it is still executing in the kernel, let it keep running
  // until it gets to the regular system call return.)
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();

  // Force process to give up CPU on clock tick.
  // If interrupts were on while locks held, would need to check nlock.
  if(proc && proc->state == RUNNING && tf->trapno == T_IRQ0+IRQ_TIMER)
    yield();

  // Check if the process has been killed since we yielded
  if(proc && proc->killed && (tf->cs&3) == DPL_USER)
    exit();
}
