#include <pthread.h>
#include <stdlib.h>
#include "mandelbrot.h"

// Mandelbrot convergence map
unsigned mandelbrot[resolution*resolution];

// flag variable that checks if all threads are done
int flag;

// Lock and cond variables for thread_exit() and thread_join()
pthread_mutex_t lock;
pthread_cond_t  cond;

// This must be called at the end of thread function
void thread_exit(void) {
	pthread_mutex_lock(&lock);
	flag--;
	pthread_cond_signal(&cond);
	pthread_mutex_unlock(&lock);
}

// This is called by the main thread.
void thread_join(void) {
	pthread_mutex_lock(&lock);
	while(flag > 0)
		pthread_cond_wait(&cond, &lock);
	
	pthread_mutex_unlock(&lock);
}

// Thread function to calculate a part of Mandelbrot set.
void* thread_mandelbrot(void* arg) {
	int startpos = *(int*)arg;
	int jobnum;				// number of jobs allocated
	int w,h, iteration;		// width, height and iteration

	// if last thread
	if( startpos == (int)((num_threads-1)*((resolution*resolution)/(num_threads))) )
	{
		jobnum = (resolution*resolution) - startpos;	// rest jobs are done by last thread
	}
	else
	{
		jobnum = ((resolution*resolution)/(num_threads));
	}

	for(; jobnum > 0; jobnum--)
	{
		iteration = 0;
		w = startpos % resolution;	// width
		h = startpos / resolution;	// height

		// generate two complex numbers, c and Z
		Complex c(-2.2 + (3.2*w)/resolution, -1.5 + (3.2*h)/resolution);
		Complex Z(-2.2 + (3.2*w)/resolution, -1.5 + (3.2*h)/resolution);

		while(Z.magnitude2() <= escape)
		{
			Z = Z*Z + c;	// iterate
			iteration++;
			if(iteration >= max_iterations)
				break;
		}

		mandelbrot[startpos] = iteration;	// store final value
		startpos++;
	}
	
    thread_exit();  // Wake up a thread waiting on the condition variable.
    return 0;
}

// Calculate the Mandelbrot convergence map.
void calc_mandelbrot(void) {
	// initialize mutex variables
	pthread_mutex_init(&lock, NULL);
	pthread_cond_init(&cond, NULL);

	flag = num_threads;		// initialize flag variable

	int i;
	pthread_t *thread = (pthread_t*)malloc(num_threads * sizeof(pthread_t));	// threads
	int *start_pos = (int*)malloc(num_threads * sizeof(int));	// thread's start position in array

	for(i = 0; i < (int)num_threads; i++) {
		start_pos[i] = i*((resolution * resolution)/(num_threads));
		pthread_create(&thread[i], 0, &thread_mandelbrot, (void*)&start_pos[i]);
	}

	thread_join();
	pthread_mutex_destroy(&lock);
	pthread_cond_destroy(&cond);
}


