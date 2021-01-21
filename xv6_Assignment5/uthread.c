/* uthread.c */

/*************************************************
 * EEE3535-02 Fall 2018                          *
 * School of Electrical & Electronic Engineering *
 * Yonsei University, Seoul, South Korea         *
 *************************************************/

#include "syscall.h"
#include "types.h"
#include "user.h"
#include "x86.h"
#include "thread.h"


// Thread create
int thread_create(thread_t *thread, void(*func)(void*), void *arg) {

	//////////////////// changed part ////////////////////////
	if(!threadcreate(thread, func, arg))
	{
		thread->tid = *((int*)arg);
		return 0;
	}
	else
	{
		return -1;
	}
	//////////////////////////////////////////////////////////

}

// Thread join
int thread_join(thread_t thread) {
	//////////////////// changed part ////////////////////////
	if(!threadjoin(&thread))
	{
		return 0;
	}
	else
	{
		return -1;
	}
	//////////////////////////////////////////////////////////
}
// Mutex init
void mutex_init(mutex_t *lock) {
	lock->flag = 0;  // initialize flag.(changed part)
}

// Mutex lock
void mutex_lock(mutex_t *lock) {
	//////////////////// changed part ////////////////////////
	while(xchg((volatile uint *)(&lock->flag), 1) == 1)
	{
		continue;	// spin lock does nothing.
	}
	//////////////////////////////////////////////////////////
}

// Mutex unlock
void mutex_unlock(mutex_t *lock) {
	lock->flag = 0;	// release the lock.(changed part)
}



