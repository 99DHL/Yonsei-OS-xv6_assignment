#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/sid.h"

int gvar = 3535;                                            // A global variable

// Print 3-level page table information.
void ptinfo(char *name, void *addr) {
    printf("%s: \n", name);                                 // Variable name
    printf("    Virtual address = %p\n", addr);             // Virtual address
    printf("    Physical address = %p\n", phyaddr(addr));   // Physical address
    for(int l = 2; l >= 0; l--) {                           // Page table index at each level
        printf("    PT index at level %d: %d\n", l, ptidx(addr, l));
    }
    printf("\n");
}

int main(int argc, char **argv) {
    int lvar = 2020;                                        // A local variable
    int *array = (int*)malloc(gvar*lvar*sizeof(int));       // Heap array
    
    for(int i = 0; i < gvar*lvar; i++) { array[i] = i; }    // Dummy loop

    // Print out the details of each code component.
    ptinfo("&array[N-1]", &array[gvar*lvar-1]);
    ptinfo("array", array);
    ptinfo("&lvar", &lvar);
    ptinfo("&array", &array);
    ptinfo("&gvar", &gvar);
    ptinfo("&main", &main);

    // Tell how many pages this program uses.
    printf("Total number of pages = %d\n", pgcnt());

    free(array);
    exit(0);
}

