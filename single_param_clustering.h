#ifndef SINGLE_PARAM_CLUSTERING_H
#define SINGLE_PARAM_CLUSTERING_H

#include <iostream>
#include <vector>

std::vector<long long int> single_param_clustering(std::vector<long long int> parameters, long long int buckets);
long long int OPT(std::vector<long long int> parameters, long long int buckets, long long int **T, int uniform_flag);

#endif
