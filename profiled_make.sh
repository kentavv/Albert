#!/bin/bash

set -e

make clean
make

TIME=/usr/bin/time

$TIME ./albert < tests/profile.in
rm basis.txt mult.txt mult.bin

CFLAGS="-g -O3 -Wall -fopenmp -fprofile-correction -mavx"
LDFLAGS="-fopenmp -fprofile-correction -mavx"

rm -f *.gcda
make clean
make CFLAGS="$CFLAGS -fprofile-generate" LDFLAGS="$LDFLAGS -fprofile-generate"

$TIME ./albert < tests/profile.in
rm basis.txt mult.txt mult.bin

make clean
make CFLAGS="$CFLAGS -fprofile-use" LDFLAGS="$LDFLAGS -fprofile-use"
rm -f *.gcda

$TIME ./albert < tests/profile.in
rm basis.txt mult.txt mult.bin

