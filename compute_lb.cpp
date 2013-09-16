#include <iostream>
#include <stdlib.h>
#include <time.h>

#include "single_param_clustering.h"


#define MAX 99999999
using namespace std;
//Helper functions
double findmin(double *list, int i)
{
        double min = MAX;
        for(int l = 0 ; l < i ; l ++)
                if(min > list[l])
                        min = list[l];
        return min;
}
int ceil(double n)
{       return (int)n+1;        }
int floor(double n)
{       return (int) n;         }
void initialize2d(double **array, int n, int m, int value)
{
        for(int i = 0 ; i < n ; i ++)
                for(int j = 0 ; j < m ; j++)
                        array[i][j] = value;
}
//Simple sort O(n**2)
void sort(double *list, int n)
{
        double *sorted, min = MAX;
        sorted = new double[n];
        int minindex = -11;
        for(int j = 0 ; j < n ; j++)
        {
                for(int i = 0 ; i < n ; i ++)
                        if(list[i] < min)
                        {
                                min = list[i];
                                minindex = i;
                        }
                sorted[j] = list[minindex];
                list[minindex] = MAX;
                min = MAX;
        }
        for(int i = 0 ; i < n ; i ++)
                list[i] = sorted[i];
        delete[] sorted;

}
//Compute OPT(i,j) = MIN(l = 0 to i-1) [COST(l,i) + OPT(l,j-1)]
//COST(l,i) = (i-l) * size[i-1]
double OPT(int i, int j, double *sizes, int k, double **M)
{
        if(i == 0)              return 0;
        else if(j==0)           return MAX;
        //else if(j==1 && i>=k)   return i*sizes[i-1];
        //else if(j==1 && i <k)   return MAX;
        if(M[i][j]!=MAX)        return M[i][j];
        double *minlist;
        double cost;
        minlist = new double[i];
        for (int l = 0; l < i ; l ++)
        {
                if((i-l)>=k)
                        cost = (double)(i-l)*sizes[i-1];
                else
                        cost = (double)MAX;
                if(M[l][j-1] != MAX)
                        minlist[l] = (double)(cost + M[l][j-1]);
                else
                        minlist[l] = (double)(cost + OPT(l, j-1, sizes,k, M));
        }
        M[i][j] = (double)(findmin(minlist,i));
        delete[] minlist;
        return M[i][j];
}
int main(int argc, char *argv[])
{
/*        
	double tstart, tinit, tsort, tfinal;
        tstart = clock();
        int n, k;
        n = atoi(argv[1]);
        k = atoi(argv[2]);
        double W = 0, **M, epsilon, opt;
        double *sizes = new double[n];
        M = new double*[n+1];
        int buckets;
        buckets = floor(n/k);
        epsilon = (double)1/k;
        for(int i = 0 ; i < n+1 ; i ++)
                M[i] = new double[buckets+1];
        initialize2d(M,n+1,buckets+1,MAX);
        for(int i = 0; i < n; i ++)
        {
                sizes[i] = atof(argv[i+3]);
                W += sizes[i];
        }
        tinit = (clock() - tstart)/CLOCKS_PER_SEC;
        sort(sizes, n);
        tsort = (clock() - tinit)/CLOCKS_PER_SEC;
        opt = OPT(n,buckets,sizes,k,M);
        cout<<"k: "<<k<<", e: "<<epsilon<<", MIN BO: ";
	cout<<fixed<<opt<<", RATIO: "<<opt/W<<endl;
        tfinal = (clock() - tsort)/CLOCKS_PER_SEC;
        for(int i = 0 ; i < n+1; i++)
                delete[] M[i];
        delete[] M;
        delete[] sizes;
        return 0;
*/
	long long int n, k;
	n = atol(argv[1]);
	k = atol(argv[2]);
	vector<long long int> sizes;
	for(long long int i = 3; i < n+3 ; i ++)
		sizes.push_back(atol(argv[i]));
	long long int buckets;
	buckets = n/k;
	cout<<"K :"<<k<<", N: "<<n<<", B: "<<buckets<<endl;
	vector<long long int> opt = single_param_clustering(sizes, buckets);
	cout<<"K : "<<k<<", N: "<<n<<", OPT_1: "<<opt[0]<<", OPT_2: "<<opt[1]<<endl;
}
