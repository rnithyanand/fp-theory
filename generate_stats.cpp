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
	double loh = find_loh(site, temp);
	
	byte_file.str("");
	time_file.str("");
	stats_st<<"Site: "<<site<<", BOPT B: "<<temp.total_bytes<<", BOPT T: "<<temp.ttc<<", BOPT L: "<<temp.length <<", BOPT LOH: "<<loh<<endl;
/*	byte_file<<"./Top500C/Selected/LOPT"<<site<<".size";
	time_file<<"./Top500C/Selected/LOPT"<<site<<".time";
	status = read_trace(byte_file.str(), time_file.str(), &temp);
	loh = find_loh(site, temp);
	stats_st<<", LOPT B: "<<temp.total_bytes<<", LOPT T: "<<temp.ttc<<", LOPT L: "<<temp.length<<", LOPT LOH: "<<loh<<endl;
*/
	stats_st.close();
	return 0;
}

double find_loh(int site, trace st)
{
	ofstream log;
	log.open("log_selected.stats", ios::app|ios::out);
	trace temp;
	//log<<"Site: "<<site<<endl;
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
	int no_traces = input.size(), complete_count = 0, bytes_remaining_ip = 0;
	vector<long long int> curr_packet_ip(no_traces), bytes_remaining_st(no_traces);
	long long int curr_packet_st = 0, b_removed = 0;

	for(int i = 0 ; i < st.packets.size() ; i ++)
	{
		long long int st_size = st.packets[i].size, st_time = st.packets[i].time, st_dir = st.packets[i].direction;
		//log<<"ST P#"<<i<<". Size: "<<st_size<<", Time: "<<st_time<<", Direction: "<<st_dir<<endl;
		for(int j = 0 ; j < input.size() ; j++)
		{
			//log<<"Looking at input trace "<<j<<endl;
			bytes_remaining_st[j] = 0;
			st_size = st.packets[i].size;
			//log<<"Current packet: "<<curr_packet_ip[j]<<", Size: "<<input[j].packets[curr_packet_ip[j]].size<<", Time: "<<input[j].packets[curr_packet_ip[j]].time<<", Direction: "<<input[j].packets[curr_packet_ip[j]].direction<<endl;
	
			while(complete_count < 40 && bytes_remaining_st[j] != 1 && completion_times[j] == 0 && st_dir == input[j].packets[curr_packet_ip[j]].direction && st_time >= input[j].packets[curr_packet_ip[j]].time)
			{
				//log<<"Current packet: "<<curr_packet_ip[j]<<", Size: "<<input[j].packets[curr_packet_ip[j]].size<<", Time: "<<input[j].packets[curr_packet_ip[j]].time<<", Direction: "<<input[j].packets[curr_packet_ip[j]].direction<<endl;
				if(st_size >= input[j].packets[curr_packet_ip[j]].size)
				{
					//log<<"ST packet has more bytes than input... Sending and moving to next input packet"<<endl;
					st_size -= input[j].packets[curr_packet_ip[j]].size;
					curr_packet_ip[j]++;
					if(curr_packet_ip[j] >= input[j].packets.size())
					{
						//log<<"Reached end of input trace...Time:"<<st_time<<endl;
						complete_count++;
						completion_times[j] = st_time;
					}
				}
				else
				{
					//log<<"Input packet larger than ST. Sending and moving to next ST packet"<<endl;
					input[j].packets[curr_packet_ip[j]].size -= st_size;
					bytes_remaining_st[j] = 1;
				}
			}
		}
	}
	log<<"Site: "<<site<<", Completed: "<<complete_count<<endl;
	double total_completion_time = 0;
	double actual_completion_time =0;
	for(int i = 0 ; i < input.size() ; i ++)
	{
		if(completion_times[i] > 0)
		{
			//log<<"Trace "<<i<<", ST Completion Time: "<<completion_times[i]<<", Actual Completion Time: "<<input[i].ttc<<endl;
			total_completion_time += completion_times[i];
			actual_completion_time += input[i].ttc;
		}
	}
	return total_completion_time/actual_completion_time;
}
