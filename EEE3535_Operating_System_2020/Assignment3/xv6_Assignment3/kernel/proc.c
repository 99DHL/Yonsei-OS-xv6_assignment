#include "types.h"
#include "param.h"
#include "memlayout.h"
#include "riscv.h"
#include "spinlock.h"
#include "proc.h"
#include "defs.h"

struct cpu cpus[NCPU];

struct proc proc[NPROC];

struct proc *initproc;

int nextpid = 1;
struct spinlock pid_lock;

extern void forkret(void);
static void wakeup1(struct proc *chan);
static void freeproc(struct proc *p);

extern char trampoline[]; // trampoline.S

// initialize the proc table at boot time.
void
procinit(void)
{
  struct proc *p;
  
  initlock(&pid_lock, "nextpid");
  for(p = proc; p < &proc[NPROC]; p++) {
      initlock(&p->lock, "proc");

      // Allocate a page for the process's kernel stack.
      // Map it high in memory, followed by an invalid
      // guard page.
      char *pa = kalloc();
      if(pa == 0)
        panic("kalloc");
      uint64 va = KSTACK((int) (p - proc));
      kvmmap(va, (uint64)pa, PGSIZE, PTE_R | PTE_W);
      p->kstack = va;
  }
  kvminithart();
}

// Must be called with interrupts disabled,
// to prevent race with process being moved
// to a different CPU.
int
cpuid()
{
  int id = r_tp();
  return id;
}

// Return this CPU's cpu struct.
// Interrupts must be disabled.
struct cpu*
mycpu(void) {
  int id = cpuid();
  struct cpu *c = &cpus[id];
  return c;
}

// Return the current struct proc *, or zero if none.
struct proc*
myproc(void) {
  push_off();
  struct cpu *c = mycpu();
  struct proc *p = c->proc;
  pop_off();
  return p;
}

int
allocpid() {
  int pid;
  
  acquire(&pid_lock);
  pid = nextpid;
  nextpid = nextpid + 1;
  release(&pid_lock);

  return pid;
}

// Look in the process table for an UNUSED proc.
// If found, initialize state required to run in the kernel,
// and return with p->lock held.
// If there are no free procs, or a memory allocation fails, return 0.
static struct proc*
allocproc(void)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == UNUSED) {
      goto found;
    } else {
      release(&p->lock);
    }
  }
  return 0;

found:
  p->pid = allocpid();

  ////////////////////////// changed part ///////////////////////////
  
  p->is_new = 1;	// initialize p->is_new
  p->is_sys = 0;	// initialize p->is_sys
  p->is_yield = 0;	// initialize p->is_yield
  p->q0 = 0;	p->q1 = 0;	  p->q2 = 0;
  p->start = 0;		p->end = 0;

  ///////////////////////////////////////////////////////////////////

  // Allocate a trapframe page.
  if((p->trapframe = (struct trapframe *)kalloc()) == 0){
    release(&p->lock);
    return 0;
  }

  // An empty user page table.
  p->pagetable = proc_pagetable(p);
  if(p->pagetable == 0){
    freeproc(p);
    release(&p->lock);
    return 0;
  }

  // Set up new context to start executing at forkret,
  // which returns to user space.
  memset(&p->context, 0, sizeof(p->context));
  p->context.ra = (uint64)forkret;
  p->context.sp = p->kstack + PGSIZE;

  return p;
}

// free a proc structure and the data hanging from it,
// including user pages.
// p->lock must be held.
static void
freeproc(struct proc *p)
{
  // Print out the runtime stats of queue occupancy.
  int total_time = p->q0 + p->q1 + p->q2;
  printf("%s (pid=%d): Q2(%d%%), Q1(%d%%), Q0(%d%%)\n",
         p->name, p->pid, (int)((100*p->q2)/total_time), (int)((100*p->q1)/total_time), (int)((100*p->q0)/total_time));

  if(p->trapframe)
    kfree((void*)p->trapframe);
  p->trapframe = 0;
  if(p->pagetable)
    proc_freepagetable(p->pagetable, p->sz);
  p->pagetable = 0;
  p->sz = 0;
  p->pid = 0;
  p->parent = 0;
  p->name[0] = 0;
  p->chan = 0;
  p->killed = 0;
  p->xstate = 0;
  p->state = UNUSED;
}

// Create a user page table for a given process,
// with no user memory, but with trampoline pages.
pagetable_t
proc_pagetable(struct proc *p)
{
  pagetable_t pagetable;

  // An empty page table.
  pagetable = uvmcreate();
  if(pagetable == 0)
    return 0;

  // map the trampoline code (for system call return)
  // at the highest user virtual address.
  // only the supervisor uses it, on the way
  // to/from user space, so not PTE_U.
  if(mappages(pagetable, TRAMPOLINE, PGSIZE,
              (uint64)trampoline, PTE_R | PTE_X) < 0){
    uvmfree(pagetable, 0);
    return 0;
  }

  // map the trapframe just below TRAMPOLINE, for trampoline.S.
  if(mappages(pagetable, TRAPFRAME, PGSIZE,
              (uint64)(p->trapframe), PTE_R | PTE_W) < 0){
    uvmunmap(pagetable, TRAMPOLINE, 1, 0);
    uvmfree(pagetable, 0);
    return 0;
  }

  return pagetable;
}

// Free a process's page table, and free the
// physical memory it refers to.
void
proc_freepagetable(pagetable_t pagetable, uint64 sz)
{
  uvmunmap(pagetable, TRAMPOLINE, 1, 0);
  uvmunmap(pagetable, TRAPFRAME, 1, 0);
  uvmfree(pagetable, sz);
}

// a user program that calls exec("/init")
// od -t xC initcode
uchar initcode[] = {
  0x17, 0x05, 0x00, 0x00, 0x13, 0x05, 0x45, 0x02,
  0x97, 0x05, 0x00, 0x00, 0x93, 0x85, 0x35, 0x02,
  0x93, 0x08, 0x70, 0x00, 0x73, 0x00, 0x00, 0x00,
  0x93, 0x08, 0x20, 0x00, 0x73, 0x00, 0x00, 0x00,
  0xef, 0xf0, 0x9f, 0xff, 0x2f, 0x69, 0x6e, 0x69,
  0x74, 0x00, 0x00, 0x24, 0x00, 0x00, 0x00, 0x00,
  0x00, 0x00, 0x00, 0x00
};

// Set up first user process.
void
userinit(void)
{
  struct proc *p;

  p = allocproc();
  initproc = p;
  
  // allocate one user page and copy init's instructions
  // and data into it.
  uvminit(p->pagetable, initcode, sizeof(initcode));
  p->sz = PGSIZE;

  // prepare for the very first "return" from kernel to user.
  p->trapframe->epc = 0;      // user program counter
  p->trapframe->sp = PGSIZE;  // user stack pointer

  safestrcpy(p->name, "initcode", sizeof(p->name));
  p->cwd = namei("/");

  p->state = RUNNABLE;

  release(&p->lock);
}

// Grow or shrink user memory by n bytes.
// Return 0 on success, -1 on failure.
int
growproc(int n)
{
  uint sz;
  struct proc *p = myproc();

  sz = p->sz;
  if(n > 0){
    if((sz = uvmalloc(p->pagetable, sz, sz + n)) == 0) {
      return -1;
    }
  } else if(n < 0){
    sz = uvmdealloc(p->pagetable, sz, sz + n);
  }
  p->sz = sz;
  return 0;
}

// Create a new process, copying the parent.
// Sets up child kernel stack to return as if from fork() system call.
int
fork(void)
{
  int i, pid;
  struct proc *np;
  struct proc *p = myproc();

  // Allocate process.
  if((np = allocproc()) == 0){
    return -1;
  }

  // Copy user memory from parent to child.
  if(uvmcopy(p->pagetable, np->pagetable, p->sz) < 0){
    freeproc(np);
    release(&np->lock);
    return -1;
  }
  np->sz = p->sz;

  np->parent = p;

  // copy saved user registers.
  *(np->trapframe) = *(p->trapframe);

  // Cause fork to return 0 in the child.
  np->trapframe->a0 = 0;

  // increment reference counts on open file descriptors.
  for(i = 0; i < NOFILE; i++)
    if(p->ofile[i])
      np->ofile[i] = filedup(p->ofile[i]);
  np->cwd = idup(p->cwd);

  safestrcpy(np->name, p->name, sizeof(p->name));

  pid = np->pid;

  np->state = RUNNABLE;

  release(&np->lock);

  return pid;
}

// Pass p's abandoned children to init.
// Caller must hold p->lock.
void
reparent(struct proc *p)
{
  struct proc *pp;

  for(pp = proc; pp < &proc[NPROC]; pp++){
    // this code uses pp->parent without holding pp->lock.
    // acquiring the lock first could cause a deadlock
    // if pp or a child of pp were also in exit()
    // and about to try to lock p.
    if(pp->parent == p){
      // pp->parent can't change between the check and the acquire()
      // because only the parent changes it, and we're the parent.
      acquire(&pp->lock);
      pp->parent = initproc;
      // we should wake up init here, but that would require
      // initproc->lock, which would be a deadlock, since we hold
      // the lock on one of init's children (pp). this is why
      // exit() always wakes init (before acquiring any locks).
      release(&pp->lock);
    }
  }
}

// Exit the current process.  Does not return.
// An exited process remains in the zombie state
// until its parent calls wait().
void
exit(int status)
{
  struct proc *p = myproc();

  if(p == initproc)
    panic("init exiting");

  // Close all open files.
  for(int fd = 0; fd < NOFILE; fd++){
    if(p->ofile[fd]){
      struct file *f = p->ofile[fd];
      fileclose(f);
      p->ofile[fd] = 0;
    }
  }

  begin_op();
  iput(p->cwd);
  end_op();
  p->cwd = 0;

  // we might re-parent a child to init. we can't be precise about
  // waking up init, since we can't acquire its lock once we've
  // acquired any other proc lock. so wake up init whether that's
  // necessary or not. init may miss this wakeup, but that seems
  // harmless.
  acquire(&initproc->lock);
  wakeup1(initproc);
  release(&initproc->lock);

  // grab a copy of p->parent, to ensure that we unlock the same
  // parent we locked. in case our parent gives us away to init while
  // we're waiting for the parent lock. we may then race with an
  // exiting parent, but the result will be a harmless spurious wakeup
  // to a dead or wrong process; proc structs are never re-allocated
  // as anything else.
  acquire(&p->lock);
  struct proc *original_parent = p->parent;
  release(&p->lock);
  
  // we need the parent's lock in order to wake it up from wait().
  // the parent-then-child rule says we have to lock it first.
  acquire(&original_parent->lock);

  acquire(&p->lock);

  // Give any children to init.
  reparent(p);

  // Parent might be sleeping in wait().
  wakeup1(original_parent);

  p->xstate = status;
  p->state = ZOMBIE;

  release(&original_parent->lock);

  // Jump into the scheduler, never to return.
  sched();
  panic("zombie exit");
}

// Wait for a child process to exit and return its pid.
// Return -1 if this process has no children.
int
wait(uint64 addr)
{
  struct proc *np;
  int havekids, pid;
  struct proc *p = myproc();

  // hold p->lock for the whole time to avoid lost
  // wakeups from a child's exit().
  acquire(&p->lock);

  for(;;){
    // Scan through table looking for exited children.
    havekids = 0;
    for(np = proc; np < &proc[NPROC]; np++){
      // this code uses np->parent without holding np->lock.
      // acquiring the lock first would cause a deadlock,
      // since np might be an ancestor, and we already hold p->lock.
      if(np->parent == p){
        // np->parent can't change between the check and the acquire()
        // because only the parent changes it, and we're the parent.
        acquire(&np->lock);
        havekids = 1;
        if(np->state == ZOMBIE){
          // Found one.
          pid = np->pid;
          if(addr != 0 && copyout(p->pagetable, addr, (char *)&np->xstate,
                                  sizeof(np->xstate)) < 0) {
            release(&np->lock);
            release(&p->lock);
            return -1;
          }
          freeproc(np);
          release(&np->lock);
          release(&p->lock);
          return pid;
        }
        release(&np->lock);
      }
    }

    // No point waiting if we don't have any children.
    if(!havekids || p->killed){
      release(&p->lock);
      return -1;
    }
    
    // Wait for a child to exit.
    sleep(p, &p->lock);  //DOC: wait-sleep
  }
}


////////////////////////////// changed part //////////////////////////////
// MLFQ functions

// push back a new process to MLFQ
void push_back(struct Queue *queue, struct proc *proc)
{
  struct Queue *temp = (struct Queue*)kalloc();
  temp->next = 0; temp->p = proc;

  struct Queue *iter = queue;
  while(iter->next != 0)
  {
	iter = iter->next;
  }
  iter->next = temp;
}

// pop the target from the MLFQ --> different from general pop function in queue!!
struct proc* pop(struct Queue *queue, struct Queue *target)
{
  struct Queue *iter = queue;
  struct Queue *ffree = target;
  struct proc *temp = target->p;
  
  while(iter->next != target)
  {
	iter = iter->next;
  }

  iter->next = iter->next->next;
  kfree(ffree);
  return temp;
}
//////////////////////////////////////////////////////////////////////////


// Per-CPU process scheduler.
// Each CPU calls scheduler() after setting itself up.
// Scheduler never returns.  It loops, doing:
//  - choose a process to run.
//  - swtch to start running that process.
//  - eventually that process transfers control
//    via swtch back to the scheduler.
void
scheduler(void)
{
  struct proc *p;
  struct cpu *c = mycpu();  
  c->proc = 0;

  ///////////////////////////////// changed part ////////////////////////////////////

  // allocate MLFQ and initialize them. (cannot use malloc in kernel.)
  struct Queue *Q0 = (struct Queue*)kalloc();
  struct Queue *Q1 = (struct Queue*)kalloc();
  struct Queue *Q2 = (struct Queue*)kalloc();
  Q0->p = 0;	Q0->next = 0;
  Q1->p = 0;	Q1->next = 0;
  Q2->p = 0;	Q2->next = 0;

  // define varibles for scheduling.
  int current_queue = 2;
  struct Queue *current_pos_1 = Q1;
  struct Queue *current_pos_2 = Q2;

  for(;;){
    // Avoid deadlock by ensuring that devices can interrupt.
    intr_on();
    
	// new process is first placed in Q2.
	for(p = proc; p < &proc[NPROC]; p++) 
	{
      if(p->state == RUNNABLE && p->is_new == 1)
	  {	
		push_back(Q2, p);
		p->start = ticks;	// Q2 execution time starts.
		p->is_new = 0;
	  }
    }

	// update Q0
	struct proc *temp;
	struct Queue *q;
	struct Queue *iter = Q0->next;
	while(iter != 0)
	{
	  // if the process is terminated, evict it from MLFQ.
	  if(iter->p->state == UNUSED)
	  {
		q = iter->next;
		pop(Q0, iter);
		iter->p->end = ticks;
		iter->p->q0 += (int)(iter->p->end - iter->p->start);	// update Q0 execution time.
		iter = q;
		continue;
	  }
	  else if(iter->p->state == RUNNABLE)
	  {
		q = iter->next;
		temp = pop(Q0, iter);
		push_back(Q2, temp);
		iter->p->end = ticks;
		iter->p->q0 += (int)(iter->p->end - iter->p->start);	// update Q0 execution time.
		iter->p->start = ticks;				// Q2 execution time starts.
		iter = q;
		continue;
	  }
	  iter = iter->next;
	}

	// update Q1
	iter = Q1->next;
	while(iter != 0)
	{
	  // move zombie or sleeping processes to Q0.
	  if(iter->p->state == ZOMBIE || iter->p->state == SLEEPING)
	  {
		q = iter->next;
		temp = pop(Q1, iter);
		push_back(Q0, temp);
		iter->p->end = ticks;
		iter->p->q1 += (int)(iter->p->end - iter->p->start);	// update Q1 execution time.
		iter->p->start = ticks;				// Q0 execution time starts.
		iter = q;
		continue;
	  }
	  // if the process is terminated, evict it from MLFQ.
	  else if(iter->p->state == UNUSED)
	  {
		q = iter->next;
		pop(Q1, iter);
		iter->p->end = ticks;
		iter->p->q1 += (int)(iter->p->end - iter->p->start);	// update Q1 execution time.
		iter = q;
		continue;
	  }
	  // if RUNNABLE, check if the process can proceed to Q2.
	  else if(iter->p->state == RUNNABLE)
	  {
		if(iter->p->is_sys == 1 || iter->p->is_yield != 1)
		{
		  q = iter->next;
		  iter->p->is_sys = 0;
		  iter->p->is_yield = 0;
		  temp = pop(Q1, iter);
		  push_back(Q2, temp);
		  iter->p->end = ticks;
		  iter->p->q1 += (int)(iter->p->end - iter->p->start);	// update Q1 execution time.
		  iter->p->start = ticks;				// Q2 execution time starts.
		  iter = q;
		  continue;
		}
		else
		{
		  iter->p->is_yield = 0;
		}
	  }
	  iter = iter->next;
	}

	// update Q2
	iter = Q2->next;
	while(iter != 0)
	{
	  // move zombie or sleeping processes to Q0.
	  if(iter->p->state == ZOMBIE || iter->p->state == SLEEPING)
	  {
		q = iter->next;
		temp = pop(Q2, iter);
		push_back(Q0, temp);
		iter->p->end = ticks;
		iter->p->q2 += (int)(iter->p->end - iter->p->start);	// update Q2 execution time.
		iter->p->start = ticks;					// Q0 execution time starts.
		iter = q;
		continue;
	  }
	  // if the process is terminated, evict it from MLFQ.
	  else if(iter->p->state == UNUSED)
	  {
		q = iter->next;
		pop(Q2, iter);
		iter->p->end = ticks;
		iter->p->q2 += (int)(iter->p->end - iter->p->start);	// update Q2 execution time.
		iter = q;
		continue;
	  }
	  // if RUNNABLE, check if the process can remain in Q2.
	  else if(iter->p->state == RUNNABLE)
	  {
		if(iter->p->is_sys != 1 && iter->p->is_yield == 1)
		{
		  q = iter->next;
		  iter->p->is_yield = 0;
		  temp = pop(Q2, iter);
		  push_back(Q1, temp);
	      iter->p->end = ticks;
		  iter->p->q2 += (int)(iter->p->end - iter->p->start);	// update Q2 execution time.
		  iter->p->start = ticks;					// Q1 execution time starts.
		  iter = q;
		  continue;
		}
		else
		{
		  iter->p->is_sys = 0;
		  iter->p->is_yield = 0;
		}
	  }
	  iter = iter->next;
	}

	// run processes in Q2
	if(Q2->next == 0 && current_queue == 2)	// if no process in Q2.
	{
	  current_queue = 1;
	}
	else if(current_queue == 2)
	{
	  // if first time running Q2.
	  if(current_pos_2 == Q2)
		current_pos_2 = Q2->next;

	  if(current_pos_2 == 0)
	  {
		current_queue = 1;
		current_pos_2 = Q2->next;
	  }
	  else
	  {
        // Switch to chosen process.  It is the process's job
        // to release its lock and then reacquire it
        // before jumping back to us.
		acquire(&(current_pos_2->p)->lock);
		current_pos_2->p->state = RUNNING;
		c->proc = current_pos_2->p;
		swtch(&c->context, &(current_pos_2->p)->context);
		release(&(current_pos_2->p)->lock);

	    // Process is done running for now.
        // It should have changed its p->state before coming back.
		c->proc = 0;
		current_pos_2 = current_pos_2->next;
	  }
	}	

	// run processes in Q1
	if(Q1->next == 0 && current_queue == 1)	// if no process in Q1.
	{
	  current_queue = 2;
	}
	else if(current_queue == 1)
	{
	  // if first time running Q1.
	  if(current_pos_1 == Q1)
		current_pos_1 = Q1->next;

	  if(current_pos_1 == 0)
	  {
		current_pos_1 = Q1->next;
	  }
	  acquire(&(current_pos_1->p)->lock);
	  current_pos_1->p->state = RUNNING;
	  c->proc = current_pos_1->p;
	  swtch(&c->context, &(current_pos_1->p)->context);
	  release(&(current_pos_1->p)->lock);

	  c->proc = 0;
	  current_pos_1 = current_pos_1->next;
	  current_queue = 2;
	}
  }

  ///////////////////////////////////////////////////////////////////////////////////

}

// Switch to scheduler.  Must hold only p->lock
// and have changed proc->state. Saves and restores
// intena because intena is a property of this
// kernel thread, not this CPU. It should
// be proc->intena and proc->noff, but that would
// break in the few places where a lock is held but
// there's no process.
void
sched(void)
{
  int intena;
  struct proc *p = myproc();

  if(!holding(&p->lock))
    panic("sched p->lock");
  if(mycpu()->noff != 1)
    panic("sched locks");
  if(p->state == RUNNING)
    panic("sched running");
  if(intr_get())
    panic("sched interruptible");

  intena = mycpu()->intena;
  swtch(&p->context, &mycpu()->context);
  mycpu()->intena = intena;
}

// Give up the CPU for one scheduling round.
void
yield(void)
{
  struct proc *p = myproc();
  acquire(&p->lock);
  p->state = RUNNABLE;

  p->is_yield = 1;	// changed part(update is_yield)

  sched();
  release(&p->lock);
}

// A fork child's very first scheduling by scheduler()
// will swtch to forkret.
void
forkret(void)
{
  static int first = 1;

  // Still holding p->lock from scheduler.
  release(&myproc()->lock);

  if (first) {
    // File system initialization must be run in the context of a
    // regular process (e.g., because it calls sleep), and thus cannot
    // be run from main().
    first = 0;
    fsinit(ROOTDEV);
  }

  usertrapret();
}

// Atomically release lock and sleep on chan.
// Reacquires lock when awakened.
void
sleep(void *chan, struct spinlock *lk)
{
  struct proc *p = myproc();
  
  // Must acquire p->lock in order to
  // change p->state and then call sched.
  // Once we hold p->lock, we can be
  // guaranteed that we won't miss any wakeup
  // (wakeup locks p->lock),
  // so it's okay to release lk.
  if(lk != &p->lock){  //DOC: sleeplock0
    acquire(&p->lock);  //DOC: sleeplock1
    release(lk);
  }

  // Go to sleep.
  p->chan = chan;
  p->state = SLEEPING;

  sched();

  // Tidy up.
  p->chan = 0;

  // Reacquire original lock.
  if(lk != &p->lock){
    release(&p->lock);
    acquire(lk);
  }
}

// Wake up all processes sleeping on chan.
// Must be called without any p->lock.
void
wakeup(void *chan)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++) {
    acquire(&p->lock);
    if(p->state == SLEEPING && p->chan == chan) {
      p->state = RUNNABLE;
    }
    release(&p->lock);
  }
}

// Wake up p if it is sleeping in wait(); used by exit().
// Caller must hold p->lock.
static void
wakeup1(struct proc *p)
{
  if(!holding(&p->lock))
    panic("wakeup1");
  if(p->chan == p && p->state == SLEEPING) {
    p->state = RUNNABLE;
  }
}

// Kill the process with the given pid.
// The victim won't exit until it tries to return
// to user space (see usertrap() in trap.c).
int
kill(int pid)
{
  struct proc *p;

  for(p = proc; p < &proc[NPROC]; p++){
    acquire(&p->lock);
    if(p->pid == pid){
      p->killed = 1;
      if(p->state == SLEEPING){
        // Wake process from sleep().
        p->state = RUNNABLE;
      }
      release(&p->lock);
      return 0;
    }
    release(&p->lock);
  }
  return -1;
}

// Copy to either a user address, or kernel address,
// depending on usr_dst.
// Returns 0 on success, -1 on error.
int
either_copyout(int user_dst, uint64 dst, void *src, uint64 len)
{
  struct proc *p = myproc();
  if(user_dst){
    return copyout(p->pagetable, dst, src, len);
  } else {
    memmove((char *)dst, src, len);
    return 0;
  }
}

// Copy from either a user address, or kernel address,
// depending on usr_src.
// Returns 0 on success, -1 on error.
int
either_copyin(void *dst, int user_src, uint64 src, uint64 len)
{
  struct proc *p = myproc();
  if(user_src){
    return copyin(p->pagetable, dst, src, len);
  } else {
    memmove(dst, (char*)src, len);
    return 0;
  }
}

// Print a process listing to console.  For debugging.
// Runs when user types ^P on console.
// No lock to avoid wedging a stuck machine further.
void
procdump(void)
{
  static char *states[] = {
  [UNUSED]    "unused",
  [SLEEPING]  "sleep ",
  [RUNNABLE]  "runble",
  [RUNNING]   "run   ",
  [ZOMBIE]    "zombie"
  };
  struct proc *p;
  char *state;

  printf("\n");
  for(p = proc; p < &proc[NPROC]; p++){
    if(p->state == UNUSED)
      continue;
    if(p->state >= 0 && p->state < NELEM(states) && states[p->state])
      state = states[p->state];
    else
      state = "???";
    printf("%d %s %s", p->pid, state, p->name);
    printf("\n");
  }
}
