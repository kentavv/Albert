//
// Created by kent on 12/26/2020.
// kent.vandervelden@gmail.com
//

#include <vector>
#include <sys/time.h>

#ifdef __linux__
#include <proc/readproc.h>
#include <malloc.h>
#endif

using std::vector;
using std::fill;
using std::pair;
using std::make_pair;

int memory_usage0;
double t0;
vector<pair<double, int> > memory_usage;

int current_memory_usage() {
#ifdef __linux__
#if 0
    proc_t usage;
    look_up_our_self(&usage);
    return usage.vsize;
#elif 1
    struct mallinfo m = mallinfo();
    //return m.arena;
    return m.uordblks;
#else
    char *p;
    size_t n;
    FILE *f = open_memstream(&p, &n);
    malloc_info(0, f);
    fclose(f);
    puts(p);
    return 0;
#endif
#else
    return 0;
#endif
}

double current_time() {
    timeval tv;
    gettimeofday(&tv, nullptr);
    return double(tv.tv_sec) + double(tv.tv_usec) / double(1e6);
}

void memory_usage_init(int n) {
    memory_usage.resize(n);
    fill(memory_usage.begin(), memory_usage.end(), make_pair(0, 0));
    t0 = current_time();
    memory_usage0 = current_memory_usage();
}

void memory_usage_update(int i) {
    memory_usage[i] = make_pair(current_time() - t0, current_memory_usage() - memory_usage0);
}
