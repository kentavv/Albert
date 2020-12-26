//
// Created by kent on 12/26/2020.
// kent.vandervelden@gmail.com
//

#include <vector>
#include <sys/time.h>

#ifdef __linux__
#include <proc/readproc.h>
#endif

using std::vector;
using std::fill;
using std::pair;
using std::make_pair;

size_t memory_usage0;
double t0;
vector<pair<double, size_t> > memory_usage;

size_t current_memory_usage() {
#ifdef __linux__
    proc_t usage;
    look_up_our_self(&usage);
    return usage.vsize;
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
