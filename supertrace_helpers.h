#ifndef SUPERTRACE_HELPERS_H
#define SUPERTRACE_HELPERS_H

#include <iostream>
#include <vector>
#include "traces.h"

long long int findPercentileTime(double percentile, std::vector<long long int> frontier, std::vector<trace> t, int direction, std::vector<long long int> next_available_time);
int isPacketInDirection(std::vector<long long int> frontier, std::vector<trace> t, int direction);
int findMajorityDirection(std::vector<long long int> frontier, std::vector<trace> t, long long int thresh_time, std::vector<long long int> next_available_time);
int findMajorityLengthWtdDirection(std::vector<long long int> frontier, std::vector<trace> t, long long int thresh_time, std::vector<long long int> next_available_time);
int findMajorityByteWtdDirection(std::vector<long long int> frontier, std::vector<trace> t, long long int thresh_time, std::vector<long long int> next_available_time);
long long int findMaxSize(std::vector<long long int> frontier, std::vector<trace> t, int direction, long long int thresh_time, std::vector<long long int> next_available_time);
long long int findMinSize(std::vector<long long int> frontier, std::vector<trace> t, int direction, long long int thresh_time, std::vector<long long int> next_available_time);
long long int findPacketTime(std::vector<long long int> frontier, std::vector<trace> t, int direction, double time_multiplier, long long int median_time, std::vector<long long int> next_available_time);
long long int countCompleteTraces(std::vector<int> complete_flags);
std::vector<int> whichTracesComplete(std::vector<long long int> frontier, std::vector<trace> t);

#endif
