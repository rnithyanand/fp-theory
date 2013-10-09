#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

#include "file_ops.h"

using namespace std;
long long int compute_min_time(vector<trace> tr, vector<packet> p)
{
	long long int no_traces = tr.size();
	long long int *current_packet = new long long int[no_traces];
	long long int *time = new long long int[no_traces];

	for(int i = 0 ; i < no_traces ; i ++)
	{
		current_packet[i] = 0;
		time[i] = -1;
	}

	long long int curr_size, curr_time;
	int curr_direction;
	long long int no_packets_st = p.size();
	for(int i = 0 ; i < no_packets_st ; i ++)
	{
		//Store current size and direction of st packet
		curr_size = p[i].size;
		curr_direction = p[i].direction;
		curr_time = p[i].time;
		//if any of the current trace packets are in the same direction, remove these bytes from them//
		for(int j = 0 ; j < no_traces ; j ++)
		{
			if((time[j] == -1) && (tr[j].packets[current_packet[j]].direction == curr_direction) && (tr[j].packets[current_packet[j]].time <= curr_time))
			{
				tr[j].packets[current_packet[j]].size -= curr_size;
				//if the packet is now empty, move on to the next one
				if(tr[j].packets[current_packet[j]].size <= 0)
					current_packet[j]++;
				//if the last packet was the last in the trace, then write the current time as the completion time
				if(current_packet[j] == tr[j].packets.size())
					time[j] = curr_time;
			}
		}
	}
	for(int i = 0 ; i < no_traces ; i ++)
		if(time[i] == -1)
			time[i] = curr_time + 1 * (tr[i].packets[tr[i].packets.size()-1].time - tr[i].packets[current_packet[i]-1].time);
	long long int min_time = 0;
	for(int i = 0 ; i < no_traces ; i ++)
		min_time += time[i];
	return min_time;

}
long long int compute_min_bytes(vector<trace> tr, vector<packet> p)
{
	vector<long long int> completed_bytes(tr.size());
	vector<long long int> remaining_bytes;
	long long int no_traces = tr.size();
	long long int *current_packet = new long long int[no_traces];

	for(long long int i = 0 ; i < no_traces ; i ++)
	{
		remaining_bytes.push_back(tr[i].total_bytes);
		current_packet[i] = 0;
	}
	
	long long int curr_size, curr_time;
	int curr_direction;
	long long int no_packets_st = p.size();

	for(long long int i = 0 ; i < no_packets_st ; i ++)
	{
		curr_size = p[i].size;
		curr_time = p[i].time;
		curr_direction = p[i].direction;
		for(long long int j = 0 ; j < no_traces ; j ++)
			if(remaining_bytes[j] > 0 && tr[j].packets[current_packet[j]].direction == curr_direction && tr[j].packets[current_packet[j]].time <= curr_time)
                        {
                                completed_bytes[j] += curr_size;
                                remaining_bytes[j] -= curr_size;
                        }
	}
	long long int min_bytes = 0;
	for(long long int i = 0 ; i < no_traces ; i ++)
		min_bytes += tr[i].total_bytes //+ 1 * (remaining_bytes[i]);
	return min_bytes;
}

int main(int argc, char *argv[])
{
	ofstream log;
	log.open("log_select_best_traces.txt", ios::app|ios::out);
	long long int site_no = atol(argv[1]);
	long long int threshold = atol(argv[2]);
	long long int no_trials = atol(argv[3]);
	log<<"Loading all traces for site: "<<site_no<<endl;

	vector<trace> t;
	int status = -1;
	stringstream bfile, tfile;
	trace temp;

	for(long long int i = 0 ; i <= threshold ; i ++)
	{
		bfile<<"./input_data/"<<site_no<<"_"<<i+1<<".cap.txt";
		tfile<<"./input_data/timeseq_"<<site_no<<"_"<<i+1<<".cap.txt";
		status = read_trace(bfile.str(), tfile.str(), &temp);
		bfile.str("");
		tfile.str("");
		t.push_back(temp);
	}
	log<<"Traces for site "<<site_no<<" successfully loaded"<<endl;
	
	stringstream cbfile, ctfile;
	vector<trace> candidate;
	for(double i = 1 ; i <= 5 ; i += .25)
	{
		cbfile<<"./Top1000/"<<no_trials<<"BOPT_ST_"<<threshold<<"_"<<i<<"/"<<site_no<<".size";
		ctfile<<"./Top1000/"<<no_trials<<"BOPT_ST_"<<threshold<<"_"<<i<<"/"<<site_no<<".time";
		status = read_trace(cbfile.str(), ctfile.str(), &temp);
		if(status != 2)
		{	
			candidate.push_back(temp);
			log<<cbfile.str()<<" trace loaded"<<endl;
		}
		cbfile.str("");
		ctfile.str("");
	}
	for(double i = 1 ; i <= 5 ; i += .25)
	{
		cout<<i<<endl;
		cbfile<<"./Top1000/"<<no_trials<<"LOPT_ST_"<<threshold<<"_"<<i<<"/"<<site_no<<".size";
		ctfile<<"./Top1000/"<<no_trials<<"LOPT_ST_"<<threshold<<"_"<<i<<"/"<<site_no<<".time";
		status = read_trace(cbfile.str(), ctfile.str(), &temp);
		if(status != 2)
		{	
			candidate.push_back(temp);
			log<<cbfile.str()<<" trace loaded"<<endl;
		}
		cbfile.str("");
		ctfile.str("");
	}
	for(double i = 1 ; i <= 5 ; i += .25)
	{
		cbfile<<"./Top1000/"<<no_trials<<"BLOPT_ST_"<<threshold<<"_"<<i<<"/"<<site_no<<".size";
		ctfile<<"./Top1000/"<<no_trials<<"BLOPT_ST_"<<threshold<<"_"<<i<<"/"<<site_no<<".time";
		status = read_trace(cbfile.str(), ctfile.str(), &temp);
		if(status != 2)
		{	
			candidate.push_back(temp);
			log<<cbfile.str()<<" trace loaded"<<endl;
		}
		cbfile.str("");
		ctfile.str("");
	}	
	log<<"Finding the minimum bandwidth trace from candidates"<<endl;
	
	vector<long long int> b_min, l_min, bl_min;
	vector<double> b_oh, l_oh, bl_oh;
	long long int st_bytes = 0, tr_bytes = 0, tr_time = 0, st_time = 0;
	double overhead1 = 0, overhead2 = 0;
	long long int min_bytes = 0;
	long long int min_time = 0;

	for(long long int i = 0 ; i < candidate.size() ; i ++)
	{
		min_bytes = compute_min_bytes(t, candidate[i].packets);
		min_time = compute_min_time(t, candidate[i].packets);
		tr_time = 0;
		tr_bytes = 0;
		for(long long int j = 0 ; j < t.size() ; j ++)
		{
			tr_bytes += t[j].total_bytes;
			tr_time += t[j].ttc;
		}
		l_min.push_back(min_time);
		b_min.push_back(min_bytes);
		overhead1 = ((double)min_bytes);
		b_oh.push_back(overhead1);
		log<<"Bytes: "<<min_bytes<<", OH: "<<overhead1<<endl;
		overhead2 = ((double) (double)min_time/(double) tr_time);
		l_oh.push_back(overhead2);
		log<<"Time: "<<min_time<<", OH: "<<overhead2<<endl;
		bl_min.push_back(min_time + min_bytes);
		bl_oh.push_back(overhead1 + overhead2);
	}
	log<<"Min B OH at: "<<min_element(b_oh.begin(), b_oh.end()) - b_oh.begin()<<endl;
	log<<"Min L OH at: "<<min_element(l_oh.begin(), l_oh.end()) - l_oh.begin()<<endl;
	log<<"Min B+L OH at: "<<min_element(bl_oh.begin(), bl_oh.end()) - bl_oh.begin()<<endl;
	
	long long int min_btrace = min_element(b_oh.begin(), b_oh.end()) - b_oh.begin();
	long long int min_ltrace = min_element(l_oh.begin(), l_oh.end()) - l_oh.begin();
	long long int min_bltrace = min_element(bl_oh.begin(), bl_oh.end()) - bl_oh.begin();

	log<<"Writing best trace files to disk"<<endl;	
	stringstream min_bname_size, min_bname_time, min_lname_size, min_lname_time, min_blname_size, min_blname_time;

	min_bname_size<<"./Top1000/BestTraces/B_OH/"<<no_trials<<"/"<<site_no<<"_"<<no_trials<<"_"<<threshold<<".size";
	min_lname_size<<"./Top1000/BestTraces/L_OH/"<<no_trials<<"/"<<site_no<<"_"<<no_trials<<"_"<<threshold<<".size";
	min_blname_size<<"./Top1000/BestTraces/BL_OH/"<<no_trials<<"/"<<site_no<<"_"<<no_trials<<"_"<<threshold<<".size";
	
	min_bname_time<<"./Top1000/BestTraces/B_OH/"<<no_trials<<"/"<<site_no<<"_"<<no_trials<<"_"<<threshold<<".time";
	min_lname_time<<"./Top1000/BestTraces/L_OH/"<<no_trials<<"/"<<site_no<<"_"<<no_trials<<"_"<<threshold<<".time";
	min_blname_time<<"./Top1000/BestTraces/BL_OH/"<<no_trials<<"/"<<site_no<<"_"<<no_trials<<"_"<<threshold<<".time";
	
	stringstream command;
        command<<"mkdir ./Top1000/BestTraces/B_OH/"<<no_trials<<"/";
        string cstr = command.str();
        system(cstr.c_str());
        command.str("");
	command<<"mkdir ./Top1000/BestTraces/L_OH/"<<no_trials<<"/";
	cstr = command.str();
	system(cstr.c_str());
	command.str("");
	command<<"mkdir ./Top1000/BestTraces/BL_OH/"<<no_trials<<"/";
	cstr = command.str();
	system(cstr.c_str());
	command.str("");
/*
        command<<"mkdir ./Top1000/BestTraces/B_OH/90/";
        string cstr = command.str();
        system(cstr.c_str());
        command.str("");
	command<<"mkdir ./Top1000/BestTraces/L_OH/90/";
	cstr = command.str();
	system(cstr.c_str());
	command.str("");
	command<<"mkdir ./Top1000/BestTraces/BL_OH/90/";
	cstr = command.str();
	system(cstr.c_str());
	command.str("");

        command<<"mkdir ./Top1000/BestTraces/B_OH/100/";
        string cstr = command.str();
        system(cstr.c_str());
        command.str("");
	command<<"mkdir ./Top1000/BestTraces/L_OH/100/";
	cstr = command.str();
	system(cstr.c_str());
	command.str("");
	command<<"mkdir ./Top1000/BestTraces/BL_OH/100/";
	cstr = command.str();
	system(cstr.c_str());
	command.str("");
*/

	write_trace(candidate[min_btrace], min_bname_size.str(), min_bname_time.str());
	write_trace(candidate[min_ltrace], min_lname_size.str(), min_lname_time.str());
	write_trace(candidate[min_bltrace], min_blname_size.str(), min_blname_time.str());

	log<<"Traces written to disk."<<endl;
	log<<"Computing trace statistics."<<endl;



	return 0;
}
