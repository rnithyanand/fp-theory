#ifndef SUPERTRACE_FUNC_H
#define SUPERTRACE_FUNC_H

#include <iostream>
#include <vector>

#include "supertrace_helpers.h"

trace frontierMax(long long int threshold, std::vector<trace> t, double time_multiplier);
trace frontierMaxPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMaxTT(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);
trace frontierMaxPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMaxTT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);

trace frontierMin(long long int threshold, std::vector<trace> t, double time_multiplier);
trace frontierMinPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMinTT(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);
trace frontierMinPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace frontierMinTT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);

trace trLenWtdMin(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trLenWtdMinPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMinTT(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);
trace trLenWtdMinPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMinTT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);

trace trByteWtMin(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trByteWtMinPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMinTT(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);
trace trByteWtMinPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMinTT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);

trace trLenWtdMax(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trLenWtdMaxPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMaxTT(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);
trace trLenWtdMaxPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trLenWtdMaxTT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);

trace trByteWtMax(long long int threshold, std::vector<trace> t, double time_multiplier);
trace trByteWtMaxPT(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMaxTT(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);
trace trByteWtMaxPT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int p_thresh);
trace trByteWtMaxTT_UP(long long int threshold, std::vector<trace> t, double time_multiplier, int t_thresh);


#endif
