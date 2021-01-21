#!/bin/bash
#########################################################
# EEE3535 Operating Systems                             #
# Written by William J. Song                            #
# School of Electrical Engineering, Yonsei University   #
#########################################################

patch_tar="syscall.tar"
patch_path="https://icsl.yonsei.ac.kr/wp-content/uploads"

if [[ -d kernel ]]; then
    if [[ ! -f user/syscalltest.c ]]; then
        make clean
        rm -rf user Makefile tar.sh
        wget $patch_path/$patch_tar
        tar xf $patch_tar
        rm -f $patch_tar
    else
        echo "xv6-riscv is already up to date"
    fi
else
    echo "Error: $0 must run in the xv6-riscv/ directory"
fi

