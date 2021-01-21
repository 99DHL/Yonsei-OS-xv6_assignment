#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "date.h"
#include "param.h"
#include "memlayout.h"
#include "spinlock.h"
#include "proc.h"

uint64
sys_exit(void)
{
  int n;
  if(argint(0, &n) < 0)
    return -1;
  exit(n);
  return 0;  // not reached
}

uint64
sys_getpid(void)
{
  return myproc()->pid;
}

uint64
sys_fork(void)
{
  return fork();
}

uint64
sys_wait(void)
{
  uint64 p;
  if(argaddr(0, &p) < 0)
    return -1;
  return wait(p);
}

uint64
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

uint64
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

uint64
sys_kill(void)
{
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

// return how many clock tick interrupts have occurred
// since start.
uint64
sys_uptime(void)
{
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/////////////////////////////// changed part //////////////////////////////////

// Return the physical address of a virtual address.
uint64
sys_phyaddr(void)
{
  int va;		// virtual address
  int offset;	// offset of the address
  uint64 pa;	// physical address
  pte_t *pte;	// page table entry address
  pagetable_t pagetable = myproc()->pagetable;	// get the pagetable
  argint(0, &va);								// get virtual address

  offset = va & (0xFFF);

  for(int level = 2; level > 0; level--) 
  {
    pte = &pagetable[PX(level, va)];			// get the address of pte
    
	if(*pte & PTE_V) 
	{
      pagetable = (pagetable_t)PTE2PA(*pte);	// get the address of pagetable
    } 
	else 
	{
	  return -1;
    }
  }

  pte = &pagetable[PX(0,va)];	// address of level_0 pte
  pa = PTE2PA(*pte) + offset;	// level_0 pte -> final physical address with offset 0, offset must be added!!
  return pa;					// return physical address

}

// Return the page table (or directory) index of a virtual address
// at the specified page table level.
uint64
sys_ptidx(void)
{
  int level = 0;	// level of pte
  int va = 0;		// virtual address
  argint(0, &va);
  argint(1, &level);

  return (uint64)PX(level, va);
}

// Count the total number pages allocated by a process.
uint64
sys_pgcnt(void)
{
  uint64 i, j, k;
  pagetable_t pagetable_2, pagetable_1, pagetable_0;	// pagetable(level 0,1,2)
  pte_t *pte;				// temporary variable to save pte

  int page_count = 0;			// initialize page_count

  pagetable_2 = myproc()->pagetable;
  for(i = 0; i < 512; i++)		// level 2
  {
	pte = &pagetable_2[i];
	if(*pte & PTE_V)
	{
	  pagetable_1 = (pagetable_t)PTE2PA(*pte); 
	  for(j = 0; j < 512; j++)		// level 1
	  {
		pte = &pagetable_1[j];
		if(*pte & PTE_V)
		{
		  pagetable_0 = (pagetable_t)PTE2PA(*pte);
		  for(k = 0; k < 512; k++)	// level 0
		  {
			pte = &pagetable_0[k];
			if(*pte & PTE_V)
			{
			  page_count++;		// valid page found!!
			}
		  }
		}
	  }
	}
  }

  return (uint64)page_count;

}

///////////////////////////////////////////////////////////////////////////////

