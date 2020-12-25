#ifndef _PROFILE_H_
#define _PROFILE_H_

#include <string>
#include <stdio.h>
#include <time.h>

#ifndef WIN32
#include <sys/time.h>
#else
#include <sys/timeb.h>

typedef int suseconds_t;
struct timeval {
    time_t tv_sec;
    suseconds_t tv_usec;
};

inline int gettimeofday(struct timeval *tv, void * /*tz*/) {
  struct _timeb tb;
    
  _ftime(&tb);
  tv->tv_sec = tb.time;
  tv->tv_usec = tb.millitm * 1000;
    
  return 0;
}
#endif

struct Profile {
  Profile(const char *str) : rank(-1), name(str) {
    restart();
  }

  Profile(int rank_, const char *str) : rank(rank_), name(str) {
    restart();
  }

  ~Profile() {
    if(!stopped_) report();
  }

  void restart() {
    gettimeofday(&tv_start, NULL);
    stopped_ = false;
  }

  void stop() {
    stopped_ = true;
  }

  void setRank(int rank_) {
    rank = rank_;
  }

  void report() {
    struct timeval tv_end;
    gettimeofday(&tv_end, NULL);
    double dt = (tv_end.tv_sec + tv_end.tv_usec / 1e6) - (tv_start.tv_sec + tv_start.tv_usec / 1e6);
    if(rank < 0) {
      printf("%s: (profile) %f s\n", name.c_str(), dt);
      //qDebug("%s: (profile) %f s\n", name.c_str(), dt);
    } else {
      printf("<%d> %s: (profile) %f s\n", rank, name.c_str(), dt);
      //qDebug("<%d> %s: (profile) %f s\n", rank, name.c_str(), dt);
    }
  }

  int rank;
  std::string name;
  struct timeval tv_start;
  bool stopped_;

protected:
  Profile(const Profile &);
  Profile operator=(const Profile &);
};

#endif
