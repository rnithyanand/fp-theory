#include <iostream>
#include <stdlib.h> 
#include <vector>
#include <algorithm>

#include <fstream>
#include <sstream>

#define INIT_MAX   1000000000000
#define INFEAS_MAX 1000000000001
using namespace std;

long long int findmin(long long int *array, int size)
{
	long long int min = INIT_MAX;
	for(long long int l = 0 ; l < size ; l ++)
		if(min > array[l])	
			min = array[l];
	return min;
}

long long int OPT(int i, int j, vector<long long int> params, int k, long long int **M, int uniform_flag)
{
	if(i == 0) return 0;
	if(j == 0) return INFEAS_MAX;
	if((M[i][j] != INIT_MAX)) return M[i][j];

	long long int *minlist, cost, b_size;
	if(uniform_flag == 0)
		b_size = 1;
	else 
		b_size = k;
	minlist = new long long int[i];
	for(long long int l = 0 ; l < i ; l ++)
	{
		if((i-l)>=b_size)
			cost = (long long int) (i-l)*params[i-1];
		else
			cost = (long long int) INFEAS_MAX;
		if((M[l][j-1] != INIT_MAX) && (M[l][j-1] != INFEAS_MAX))
			minlist[l] = (long long int)(cost + M[l][j-1]);
		else if(M[l][j-1] == INFEAS_MAX)
			minlist[l] = (long long int) INFEAS_MAX;
		else
			minlist[l] = (long long int)(cost + OPT(l, j-1, params, k, M, uniform_flag));
	}
	M[i][j] = (long long int) findmin(minlist, i);
	delete[] minlist;
	return M[i][j];
}

void initialize_matrix(long long int **array, long long int rows, long long int cols, long long int val)
{
	for(long long int i = 0 ; i < rows ; i ++)
		for(long long int j = 0 ; j < cols ; j ++)
			array[i][j] = val;
}

void solver(long long int n, long long int m, vector<long long int> params, long long int k, long long int **M, long long int* S)
{
	S[0] = 0;
	S[m] = n;
	for(long long int i = m ; i >= 0 ; i --)
		for(long long int j = n ; j >= i ; j --)
			if(M[S[i]][i] == (M[j][i-1] + (S[i] - j)*params[S[i]-1]))
				S[i-1] = j;
}

int main(int argc, char *argv[])
{
	ofstream clusters, log;
	log.open("log_clusters.txt", ios::app|ios::out);

	long long int n, k, buckets;
	int uniform_flag = 0;
	n = atol(argv[1]);
	k = atol(argv[2]);
	uniform_flag = atoi(argv[3]);
	buckets = n/k;
	long long int sum = 0;
	vector<long long int> parameter;
	for(long long int i = 0 ; i < n ; i ++)
	{
		parameter.push_back(atol(argv[4+i]));
		sum += atol(argv[4+i]);
	}
	sort(parameter.begin(), parameter.end());	

	long long int **M = new long long int*[n+1];
	for(long long int i = 0 ; i < n + 1 ; i ++)
		M[i] = new long long int[buckets+1];
	
	initialize_matrix(M, n+1, buckets+1, INIT_MAX);
	long long int opt;
	double opt_ratio;
	opt = OPT(n, buckets, parameter, k, M, uniform_flag);
	opt_ratio = (double)opt/(double)sum;

	log<<"Uniform Flag: "<<uniform_flag<<", K: "<<k<<", OR: "<<opt_ratio<<", OPT: "<<opt<<endl;

	long long int *solution = new long long int[buckets + 1];
	solver(n, buckets, parameter, k, M, solution);
	log<<"Partition: ";
	stringstream cluster;
	string fname = "";
	for(long long int i = 1 ; i < buckets + 1 ; i ++)
	{
		cluster<<"./clusters/"<<uniform_flag<<"_"<<k<<"_cluster_"<<i<<".cluster";
		fname = cluster.str();
		clusters.open(fname.c_str(), ios::app|ios::out);
		for(long long int s = solution[i-1] + 1 ; s <= solution[i] ; s ++)
			clusters<<", "<<s;
		cluster.str("");
		clusters.close();
		log<<", "<<solution[i];
		
	}
	log<<endl;

	return 0;
}
