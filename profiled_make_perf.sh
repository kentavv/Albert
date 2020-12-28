#!/bin/bash

# See the discussion in profiled_make.sh for more information.

set -e

TIME=/usr/bin/time
CFLAGS="-w -g -O3 -fopenmp -mavx"
LDFLAGS="-fopenmp -mavx"
ALBERT_TEST=tests/profile.in

echo "Testing standard build..."
make clean > /dev/null
make CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" > /dev/null

a=$($TIME ./albert 2>&1 < $ALBERT_TEST)

echo "Generating profile..."
#b=$($TIME ./albert 2>&1 < $ALBERT_TEST)
# this works on Intel, but not AMD. Each processor may require its own -e options
#sudo time perf record  -e br_inst_retired:near_taken -b -o perf.data ./albert 2>&1 < $ALBERT_TEST
sudo time perf record  -e cpu/event=0xc4,umask=0x0,name=br_inst_retired.taken/ -o perf.data ./albert 2>&1 < $ALBERT_TEST
b="n/a"
rm basis.txt mult.txt mult.bin
create_gcov --binary=./albert --profile=perf.data -gcov_version 1 -use_lbr=false

echo "Testing profiled build..."
make clean > /dev/null
make CFLAGS="$CFLAGS -fauto-profile" LDFLAGS="$LDFLAGS -fprofile-use" > /dev/null
rm -f *.gcda

c=$($TIME ./albert 2>&1 < $ALBERT_TEST)
rm basis.txt mult.txt mult.bin
rm -f rm perf.ado perf.ado.imports  perf.data perf.data.old fbdata.afdo fbdata.afdo.imports

echo -e "\nStandard build:"
echo "$a" | tail -2
echo -e "\nProfiling build:"
echo "$b" | tail -2
echo -e "\nProfiled build:"
echo "$c" | tail -2
