#!/bin/bash

# This script builds Albert three times to improve Albert's performance
# by monitoring its behavior using test commands. This script uses compiler
# inserted instrumentation. A second method, profiled_make_perf.sh, uses
# performance counters and is much faster. The performance counter method
# would seem preferred since the program is not disturbed by instrumentation,
# however in practice, better performance improvements have come from using
# instrumentation. Also, the performance counter method requires elevated
# permissions and potential customization for a given processor model. For
# the performance counter version, perhaps a larger problem for Albert is
# required, increasing profiling time close to the instrumented version. The
# three builds done by this script are to collect timing for a normal build,
# profiling information, and timing for a profile driven build. The
# performance counter method merges the first two steps into one. The use of
# -fprofile-update=atomic seems reasonable since Albert uses OpenMP. This option
# doubles the time required to collect profiling information. Previously,
# -fprofile-correction was used. Perhaps a different combination is more correct.
#
# References
# https://gcc.gnu.org/wiki/AutoFDO
# https://gcc.gnu.org/wiki/AutoFDO/Tutorial
# https://research.google/pubs/pub36576/
# https://github.com/google/autofdo

set -e

TIME=/usr/bin/time
CFLAGS="-w -O3 -fopenmp -mavx"
LDFLAGS="-fopenmp -mavx"
ALBERT_TEST=tests/profile.in

echo "Testing standard build..."
make clean > /dev/null
make CFLAGS="$CFLAGS" LDFLAGS="$LDFLAGS" > /dev/null

a=$($TIME ./albert 2>&1 < $ALBERT_TEST)
rm basis.txt mult.txt mult.bin

echo "Generating profile..."
rm -f *.gcda
make clean > /dev/null
#make CFLAGS="$CFLAGS -fprofile-correction -fprofile-generate" LDFLAGS="$LDFLAGS -fprofile-correction -fprofile-generate" > /dev/null
make CFLAGS="$CFLAGS -fprofile-generate -fprofile-update=atomic" LDFLAGS="$LDFLAGS -fprofile-generate -fprofile-update=atomic" > /dev/null

b=$($TIME ./albert 2>&1 < $ALBERT_TEST)
rm basis.txt mult.txt mult.bin

echo "Testing profiled build..."
make clean > /dev/null
#make CFLAGS="$CFLAGS -fprofile-correction -fprofile-use" LDFLAGS="$LDFLAGS -fprofile-correction -fprofile-use" > /dev/null
make CFLAGS="$CFLAGS -fprofile-use" LDFLAGS="$LDFLAGS -fprofile-use" > /dev/null
rm -f *.gcda

c=$($TIME ./albert 2>&1 < $ALBERT_TEST)
rm basis.txt mult.txt mult.bin

echo -e "\nStandard build:"
echo "$a" | tail -2
echo -e "\nProfiling build:"
echo "$b" | tail -2
echo -e "\nProfiled build:"
echo "$c" | tail -2
