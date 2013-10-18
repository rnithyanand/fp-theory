#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include <ctime>
#include <cstdlib>
#include "file_ops.h"
#include "supertrace_func.h"

using namespace std;

int main(int argc, char *argv[])
{
	ofstream log, stats;
	log.open("log_supertrace_computations.txt", ios::app|ios::out);
	stats.open("supertrace.stats", ios::app|ios::out);

	int random, site = atoi(argv[1]), no_trials = atoi(argv[2]), threshold = atoi(argv[3]), status = -1;
	int temp_index = -1, min_times_index = -1, min_bytes_index = -1, alg_set_id = atoi(argv[5]);
	long long int st_size_sum = 0, st_time_sum = 0;
	double start_pthresh = 0, percentile = atof(argv[4]), min_bytes = INT_MAX, min_times = INT_MAX;
	vector<trace> t, candidates;
	vector<double> ratio_bytes, ratio_times;
	stringstream byte_file, time_file;
	trace temp;
	srand((unsigned)time(0));

	for(int i = 1 ; i <= no_trials ; i ++)
	{
		random = (rand()%100)+1;
		byte_file<<"./input_data/"<<site<<"_"<<random<<".cap.txt";
		time_file<<"./input_data/timeseq_"<<site<<"_"<<random<<".cap.txt";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for trial "<<i<<" / site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		t.push_back(temp);
		start_pthresh += (temp.length);
	}
	start_pthresh = .01 * start_pthresh/no_trials;
	
	log<<"Regular (ND) Super Traces Computation..."<<endl;
	if(alg_set_id == 1)
	{
		candidates.push_back(frontierMax(threshold, t, percentile));
		candidates.push_back(frontierMin(threshold, t, percentile));
	}
	else if(alg_set_id == 2)
	{
		candidates.push_back(trByteWtMax(threshold, t, percentile));
		candidates.push_back(trByteWtMin(threshold, t, percentile));
	}
	else if(alg_set_id == 3)
	{
		candidates.push_back(trLenWtdMax(threshold, t, percentile));
		candidates.push_back(trLenWtdMin(threshold, t, percentile));
	}

	for(int i = 1 ; i <= 25 ; i +=5)
	{
		log<<"Delayed Super Traces Computation...(p_thresh = "<<i*start_pthresh<<")"<<endl;

		if(alg_set_id == 1)
		{
			log<<"Computing Frontier Traces..."<<endl;
			candidates.push_back(frontierMaxPT(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(frontierMaxPT_UP(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(frontierMaxPT_DOWN(threshold, t, percentile, i*start_pthresh));
	
			candidates.push_back(frontierMinPT(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(frontierMinPT_UP(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(frontierMinPT_DOWN(threshold, t, percentile, i*start_pthresh));
		}
		else if(alg_set_id == 2)
		{
			log<<"Computing Byte Weighted Traces..."<<endl;
			candidates.push_back(trByteWtMaxPT(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trByteWtMaxPT_UP(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trByteWtMaxPT_DOWN(threshold, t, percentile, i*start_pthresh));
	
			candidates.push_back(trByteWtMinPT(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trByteWtMinPT_UP(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trByteWtMinPT_DOWN(threshold, t, percentile, i*start_pthresh));
		}
		else if(alg_set_id == 3)
		{ 
			log<<"Computing Length Weighted Traces..."<<endl;
			candidates.push_back(trLenWtdMaxPT(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trLenWtdMaxPT_UP(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trLenWtdMaxPT_DOWN(threshold, t, percentile, i*start_pthresh));
	
			candidates.push_back(trLenWtdMinPT(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trLenWtdMinPT_UP(threshold, t, percentile, i*start_pthresh));
			candidates.push_back(trLenWtdMinPT_DOWN(threshold, t, percentile, i*start_pthresh));
		}
	}

	log<<"Recording stats of generated traces..."<<endl;
	log<<"Writing all traces to disk..."<<endl;
	for(int i = 0; i < candidates.size() ; i ++)
	{
		stats<<"Site: "<<site<<"("<<threshold<<"/"<<no_trials<<", AlgSetID.MethodID: "<<alg_set_id<<"."<<i;
		stats<<", Size: "<<candidates[i].total_bytes<<", Time: "<<candidates[i].ttc;
		stats<<", Length: "<<candidates[i].length<<endl;
		st_size_sum += candidates[i].total_bytes;
		st_time_sum += candidates[i].ttc;
		byte_file.str("");
		time_file.str("");
		byte_file<<"./alltraces/"<<site<<"_"<<percentile<<"_"<<threshold<<"_"<<no_trials<<"_"<<alg_set_id<<"_"<<i<<".size";
		time_file<<"./alltraces/"<<site<<"_"<<percentile<<"_"<<threshold<<"_"<<no_trials<<"_"<<alg_set_id<<"_"<<i<<".time";	
		status = write_trace(candidates[i], byte_file.str(), time_file.str());
	}

	log<<"Computing Ratios of Bytes and Times..."<<endl;
	for(int i = 0 ; i < candidates.size() ; i ++)
	{
		ratio_bytes.push_back(double((double)candidates[i].total_bytes/(double)st_size_sum));
		ratio_times.push_back(double((double)candidates[i].ttc/(double)st_time_sum));
		log<<"Trace ID: "<<i<<", B_ratio: "<<ratio_bytes[i]<<", T_ratio: "<<ratio_times[i]<<endl;
		if(min_bytes > ratio_bytes[i])
		{
			min_bytes = ratio_bytes[i];
			min_bytes_index = i;
		}
		if(min_times > ratio_times[i])
		{
			min_times = ratio_times[i];
			min_times_index = i;
		}
	}

	log<<"Checking to see if compromises may be made to better overhead (L & B)..."<<endl;
	double ratio_diff = 0;
	double max_diff = INT_MIN;
	temp_index = min_bytes_index;
	log<<"Min bytes trace @ "<<min_bytes_index<<endl;
	for(int i = 0 ; i < candidates.size() ; i ++)
	{
		if(ratio_bytes[i] <= 1.1*min_bytes && ratio_times[i]<=.9*ratio_times[min_bytes_index])
		{
			log<<"\nPotential swap @ "<<i<<"....";
			ratio_diff = ratio_times[min_bytes_index]-ratio_times[i];
			if(ratio_diff >= max_diff)
			{
				temp_index = i;
				max_diff = ratio_diff;
				log<<"Swap completed!"<<endl;
			}
		}
	}
	min_bytes_index = temp_index;
	log<<"Compromised min bytes trace @ "<<min_bytes_index<<endl;
	byte_file.str("");
	time_file.str("");
	byte_file<<"./Top500/BOPT"<<site<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<threshold<<"_"<<no_trials<<".size";
	time_file<<"./Top500/BOPT"<<site<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<threshold<<"_"<<no_trials<<".time";
	write_trace(candidates[min_bytes_index], byte_file.str(), time_file.str());

	ratio_diff = 0;
	max_diff = INT_MIN;
	temp_index = min_times_index;
	log<<"Min time trace @ "<<min_times_index<<endl;
	for(int i = 0 ; i < candidates.size() ; i ++)
	{
		if(ratio_times[i] <= 1.1*min_times && ratio_bytes[i]<=.9*ratio_bytes[min_times_index])
		{
			log<<"\nPotential swap @ "<<i<<"...."<<endl;
			ratio_diff = ratio_bytes[min_times_index]-ratio_bytes[i];
			if(ratio_diff >= max_diff)
			{
				temp_index = i;
				max_diff = ratio_diff;
				log<<"Swap completed!"<<endl;
			}
		}
	}
	min_times_index = temp_index;
	log<<"Compromised min time trace @ "<<min_times_index<<endl;	
	byte_file.str("");
	time_file.str("");
	byte_file<<"./Top500/LOPT"<<site<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<threshold<<"_"<<no_trials<<".size";
	time_file<<"./Top500/LOPT"<<site<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<threshold<<"_"<<no_trials<<".time";
	write_trace(candidates[min_times_index], byte_file.str(), time_file.str());

	log<<"Results: "<<endl;	
	log<<"Best BOH Trace -- Bytes: "<<candidates[min_bytes_index].total_bytes<<", Time: "<<candidates[min_bytes_index].ttc<<endl;
	log<<"Best LOH Trace -- Bytes: "<<candidates[min_times_index].total_bytes<<", Time: "<<candidates[min_times_index].ttc<<endl;


	stats.close();
	log.close();
	return 0;
	
}	
