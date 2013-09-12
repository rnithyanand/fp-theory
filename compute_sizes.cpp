#include <iostream>
#include <algorithm>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <limits.h>

#include "traces.h"
#include "file_ops.h"

#define INIT_MAX LONG_MAX
#define INFEAS_MAX LONG_MAX-1

using namespace std;

/*
	Reads the traces supplied in ./input_data/ and computes the following:
	1. Median total size for each site
	2. Median upstream and downstream size for each site
	3. Two lower bounds for bandwidth overhead for any schemes with "k"-uniform/non-uniform security
*/


int main(int argc, char *argv[])
{
	ofstream results, m_total, m_up, m_down;

	results.open("log_lbounds.txt", ios::app|ios::out);
	m_total.open("total_size_input.txt", ios::app|ios::out);
	m_up.open("up_size_input.txt", ios::app|ios::out);
	m_down.open("down_size_input.txt", ios::app|ios::out);

	if(!results.is_open())
		return -1;

	long long int no_sites = atoi(argv[1]);
	long long int no_trials = atoi(argv[2]);
	//long long int sec_param = atoi(argv[3]);
	int status = -1;
	
	stringstream bfile, tfile;	
	trace temp;
	vector<double> median_total;
	vector<double> median_up;
	vector<double> median_down;

	double running_total = 0;
	double running_up = 0;
	double running_down = 0;

	for(long long int i = 0 ; i < no_sites ; i ++)
	{
		vector<trace> tr;
		vector<double> total_bytes(no_trials);
		vector<double> up_bytes(no_trials);
		vector<double> down_bytes(no_trials);

		for(long long int j = 0 ; j < no_trials ; j ++)
		{
			bfile<<"./input_data/"<<i+1<<"_"<<j+1<<".cap.txt";
			tfile<<"./input_data/timeseq_"<<i+1<<"_"<<j+1<<".cap.txt";
			status = read_trace(bfile.str(), tfile.str(), &temp);
			tr.push_back(temp);
			total_bytes.push_back(tr[j].total_bytes/1000);
			up_bytes.push_back(tr[j].up_bytes/1000);
			down_bytes.push_back(tr[j].down_bytes/1000);
			bfile.str("");
			tfile.str("");
		}

		sort(total_bytes.begin(), total_bytes.end());
		sort(up_bytes.begin(), up_bytes.end());
		sort(down_bytes.begin(), down_bytes.end());

		running_total += total_bytes[total_bytes.size()/2];
		running_up += up_bytes[up_bytes.size()/2];
		running_down += down_bytes[down_bytes.size()/2];

		median_total.push_back(total_bytes[total_bytes.size()/2]);
		median_up.push_back(up_bytes[up_bytes.size()/2]);
		median_down.push_back(down_bytes[down_bytes.size()/2]);
	}

	sort(median_total.begin(), median_total.end());
	for(long long int i = 0 ; i < median_total.size() ; i ++)
	{
		results<<i<<", Total Size: "<<median_total[i]<<endl;
		m_total<<median_total[i]<<endl;
		m_up<<median_up[i]<<endl;
		m_down<<median_down[i]<<endl;
	}

	sort(median_up.begin(), median_up.end());
	sort(median_down.begin(), median_down.end());

	results<<"(All Sites) Total Bytes: 100 x "<<running_total<<", Up Bytes: 100 x "<<running_up<<", Down Bytes: 100 x "<<running_down<<endl;
	results.close();
	m_total.close();
	m_up.close();
	m_down.close();
	
	return 0;
}
