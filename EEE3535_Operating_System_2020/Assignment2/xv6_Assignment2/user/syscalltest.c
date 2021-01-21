#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/sid.h"

int main(int argc, char **argv) {
    // Get the process ID.
    printf("pid = %d\n", getpid());

    // Assignment 2: System Call

    // Get the process ID of parent process.
    printf("ppid = %d\n", getppid());

    // Get the process name.
    char name[16];
    printf("name = %s\n", getpname(name) < 0 ? 0 : name);

    exit(0);
}

