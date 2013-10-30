#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <sstream>
#include <limits.h>
#include "file_ops.h"
#include "supertrace_func.h"

using namespace std;

int main(int argc, char *argv[])
{
	long long int cluster_id = atoi(argv[1]), no_clusters = atoi(argv[2]), alg_set_id = atoi(argv[3]);
	double percentile = atof(argv[4]), start_pthresh = 0, min_bytes = INT_MAX, min_times = INT_MAX;
        int temp_index = -1, min_times_index = -1, min_bytes_index = -1;
	long long int st_size_sum = 0, st_time_sum = 0;
	vector<double> ratio_bytes, ratio_times;
	ifstream members_file;
	ofstream log;
	stringstream input;
	vector<string> input_cluster_list;
	string temp_s;

	//log.open("log_hypertrace.txt", ios::trunc|ios::out);
	input<<"./cluster_data/HT_6/members_"<<cluster_id<<"."<<no_clusters;
	members_file.open(input.str());
	input.str("");	

	while(!members_file.eof())
	{	
		getline(members_file, temp_s);
		if(!temp_s.empty() && temp_s != "")
			input_cluster_list.push_back(temp_s);
	}
	members_file.close();

	vector<trace> t;
	int status = -1;
	stringstream bfile, tfile;
	trace temp;

	for(int i = 0 ; i < input_cluster_list.size() ; i ++)
	{
		bfile<<"./Top500C/Selected/BOPT"<<input_cluster_list[i]<<".size";
		tfile<<"./Top500C/Selected/BOPT"<<input_cluster_list[i]<<".time";
		status = read_trace(bfile.str(), tfile.str(), &temp);
		bfile.str("");
		tfile.str("");
		log<<"Input site "<<input_cluster_list[i]<<", R Status: "<<status<<endl;
		t.push_back(temp);
		start_pthresh += (temp.length);
	}
	start_pthresh = .01 * start_pthresh/input_cluster_list.size();
	
	ofstream stats;
	stats.open("Hypertraces.stats", ios::app|ios::out);
	vector<trace> candidates;

	if(alg_set_id == 1)
	{
		candidates.push_back(frontierMax(input_cluster_list.size()-1, t, percentile));	
		candidates.push_back(frontierMin(input_cluster_list.size()-1, t, percentile));
	}
	else if(alg_set_id == 2)
	{
		candidates.push_back(trByteWtMax(input_cluster_list.size()-1, t, percentile));
		candidates.push_back(trByteWtMin(input_cluster_list.size()-1, t, percentile));
	}
	else if(alg_set_id == 3)
	{
		candidates.push_back(trLenWtdMax(input_cluster_list.size()-1, t, percentile));
		candidates.push_back(trLenWtdMin(input_cluster_list.size()-1, t, percentile));
	}

	for(int i = 1 ; i <= 10 ; i += 1)
	{
		 if(alg_set_id == 1)
                {
                        log<<"Computing Frontier Traces..."<<endl;
                        candidates.push_back(frontierMaxPT(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(frontierMaxPT_UP(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(frontierMaxPT_DOWN(input_cluster_list.size()-1, t, percentile, i*start_pthresh));

                        candidates.push_back(frontierMinPT(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(frontierMinPT_UP(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(frontierMinPT_DOWN(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                }
                else if(alg_set_id == 2)
                {
                        log<<"Computing Byte Weighted Traces..."<<endl;
                        candidates.push_back(trByteWtMaxPT(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trByteWtMaxPT_UP(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trByteWtMaxPT_DOWN(input_cluster_list.size()-1, t, percentile, i*start_pthresh));

                        candidates.push_back(trByteWtMinPT(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trByteWtMinPT_UP(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trByteWtMinPT_DOWN(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                }
                else if(alg_set_id == 3)
                {
                        log<<"Computing Length Weighted Traces..."<<endl;
                        candidates.push_back(trLenWtdMaxPT(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trLenWtdMaxPT_UP(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trLenWtdMaxPT_DOWN(input_cluster_list.size()-1, t, percentile, i*start_pthresh));

                        candidates.push_back(trLenWtdMinPT(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trLenWtdMinPT_UP(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                        candidates.push_back(trLenWtdMinPT_DOWN(input_cluster_list.size()-1, t, percentile, i*start_pthresh));
                }

	}

        for(int i = 0; i < candidates.size() ; i ++)
        {
                stats<<"Cluster: "<<cluster_id<<", AlgSetID.MethodID: "<<alg_set_id<<"."<<i;
                stats<<", Size: "<<candidates[i].total_bytes<<", Time: "<<candidates[i].ttc;
                stats<<", Length: "<<candidates[i].length<<endl;
                st_size_sum += candidates[i].total_bytes;
                st_time_sum += candidates[i].ttc;
                bfile.str("");
                tfile.str("");
                bfile<<"./hypertraces/HT_6/"<<cluster_id<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<i<<".size";
                tfile<<"./hypertraces/HT_6/"<<cluster_id<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<i<<".time";
                status = write_trace(candidates[i], bfile.str(), tfile.str());
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
        bfile.str("");
        tfile.str("");
        bfile<<"./Top500C/HT_6/BOPT"<<cluster_id<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<".size";
        tfile<<"./Top500C/HT_6/BOPT"<<cluster_id<<"_"<<percentile<<"_"<<alg_set_id<<"_"<<".time";
        write_trace(candidates[min_bytes_index], bfile.str(), tfile.str());

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
        bfile.str("");
        tfile.str("");
        bfile<<"./Top500C/HT_6/LOPT"<<cluster_id<<"_"<<percentile<<"_"<<alg_set_id<<".size";
        tfile<<"./Top500C/HT_6/LOPT"<<cluster_id<<"_"<<percentile<<"_"<<alg_set_id<<".time";
        write_trace(candidates[min_times_index], bfile.str(), tfile.str());
	log.close();
	stats.close();

}

