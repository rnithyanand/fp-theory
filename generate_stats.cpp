#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>
#include "file_ops.h"
using namespace std;

double find_loh(int site, trace st);
int main(int argc, char *argv[])
{
	ofstream stats_st;
	stats_st.open("selected.stats", ios::app|ios::out);
	int site = atoi(argv[1]), status = -1;
	trace temp;
	stringstream byte_file, time_file;

	byte_file<<"./Top500C/Selected/BOPT"<<site<<".size";
	time_file<<"./Top500C/Selected/BOPT"<<site<<".time";
	status = read_trace(byte_file.str(), time_file.str(), &temp);

	byte_file.str("");
	time_file.str("");
	stats_st<<"Site: "<<site<<", BOPT B: "<<temp.total_bytes<<", BOPT T: "<<temp.ttc<<", BOPT L: "<<temp.length;
	byte_file<<"./Top500C/Selected/LOPT"<<site<<".size";
	time_file<<"./Top500C/Selected/LOPT"<<site<<".time";
	status = read_trace(byte_file.str(), time_file.str(), &temp);
	stats_st<<", LOPT B: "<<temp.total_bytes<<", LOPT T: "<<temp.ttc<<", LOPT L: "<<temp.length;

	double loh = find_loh(site, temp);
	stats_st<<". (LOH = "<<loh<<")"<<endl;
	stats_st.close();
	return 0;
}

double find_loh(int site, trace st)
{
	ofstream log;
	log.open("log_selected.stats", ios::app|ios::out);
	trace temp;
	log<<"Site: "<<site<<endl;
	vector<trace> input;
	stringstream input_b, input_t;
	for(int i = 1 ; i <= 100 ; i ++)
	{
		input_b<<"./input_data/"<<site<<"_"<<i<<".cap.txt";
		input_t<<"./input_data/timeseq_"<<site<<"_"<<i<<".cap.txt";
		read_trace(input_b.str(), input_t.str(), &temp);
		input_b.str("");
		input_t.str("");
		input.push_back(temp);
	}
	vector<long long int> completion_times(input.size());
	int no_traces = input.size(), complete_count = 0;
	vector<long long int> curr_packet_ip(no_traces);
	long long int curr_packet_st = 0, b_removed = 0;

	for(int i = 0 ; i < st.packets.size() ; i ++)
	{
		log<<"ST Packet #"<<i<<", ";
		curr_packet_st = i;
		long long int st_dir = st.packets[curr_packet_st].direction, st_time = st.packets[curr_packet_st].time, st_size = st.packets[curr_packet_st].size;
		log<<"ST PDir: "<<st_dir<<", ST PTime: "<<st_time<<", ST PSize: "<<st_size<<endl;
		for(int j = 0 ; j < input.size() ; j ++)
		{
			log<<"Input trace: "<<j<<", IP#: "<<curr_packet_ip[j]<<", ID: "<<input[j].packets[curr_packet_ip[j]].direction<<", IS: "<<input[j].packets[curr_packet_ip[j]].size<<", IT: "<<input[j].packets[curr_packet_ip[j]].time<<endl;
			while(input[j].packets[curr_packet_ip[j]].direction == st_dir && input[j].packets[curr_packet_ip[j]].time <= st_time && completion_times[j] == 0 && complete_count < 40)
			{
				b_removed = min(st_size, input[j].packets[curr_packet_ip[j]].size);
				log<<b_removed<<" Bytes removed from input packet....";
				input[j].packets[curr_packet_ip[j]].size -= b_removed;
				log<<input[j].packets[curr_packet_ip[j]].size<<" Bytes remain"<<endl;
				if(input[j].packets[curr_packet_ip[j]].size == 0 && curr_packet_ip[j] < input[j].packets.size()-1)
					curr_packet_ip[j]++;
				else
				{
					complete_count++;
					completion_times[j] = st_time;
				}
			}
		}
	}
	
	double total_completion_time = 0;
	double actual_completion_time =0;
	for(int i = 0 ; i < input.size() ; i ++)
	{
		if(completion_times[i] > 0)
		{
			log<<"Trace "<<i<<", ST Completion Time: "<<completion_times[i]<<", Actual Completion Time: "<<input[i].ttc<<endl;
			total_completion_time += completion_times[i];
			actual_completion_time += input[i].ttc;
		}
	}
	return total_completion_time/actual_completion_time;
}
