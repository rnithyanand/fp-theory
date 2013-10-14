#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <limits.h>
#include <stdlib.h>
#include "file_ops.h"
#include "supertrace_func.h"

using namespace std;

int main(int argc, char *argv[])
{
	long long int cluster_id, no_clusters, algo_id;
	cluster_id = atoi(argv[1]);
	no_clusters = atoi(argv[2]);
	algo_id = atoi(argv[3]);
	double time_multiplier = atof(argv[4]);

	ifstream members_file;
	ofstream log;
	stringstream input;
	vector<string> input_cluster_list;
	string temp_s;

	log.open("log_hyper.txt", ios::trunc|ios::out);
	input<<"./cluster_data/members_"<<cluster_id<<"."<<no_clusters;
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

	for(int i = 0 ; i < input_cluster_lisinput_cluster_list.size() ; i ++)
	{
		bfile<<"./Top1000/BestTraces/B_OH/80/"<<input_cluster_list[i]<<"_80_100.size";
		tfile<<"./Top1000/BestTraces/B_OH/80/"<<input_cluster_list[i]<<"_80_100.time";
		status = read_trace(bfile.str(), tfile.str(), &temp)
		log<<"Read status for site:"<<input_cluster_list[i]<<" is : "<<status<<endl;;
		bfile.str("");
		tfile.str("");
		t.push_back(temp);
	}

	ofstream ht_stats;
	ht_stats.open("stats_ht.all", ios::app|ios::out);
	stringstream alltraces_time, alltraces_size, command;

	/*
	command<<"mkdir ./alltraces/hyper/";
	string command_str = command.str();
	system(command_str.c_str());
	command.str("");	
	*/	

	vector<trace> hyper_candidates;
	if(algo_id == 1)
	{
		hyper_candidates.push_back(frontierMax(input_cluster_list.size(), t, time_multiplier));
		alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMax.time";	
		alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMax.size";
		status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
		alltraces_time.str("");
		alltraces_size.str("");
		ht_stats<<cluster_id<<","<<no_clusters<<", FrontierMax, 0, "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
		
		for(int i = 10 ; i <= 50 ; i += 5)
        	{
	                hyper_candidates.push_back(frontierMaxPT(input_cluster_list.size(), t, time_multiplier, i));
			alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMaxPT_"<<i<<".time";	
			alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMaxPT_"<<i<<".size";
	                status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
			log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
	                alltraces_time.str("");
	                alltraces_size.str("");
			ht_stats<<cluster_id<<","<<no_clusters<<", FrontierMaxPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

			hyper_candidates.push_back(frontierMaxPT_UP(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMaxUPPT_"<<i<<".time";
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMaxUPPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
			log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
	                alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", FrontierMaxUPPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

		} 
	}
	else if(algo_id == 2)
        {
                hyper_candidates.push_back(frontierMin(input_cluster_list.size(), t, time_multiplier));
                alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMin.time";       
                alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMin.size";
                status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
       		log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		alltraces_time.str("");
                alltraces_size.str("");
                ht_stats<<cluster_id<<","<<no_clusters<<", FrontierMin, 0, "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                for(int i = 10 ; i <= 50 ; i += 5)
                {
                        hyper_candidates.push_back(frontierMinPT(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMinPT_"<<i<<".time";     
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMinPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", FrontierMinPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
                
                        hyper_candidates.push_back(frontierMinPT_UP(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMinUPPT_"<<i<<".time";
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_frontierMinUPPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", FrontierMinUPPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                }

	}
	else if(algo_id == 3)
	{
                hyper_candidates.push_back(trLenWtdMin(input_cluster_list.size(), t, time_multiplier));
                alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trLenWtdMin.time";       
                alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trLenWtdMin.size";
                status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		 alltraces_time.str("");
                alltraces_size.str("");
                ht_stats<<cluster_id<<","<<no_clusters<<", trLenWtdMin, 0, "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                for(int i = 10 ; i <= 50 ; i += 5)
                {
                        hyper_candidates.push_back(trLenWtdMinPT(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMinPT_"<<i<<".time";     
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMinPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trLenWtdMinPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
                
                        hyper_candidates.push_back(trLenWtdMinPT_UP(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMinUPPT_"<<i<<".time";
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMinUPPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trLenWtdMinUPPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                } 
	
	}
	else if(algo_id == 4)
	{
		hyper_candidates.push_back(trByteWtMin(input_cluster_list.size(), t, time_multiplier));
                alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trByteWtdMin.time";       
                alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trByteWtdMin.size";
                status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		 alltraces_time.str("");
                alltraces_size.str("");
                ht_stats<<cluster_id<<","<<no_clusters<<", trByteWtdMin, 0, "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                for(int i = 10 ; i <= 50 ; i += 5)
                {
                        hyper_candidates.push_back(trByteWtMinPT(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMinPT_"<<i<<".time";     
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMinPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trByteWtdMinPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
                
                        hyper_candidates.push_back(trLenWtdMinPT_UP(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMinUPPT_"<<i<<".time";
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMinUPPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trByteWtdMinUPPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                }	
	}
	else if(algo_id == 5)
	{
                hyper_candidates.push_back(trLenWtdMax(input_cluster_list.size(), t, time_multiplier));
                alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trLenWtdMax.time";       
                alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trLenWtdMax.size";
                status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		 alltraces_time.str("");
                alltraces_size.str("");
                ht_stats<<cluster_id<<","<<no_clusters<<", trLenWtdMax, 0, "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                for(int i = 10 ; i <= 50 ; i += 5)
                {
                        hyper_candidates.push_back(trLenWtdMaxPT(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMaxPT_"<<i<<".time";     
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMaxPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trLenWtdMaxPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
                
                        hyper_candidates.push_back(trLenWtdMaxPT_UP(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMaxUPPT_"<<i<<".time";
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TLWMaxUPPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trLenWtdMaxUPPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
		}
	}
	else if(algo_id == 6)
	{
		hyper_candidates.push_back(trByteWtMax(input_cluster_list.size(), t, time_multiplier));
                alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trByteWtdMax.time";       
                alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_trByteWtdMax.size";
                status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		 alltraces_time.str("");
                alltraces_size.str("");
                ht_stats<<cluster_id<<","<<no_clusters<<", trByteWtdMax, 0, "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;

                for(int i = 10 ; i <= 50 ; i += 5)
                {
                        hyper_candidates.push_back(trByteWtMaxPT(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMaxPT_"<<i<<".time";     
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMaxPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trByteWtdMaxPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
                
                        hyper_candidates.push_back(trLenWtdMaxPT_UP(input_cluster_list.size(), t, time_multiplier, i));
                        alltraces_time<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMaxUPPT_"<<i<<".time";
                        alltraces_size<<"./alltraces/hyper/"<<no_clusters<<"cl_"<<cluster_id<<"_"<<to_string(time_multiplier)<<"_TBWMaxUPPT_"<<i<<".size";
                        status = write_trace(hyper_candidates.back(), alltraces_size.str(), alltraces_time.str());
               	log<<"Write status for cluster:"<<alltraces_size.str()<<" is : "<<status<<endl;
		         alltraces_time.str("");
                        alltraces_size.str("");
                        ht_stats<<cluster_id<<","<<no_clusters<<", trByteWtdMaxUPPT, "<<i<<", "<<hyper_candidates.back().length<<", "<<hyper_candidates.back().ttc<<", "<<hyper_candidates.back().total_bytes<<", "<<hyper_candidates.back().bw_oh<<", "<<hyper_candidates.back().latency_oh<<endl;
                }	
	}
	

	return 0;
}
