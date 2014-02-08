#include <set>

#include "pair_present.h"

using namespace std;

set<pair<int, int> > pp;

void pp_clear(void) {
  pp.clear();
}

void pp_set(int r, int c) {
  pp.insert(make_pair(r, c));
}

int pp_contains(int r, int c) {
  return pp.find(make_pair(r, c)) != pp.end();
}

int pp_count(void) {
  return pp.size();
}
