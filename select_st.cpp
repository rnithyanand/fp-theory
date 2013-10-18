#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include <algorithm>
#include "file_ops.h"
using namespace std;
int main(int argc, char *argv[])
{
	ofstream log, stats;
	log.open("log_selection.txt", ios::app|ios::out);
	stats.open("selected.stats", ios::app|ios::out);
	log<<"Selecting best traces for site: "<<argv[1]<<endl;
	int site = atoi(argv[1]), status = -1, temp_index = -1, min_times_index = -1, min_bytes_index = -1;
	long long int st_size_sum = 0, st_time_sum = 0;
	double min_bytes = INT_MAX, min_times = INT_MAX;
	vector<trace> candidates;
	vector<double> ratio_bytes, ratio_times;
	trace temp;
	stringstream byte_file, time_file;

	for(int i = 1 ; i <= 3 ; i ++)
	{
		byte_file<<"./Top500/BOPT"<<site<<"_0.5_"<<i<<"_40_50.size";
		time_file<<"./Top500/BOPT"<<site<<"_0.5_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for BOPT(.5) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/LOPT"<<site<<"_0.5_"<<i<<"_40_50.size";
		time_file<<"./Top500/LOPT"<<site<<"_0.5_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for LOPT(.5) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/BOPT"<<site<<"_0.65_"<<i<<"_40_50.size";
		time_file<<"./Top500/BOPT"<<site<<"_0.65_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for BOPT(.65) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/LOPT"<<site<<"_0.65_"<<i<<"_40_50.size";
		time_file<<"./Top500/LOPT"<<site<<"_0.65_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for LOPT(.65) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/BOPT"<<site<<"_0.8_"<<i<<"_40_50.size";
		time_file<<"./Top500/BOPT"<<site<<"_0.8_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for BOPT(.8) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/LOPT"<<site<<"_0.8_"<<i<<"_40_50.size";
		time_file<<"./Top500/LOPT"<<site<<"_0.8_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for LOPT(.8) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/BOPT"<<site<<"_0.95_"<<i<<"_40_50.size";
		time_file<<"./Top500/BOPT"<<site<<"_0.95_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for BOPT(.95) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;

		byte_file<<"./Top500/LOPT"<<site<<"_0.95_"<<i<<"_40_50.size";
		time_file<<"./Top500/LOPT"<<site<<"_0.95_"<<i<<"_40_50.time";
		status = read_trace(byte_file.str(), time_file.str(), &temp);
		log<<"Load status for LOPT(.95) trace of site "<<site<<": "<<status<<endl;
		byte_file.str("");
		time_file.str("");
		candidates.push_back(temp);
		st_size_sum += temp.total_bytes;
		st_time_sum += temp.ttc;
	}
		
	log<<"Loading complete... Computing Ratios of Bytes and Times"<<endl;
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

	log<<"Checking to see if there are compromises that may be made to lessen latency/bandwidth combined overhead..."<<endl;
	double ratio_diff = 0;
	double max_diff = INT_MIN;
	temp_index = min_bytes_index;
	log<<"Current min bytes trace @ "<<min_bytes_index<<endl;
	for(int i = 0 ; i < candidates.size() ; i++)
	{
		if(ratio_bytes[i] <= 1.1*min_bytes && ratio_times[i]<=.9*ratio_times[min_bytes_index])
		{
			log<<"Potential swap found @ "<<i<<"...."<<endl;
			ratio_diff = ratio_times[min_bytes_index]-ratio_times[i];
			if(ratio_diff >= max_diff)
			{
				temp_index = i;
				max_diff = ratio_diff;
				log<<"Swap Completed with trace "<<i<<endl;
			}
			else
				log<<"Swap not completed."<<endl;
		}
	}
	min_bytes_index = temp_index;
	log<<"Compromised min bytes trace @ "<<min_bytes_index<<endl;
	byte_file.str("");
	time_file.str("");
	byte_file<<"./Top500/Selected/BOPT"<<site<<".size";
	time_file<<"./Top500/Selected/BOPT"<<site<<".time";
	write_trace(candidates[min_bytes_index], byte_file.str(), time_file.str());
	
	ratio_diff = 0;
	max_diff = INT_MIN;
	temp_index = min_times_index;
	log<<"Current min time trace @ "<<min_times_index<<endl;
	for(int i = 0 ; i < candidates.size() ; i ++)
	{
		if(ratio_times[i] <= 1.1*min_times && ratio_bytes[i]<=.9*ratio_bytes[min_times_index])
		{
			ratio_diff = ratio_bytes[min_times_index]-ratio_bytes[i];
			if(ratio_diff >= max_diff)
			{
				temp_index = i;
				max_diff = ratio_diff;
				log<<"Swap Completed with trace "<<i<<endl;
			}
			else
				log<<"Swap not completed."<<endl;
		}
	}
	min_times_index = temp_index;
	log<<"Compromised min time trace @ "<<min_times_index<<endl;
	byte_file.str("");
	time_file.str("");
	byte_file<<"./Top500/Selected/LOPT"<<site<<".size";
	time_file<<"./Top500/Selected/LOPT"<<site<<".time";
	write_trace(candidates[min_times_index], byte_file.str(), time_file.str());
	log<<"Best B_Trace -- Bytes: "<<candidates[min_bytes_index].total_bytes<<", Time: "<<candidates[min_bytes_index].ttc<<endl;
	log<<"Best L_Trace -- Bytes: "<<candidates[min_times_index].total_bytes<<", Time: "<<candidates[min_times_index].ttc<<endl;
	stats<<"Site: "<<site<<", B_OPT Bytes: "<<candidates[min_bytes_index].total_bytes<<", B_OPT Time: "<<candidates[min_bytes_index].ttc<<", ";
	stats<<"B_OPT Len: "<<candidates[min_bytes_index].length<<", ";
	stats<<"L_OPT Bytes: "<<candidates[min_times_index].total_bytes<<", L_OPT Time: "<<candidates[min_times_index].ttc<<", ";
	stats<<"L_OPT Len: "<<candidates[min_times_index].length<<endl;
	stats.close();
	log.close();
	return 0;
}
