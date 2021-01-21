/* pagingtest.c */

/*************************************************
 * EEE3535-02 Fall 2018                          *
 * School of Electrical & Electronic Engineering *
 * Yonsei University, Seoul, South Korea         *
 *************************************************/

#include "syscall.h"
#include "types.h"
#include "user.h"

#define SIZE 0x8000
#define DIST 0x1000

int main(int argc, char *argv[]) {
  // Integer array of "SIZE" number of elements
  int *array = (int*)malloc(sizeof(int) * SIZE);

  // Touch elements that are "DIST" apart.
  for(unsigned i = 0; i < SIZE; i += DIST) {
      array[i] = 0;
  }

  free(array);

  exit();
}

