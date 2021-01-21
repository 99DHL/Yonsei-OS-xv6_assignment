/* malloctest.c */

#include "types.h"
#include "user.h"

#define num_ptr     10
#define num_loop    30
#define max_malloc  1024

int main() {
    int* array_ptr[num_ptr];    // Array of malloc pointers
    int array_sz[num_ptr];      // Array of malloc-created array sizes

    // Initialize the arrays.
    memset(array_ptr, 0, num_ptr * sizeof(int*));
    memset(array_sz, 0, num_ptr * sizeof(int));

    // Test runs for num_loop times:
    // Case 1: If an array does not exist at random index i, create one with
    //         an arbitrary size between 1 and max_malloc.
    // Case 2: If an array exists at random index i, free it.
    printf(1, "*** TEST RUNS ***\n");
    for(unsigned m = 0; m < num_loop; m++) {
        printf(1, "#%d: ", m+1);
        unsigned i = urand() % num_ptr; 
        if(array_ptr[i]) { // Array exists.
            printf(1, "free(%d bytes)\n", array_sz[i]);
            free(array_ptr[i]);
            array_ptr[i] = 0; array_sz[i] = 0;
        }
        else { // Array does not exist.
            unsigned sz = 0;
            while(!sz) { sz = urand() % max_malloc; }
            printf(1, "malloc(%d bytes)\n", sz);
            array_ptr[i] = malloc(sz); array_sz[i] = sz;
        }
    }

    // Final cleanup to avoid memory leaks.
    printf(1, "*** CLEANUP ***\n");
    for(unsigned i = 0; i < num_ptr; i++) {
        if(array_ptr[i]) {
            printf(1, "free(%d bytes)\n", array_sz[i]);
            free(array_ptr[i]);
            array_ptr[i] = 0; array_sz[i] = 0;
        }
    }

    exit();
}
