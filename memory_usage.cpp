//
// Created by kent on 12/26/2020.
// kent.vandervelden@gmail.com
//

#include <vector>
#include <sys/time.h>

#ifdef __linux__
#include <proc/readproc.h>
#include <malloc.h>
#include <string.h>
#endif

using std::vector;
using std::fill;
using std::pair;
using std::make_pair;

int memory_usage0;
double t0;
vector<pair<double, int> > memory_usage;

size_t current_memory_usage() {
#ifdef __linux__
#if 0
    // Probably the most conservative, but also the slowest to respond when
    // new memory is allocated in increasingly large chunks.
    proc_t usage;
    look_up_our_self(&usage);
    return usage.vsize;
#elif 0
    // 32-bit only and only reports main arena.
    struct mallinfo m = mallinfo();
    //return m.arena;
    return m.uordblks;
#elif 0
    // May work, similar response as readproc method, but all arenas accessible.
    char *p;
    size_t n;
    FILE *f = open_memstream(&p, &n);
    malloc_info(0, f);
    fclose(f);
    puts(p);
    return 0;
#elif 1
    // This method may be only good to 32-bit, but includes all arenas.
    char *p;
    size_t n;
    FILE *f = stderr;
    stderr = open_memstream(&p, &n);
    malloc_stats();
    fclose(stderr);
    stderr = f;

    bool ready = false;
    long system = -1, in_use = -1;
    for (char *pch = strtok (p, "\n"); pch != nullptr; pch = strtok (nullptr, "\n")) {
        if (!ready) {
            ready = !ready && strstr(pch, "Total ") != nullptr;
        } else if (ready) {
           if(strstr(pch, "system bytes") != nullptr) {
               sscanf(pch, "system bytes     = %ld", &system);
           } else if(strstr(pch, "in use bytes") != nullptr) {
               sscanf(pch, "in use bytes     = %ld", &in_use);
           }
           if(system != -1 && in_use != -1) break;
        }
    }
    return in_use;
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
    // Trim now to reduce chance that automatic trimming of earlier allocations will occur, and 
    // then reported memory becoming negative.
    malloc_trim(0);
    memory_usage.resize(n);
    fill(memory_usage.begin(), memory_usage.end(), make_pair(0, 0));
    t0 = current_time();
    memory_usage0 = current_memory_usage();
}

void memory_usage_update(int i) {
    printf("memory: %ld\n", current_memory_usage());
    memory_usage[i] = make_pair(current_time() - t0, current_memory_usage() - memory_usage0);
}
