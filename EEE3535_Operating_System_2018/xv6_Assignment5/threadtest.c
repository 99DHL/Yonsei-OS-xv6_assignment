/* threadtest.c */

/*************************************************
 * EEE3535-02 Fall 2018                          *
 * School of Electrical & Electronic Engineering *
 * Yonsei University, Seoul, South Korea         *
 *************************************************/

#include "syscall.h"
#include "types.h"
#include "user.h"
#include "thread.h"
 
mutex_t lock;   // Lock variable
int count = -5; // Shared variable

// Thread function
void func(void *arg) {
  int tid = *(int*)arg;
  int pid = getpid();

  mutex_lock(&lock);
  count += 2*tid;
  printf(1, "Thread %d of process %d done\n", tid, pid);
  mutex_unlock(&lock);

  exit();
}

// Main thread
int main(int argc, char *argv[]) {
  if(argc != 2) {
    printf(1, "Usage: %s [num_threads]\n", argv[0]);
    exit();
  }

  // Malloc thread and ID arrays.
  int num_threads = atoi(argv[1]);
  thread_t *threads = (thread_t*)malloc(num_threads * sizeof(thread_t));
  int *tid = (int*)malloc(num_threads * sizeof(int));
 
  // Initialize lock.
  mutex_init(&lock);

  // Launch threads.
  for(int t = 0; t < num_threads; t++) {
    tid[t] = t;
    if(thread_create(&threads[t], &func, &tid[t]) != 0) {
      printf(1, "thread_create failed\n");
      exit();
    }
  }

  // Wait for threads to complete.
  for(int t = 0; t < num_threads; t++) {
    if(thread_join(threads[t]) != 0) {
      printf(1, "thread_join failed\n");
      exit();
    }
  }

  printf(1, "Main thread done: count = %d\n", count); 

  free(threads);
  free(tid);

  exit();
}

