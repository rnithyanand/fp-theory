#include <iostream>
#include <stdlib.h>
#include <algorithm>
#include <limits.h>
#include <vector>

#define INIT_MAX LONG_MAX
#define INFEAS_MAX LONG_MAX-1

using namespace std;

long long int OPT(vector<long long int> parameters, long long int buckets, long long int **T, int uniform_flag)
{
	if(T[parameters.size()][buckets] != INIT_MAX)
		return T[parameters.size()][buckets];
	
	vector<long long int> min_list;
	long long int cost;
	for(int l = 0 ; l < parameters.size(), l++)
	{
		if(uniform_flag == 1 && parameters.size() - l >= parameters.size()/buckets)
			cost = (parameters.size() - l)*parameters[parameters.size() - 1];
		else if(uniform_flag == 0 && parameters.size() - l >= 1)
			cost = (parameters.size() - l)*parameters[parameters.size() - 1];
		else
			cost = INFEAS_MAX;
		if(T[l][buckets - 1] != INFEAS_MAX || T[l][buckets - 1] != INIT_MAX)
			minlist.push_back(cost + T[l][buckets - 1]);
		else
		{
			vector<long long int> temp;
			for(int x = 0 ; x < l ; x ++)
				temp.push_back(parameters[x]);
			minlist.push_back(cost + OPT(temp, buckets - 1, **T));
		}
	}
	sort(minlist.begin(), minlist.end());
	T[parameters.size()][buckets] = minlist[0];
	return T[parameters.size()][buckets];
}

void single_param_clustering(vector <long long int> parameters, long long int buckets, int uniform_security_flag)
{
	sort(parameters.begin(), parameters.end());
	/*
		Compute the running sums of parameters.
	*/
	vector<long long int> running_sum;
	long long int temp = 0;
	for(long long int i = 0 ; i < parameters.size() ; i ++)
	{
		temp += parameters[i];
		running_sum.push_back(temp);
	}

	/*
		Table: T[0...no_sites, 0...buckets]
		T[i,j] = Min OH using j buckets for clustering the first i sites
	*/
	long long int **T = new long long int*[parameters.size()+1];
	for(long long int i = 0 ; i < parameters.size() + 1 ; i ++)
	{
		T[i] = new long long int[buckets+1];
		for(long long int j = 0 ; j < buckets + 1 ; j ++)
			T[i][j] = INIT_MAX;
	}

	/*
		Set initial values for the table: 
		T[0,j] = 0, T[i, 0] = INFEAS, T[i,1] = Sum(param_1...param_i), T[1,j] = param_1.
	*/
	for(long long int i = 1 ; i < parameters.size() + 1 ; i ++)
	{	
		T[i][0] = INFEAS_MAX;
		T[i][1] = running_sum[i-1];
	}
	for(long long int j = 1 ; j < buckets + 1 ; j ++) 
	{
		T[0][j] = 0;
		T[1][j] = parameters[0];
	}
	
	long long int opt_uniform, opt_nonuniform;
	opt_uniform = OPT(parameters, buckets, T, 1);
	opt_nonuniform = OPT(parameters, buckets, T, 0);		
}
