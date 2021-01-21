#!/bin/bash
#########################################################
# EEE3535 Operating Systems                             #
# Written by William J. Song                            #
# School of Electrical Engineering, Yonsei University   #
#########################################################

patch_tar="paging.tar"
patch_path="https://icsl.yonsei.ac.kr/wp-content/uploads"

if [[ -d kernel ]]; then
    if [[ ! -f user/pgtest.c ]]; then
        make clean
        mv user/sid.h ./
        rm -rf user kernel Makefile install.sh syscall.sh sched.sh
        wget $patch_path/$patch_tar
        tar xf $patch_tar
        rm -f $patch_tar
        mv sid.h user/
    else
        echo "xv6-riscv is already up to date"
    fi
    if [[ `grep sname user/sid.h` =~ Unknown ]]; then
        read -p "Enter your 10-digit student ID: " sid
        read -p "Enter your name in English: " sname
        echo "#define sid $sid" > user/sid.h;
        echo "#define sname \"$sname\"" >> user/sid.h;
    fi
else
    echo "Error: $0 must run in the xv6-riscv/ directory"
fi

