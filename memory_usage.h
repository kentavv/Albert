//
// Created by kent on 12/26/2020.
// kent.vandervelden@gmail.com
//

#ifndef ALBERT_MEMORY_USAGE_H
#define ALBERT_MEMORY_USAGE_H

#include <vector>

extern std::vector<std::pair<double, size_t> > memory_usage;

void memory_usage_init(int n);

void memory_usage_update(int i);

#endif //ALBERT_MEMORY_USAGE_H
