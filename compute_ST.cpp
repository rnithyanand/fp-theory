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
	ofstream log;
	log.open("log_compute_ST.txt", ios::app|ios::out);
	if(!log.is_open())
		return -1;

	long long int site, no_trials, threshold;
	site = atoi(argv[1]);
	no_trials = atoi(argv[2]);
	threshold = atoi(argv[3]);
	
	double time_multiplier = atof(argv[4]);	
	//time_multiplier = time_multiplier/10;

	vector<trace> t;
	int status = -1;

	stringstream bfile, tfile;
	trace temp;

	for(long long int i = 0 ; i <= no_trials ; i ++)
	{
		bfile<<"./input_data/"<<site<<"_"<<i+1<<".cap.txt";
		tfile<<"./input_data/timeseq_"<<site<<"_"<<i+1<<".cap.txt";
		log<<"Loading traces of site "<<site<<" and trial "<<i+1<<" from files: "<<bfile.str()<<" and "<<tfile.str()<<endl;
                status = read_trace(bfile.str(), tfile.str(), &temp);
                log<<"Status: "<<status<<endl;
                bfile.str("");
                tfile.str("");
                log<<"Storing trace in vector."<<endl;
                t.push_back(temp);
                log<<"Trace loaded successfully."<<endl;
	}
	cout<<endl;
	log<<endl;

	vector<trace> super_candidates;
	log<<"Computing Supertrace using frontierMax"<<endl;
        super_candidates.push_back(frontierMax(threshold, t, time_multiplier));
	for(int i = 1 ; i <= 100 ; i += 2)
        {
		log<<"Computing Supertrace using frontierMax with Packet Threshold of "<<i<<endl;
	        super_candidates.push_back(frontierMaxPT(threshold, t, time_multiplier, i));
       		log<<"Computing Supertrace using frontierMax with Time Threshold of 10^4 x "<<i<<endl;
	        super_candidates.push_back(frontierMaxTT(threshold, t, time_multiplier, i*10000));
        	log<<"Computing Supertrace using frontierMax with UP PT of "<<i<<endl;
	        super_candidates.push_back(frontierMaxPT_UP(threshold, t, time_multiplier, i));
        	log<<"Computing Supertrace using frontierMax with UP TT of 10^4 x "<<i<<endl;
	        super_candidates.push_back(frontierMaxTT_UP(threshold, t, time_multiplier, i*10000));
	}

        log<<"Computing Supertrace using frontierMin"<<endl;
        super_candidates.push_back(frontierMin(threshold, t, time_multiplier));
	for(int i = 1 ; i <= 100 ; i += 2)
	{
	        log<<"Computing Supertrace using frontierMin with Packet Threshold of "<<i<<endl;
	        super_candidates.push_back(frontierMinPT(threshold, t, time_multiplier, i));
	        log<<"Computing Supertrace using frontierMin with Time Threshold of 10000 x "<<i<<endl;
	        super_candidates.push_back(frontierMinTT(threshold, t, time_multiplier, 10000*i));
	        log<<"Computing Supertrace using frontierMin with UP PT of "<<i<<endl;
	        super_candidates.push_back(frontierMinPT_UP(threshold, t, time_multiplier, i));
	        log<<"Computing Supertrace using frontierMin with UP TT of 10000 x "<<i<<endl;
       	 	super_candidates.push_back(frontierMinTT_UP(threshold, t, time_multiplier, 10000*i));
	}

        log<<"Computing Supertrace using trLenWtdMin"<<endl;
        super_candidates.push_back(trLenWtdMin(threshold, t, time_multiplier));
	for(int i = 1 ; i <= 100 ; i += 2)
	{
        	log<<"Computing Supertrace using trLenWtdMin with Packet Threshold of "<<i<<endl;
	        super_candidates.push_back(trLenWtdMinPT(threshold, t, time_multiplier, i));
        	log<<"Computing Supertrace using trLenWtdMin with Time Threshold of 10000 x "<<i<<endl;
	        super_candidates.push_back(trLenWtdMinTT(threshold, t, time_multiplier, 10000*i));
        	log<<"Computing Supertrace using trLenWtdMin with UP PT of "<<i<<endl;
	        super_candidates.push_back(trLenWtdMinPT_UP(threshold, t, time_multiplier, i));
        	log<<"Computing Supertrace using trLenWtdMin with UP TT of 10000 x "<<i<<endl;
	        super_candidates.push_back(trLenWtdMinTT_UP(threshold, t, time_multiplier, 10000*i));
	}

        log<<"Computing Supertrace using trByteWtMin"<<endl;
        super_candidates.push_back(trByteWtMin(threshold, t, time_multiplier));
	for(int i = 1 ; i <= 100 ; i += 2)
	{
	        log<<"Computing Supertrace using trByteWtMin with Packet Threshold of "<<i<<endl;
	        super_candidates.push_back(trByteWtMinPT(threshold, t, time_multiplier, i));
	        log<<"Computing Supertrace using trByteWtMin with Time Threshold of 10000 x "<<i<<endl;
	        super_candidates.push_back(trByteWtMinTT(threshold, t, time_multiplier, 10000*i));
	        log<<"Computing Supertrace using trByteWtMin with UP PT of "<<i<<endl;
	        super_candidates.push_back(trByteWtMinPT_UP(threshold, t, time_multiplier, i));
	        log<<"Computing Supertrace using trByteWtMin with UP TT of 10000 * "<<i<<endl;
	        super_candidates.push_back(trByteWtMinTT_UP(threshold, t, time_multiplier, 10000*i));
	}

        log<<"Computing Supertrace using trLenWtdMax"<<endl;
        super_candidates.push_back(trLenWtdMax(threshold, t, time_multiplier));
	for(int i = 1 ; i <= 100 ; i += 2)
	{
	        log<<"Computing Supertrace using trLenWtdMax with Packet Threshold of "<<i<<endl;
	        super_candidates.push_back(trLenWtdMaxPT(threshold, t, time_multiplier, i));
       		log<<"Computing Supertrace using trLenWtdMax with Time Threshold of 10000 x "<<i<<endl;
	        super_candidates.push_back(trLenWtdMaxTT(threshold, t, time_multiplier, 10000*i));
	        log<<"Computing Supertrace using trLenWtdMax with UP PT of "<<i<<endl;
	        super_candidates.push_back(trLenWtdMaxPT_UP(threshold, t, time_multiplier, i));
       		log<<"Computing Supertrace using trLenWtdMax with UP TT of 10000 x "<<i<<endl;
	        super_candidates.push_back(trLenWtdMaxTT_UP(threshold, t, time_multiplier, 10000*i));
	}

        log<<"Computing Supertrace using trByteWtMax"<<endl;
        super_candidates.push_back(trByteWtMax(threshold, t, time_multiplier));
	for(int i = 1 ; i <= 100 ; i += 2)
	{
	        log<<"Computing Supertrace using trByteWtMax with Packet Threshold of "<<i<<endl;
        	super_candidates.push_back(trByteWtMaxPT(threshold, t, time_multiplier, i));
	        log<<"Computing Supertrace using trByteWtMax with Time Threshold of 10000 x "<<i<<endl;
	        super_candidates.push_back(trByteWtMaxTT(threshold, t, time_multiplier, 10000*i));
	        log<<"Computing Supertrace using trByteWtMax with UP PT of "<<i<<endl;
	        super_candidates.push_back(trByteWtMaxPT_UP(threshold, t, time_multiplier,i));
	        log<<"Computing Supertrace using trByteWtMax with UP TT of 10000 x "<<i<<endl;
	        super_candidates.push_back(trByteWtMaxTT_UP(threshold, t, time_multiplier, 10000*i));
	}

	/*
		Now, find the supertraces with 1. lowest B OH, 2. lowest L OH, and 3. lowest B+L OH
	*/
	
	double min_b = INT_MAX, min_l = INT_MAX, min_lb = INT_MAX;
	int index_b, index_l, index_lb;

	for(int i = 0 ; i < super_candidates.size() ; i ++)
	{
		printSTShort(super_candidates[i]);
		if(super_candidates[i].bw_oh <= min_b && super_candidates[i].bw_oh > 0)
		{
			min_b = super_candidates[i].bw_oh;
			index_b = i;
		}
		if(super_candidates[i].latency_oh <= min_l && super_candidates[i].bw_oh > 0)
		{
			min_l = super_candidates[i].latency_oh;
			index_l = i;
		}
		if(super_candidates[i].latency_oh + super_candidates[i].bw_oh <= min_lb && super_candidates[i].bw_oh > 0)
		{
			min_lb = super_candidates[i].latency_oh + super_candidates[i].bw_oh;
			index_lb = i;
		}
	}

	log<<"Min B OH: "<<min_b<<", in candidate: "<<index_b<<endl;
	log<<"Min L OH: "<<min_l<<", in candidate: "<<index_l<<endl;
	log<<"Min LB OH: "<<min_lb<<", in candidate: "<<index_lb<<endl;

	/*
		Write the three traces to disk.
	*/
	
	stringstream bfile_out, tfile_out;
	bfile_out<<"./"<<threshold<<"BOPT_ST_"<<no_trials<<"_"<<time_multiplier<<"/"<<site<<".size";
	tfile_out<<"./"<<threshold<<"BOPT_ST_"<<no_trials<<"_"<<time_multiplier<<"/"<<site<<".time";
	stringstream command;
	command<<"mkdir ./"<<threshold<<"BOPT_ST_"<<no_trials<<"_"<<time_multiplier;
	string cstr = command.str();
	system(cstr.c_str());
	command.str("");	

	log<<"Writing supertraces to disk. B OH optimized supertrace files: "<<bfile_out.str()<<", "<<tfile_out.str()<<endl;
	status = write_trace(super_candidates[index_b], bfile_out.str(), tfile_out.str());


	bfile_out.str("");
	tfile_out.str("");
	bfile_out<<"./"<<threshold<<"LOPT_ST_"<<no_trials<<"_"<<time_multiplier<<"/"<<site<<".size";
	tfile_out<<"./"<<threshold<<"LOPT_ST_"<<no_trials<<"_"<<time_multiplier<<"/"<<site<<".time";
	command<<"mkdir ./"<<threshold<<"LOPT_ST_"<<no_trials<<"_"<<time_multiplier;
	cstr = command.str();
	system(cstr.c_str());
	command.str("");	

	log<<"Writing supertraces to disk. L OH optimized supertrace files: "<<bfile_out.str()<<", "<<tfile_out.str()<<endl;
	status = write_trace(super_candidates[index_l], bfile_out.str(), tfile_out.str());


	bfile_out.str("");
	tfile_out.str("");
	bfile_out<<"./"<<threshold<<"BLOPT_ST_"<<no_trials<<"_"<<time_multiplier<<"/"<<site<<".size";
	tfile_out<<"./"<<threshold<<"BLOPT_ST_"<<no_trials<<"_"<<time_multiplier<<"/"<<site<<".time";
	command<<"mkdir ./"<<threshold<<"BLOPT_ST_"<<no_trials<<"_"<<time_multiplier;
	cstr = command.str();
	system(cstr.c_str());
	command.str("");	

	log<<"Writing supertraces to disk. B+L OH optimized supertrace files: "<<bfile_out.str()<<", "<<tfile_out.str()<<endl;
	status = write_trace(super_candidates[index_lb], bfile_out.str(), tfile_out.str());

        log.close();
        return 0;
}
