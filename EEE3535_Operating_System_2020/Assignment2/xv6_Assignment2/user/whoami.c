#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "user/sid.h"

int main(int argc, char **argv) {
    printf("Student ID:   %d\n", sid);
    printf("Student name: %s\n", sname);
    exit(0);
}

