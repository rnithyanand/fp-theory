#ifndef SUPERTRACE_FUNC_H
#define SUPERTRACE_FUNC_H

#include <iostream>
#include <vector>

#include "supertrace_helpers.h"

//trace frontierMax_test(long long int threshold, std::vector<trace> t, double time_multiplier);

trace frontierMax(long long int threshold, std::vector<trace> t, double time_multiplier);
trace frontierMaxPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMaxPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMaxPT_DOWN(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMin(long long int threshold, std::vector<trace> t, double time_multiplier);
trace frontierMinPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMinPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMinPT_DOWN(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);

trace trLenWtdMin(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trLenWtdMinPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMinPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMinPT_DOWN(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMax(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trLenWtdMaxPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMaxPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMaxPT_DOWN(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);

trace trByteWtMin(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trByteWtMinPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMinPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMinPT_DOWN(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMax(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trByteWtMaxPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMaxPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMaxPT_DOWN(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
#endif
