#!/bin/bash

#########################################################
# EEE3535 Operating Systems                             #
# Written by William J. Song                            #
# School of Electrical Engineering, Yonsei University   #
#########################################################

sid=$(grep -w sid user/sid.h | cut -d' ' -f3)
make clean > /dev/null;
rm -f $sid.tar;
cd ..;
tar cf $sid.tar xv6-riscv;
mv $sid.tar xv6-riscv/

