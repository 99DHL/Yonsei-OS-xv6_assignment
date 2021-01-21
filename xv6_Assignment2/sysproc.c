#include "types.h"
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
  proc->count_fork += 1;  // changed part
  return fork();
}

int
sys_exit(void)
{
  proc->count_exit += 1;  // changed part
  exit();
  return 0;  // not reached
}

int
sys_wait(void)
{
  proc->count_wait += 1;  // changed part
  return wait();
}

int
sys_kill(void)
{
  proc->count_kill += 1;  // changed part
  int pid;

  if(argint(0, &pid) < 0)
    return -1;
  return kill(pid);
}

int
sys_getpid(void)
{
  proc->count_getpid += 1; // changed part
  return proc->pid;
}

int
sys_sbrk(void)
{
  proc->count_sbrk += 1; // changed part
  int addr;
  int n;

  if(argint(0, &n) < 0)
    return -1;
  addr = proc->sz;
  if(growproc(n) < 0)
    return -1;
  return addr;
}

int
sys_sleep(void)
{
  proc->count_sleep += 1; // changed part
  int n;
  uint ticks0;

  if(argint(0, &n) < 0)
    return -1;
  acquire(&tickslock);
  ticks0 = ticks;
  while(ticks - ticks0 < n){
    if(proc->killed){
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
  proc->count_uptime += 1;	// changed part
  uint xticks;

  acquire(&tickslock);
  xticks = ticks;
  release(&tickslock);
  return xticks;
}

/////////////////// changed part(added sys_count function) ///////////////////
int
sys_count(void)
{
	int n;
	proc->count_count += 1;
	argint(0,&n);

	switch(n)
	{
		case 1:
			return proc->count_fork;
			break;
		case 2:
			return proc->count_exit;
			break;
		case 3:
			return proc->count_wait;
			break;
		case 4:
			return proc->count_pipe;
			break;
		case 5:
			return proc->count_read;
			break;
		case 6:
			return proc->count_kill;
			break;
		case 7:
			return proc->count_exec;
			break;
		case 8:
			return proc->count_fstat;
			break;
		case 9:
			return proc->count_chdir;
			break;
		case 10:
			return proc->count_dup;
			break;
		case 11:
			return proc->count_getpid;
			break;
		case 12:
			return proc->count_sbrk;
			break;
		case 13:
			return proc->count_sleep;
			break;
		case 14:
			return proc->count_uptime;
			break;
		case 15:
			return proc->count_open;
			break;
		case 16:
			return proc->count_write;
			break;
		case 17:
			return proc->count_mknod;
			break;
		case 18:
			return proc->count_unlink;
			break;
		case 19:
			return proc->count_link;
			break;
		case 20:
			return proc->count_mkdir;
			break;
		case 21:
			return proc->count_close;
			break;
		case 22:
			return proc->count_count;
			break;
		default:
			return 0;
	}
}

