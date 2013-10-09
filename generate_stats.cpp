#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

#include "file_ops.h"

using namespace std;

long long int compute_remaining_times(vector<trace> tr, vector<packet> p)
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
                        time[i] = (tr[i].packets[tr[i].packets.size()-1].time - tr[i].packets[current_packet[i]-1].time);
        long long int rem_time = 0;
        for(int i = 0 ; i < no_traces ; i ++)
                rem_time += time[i];
        return rem_time;
}

long long int compute_remaining_bytes(vector<trace> tr, vector<packet> p)
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
        long long int rem_bytes = 0;
        for(long long int i = 0 ; i < no_traces ; i ++)
                rem_bytes += (remaining_bytes[i]);
        return rem_bytes;
}


int main(int argc, char *argv[])
{
	ofstream log;
	log.open("log_stats.txt", ios::app|ios::out);
	long long int site_no = atol(argv[1]);
	long long int threshold = atol(argv[2]);
	long long int loaded_trials = atol(argv[3]);

	vector<trace> loaded_traces;
	int status = -1;
	stringstream bfile, tfile;
	trace temp;

	for(long long int i = 1 ; i <= threshold ; i ++)
	{
		bfile<<"./input_data/"<<site_no<<"_"<<i<<".cap.txt";
		tfile<<"./input_data/timeseq_"<<site_no<<"_"<<i<<".cap.txt";
		status = read_trace(bfile.str(), tfile.str(), &temp);
		bfile.str("");
		tfile.str("");
		loaded_traces.push_back(temp);
	}
	
	log<<"Traces for site "<<site_no<<" successfully loaded"<<endl;

	stringstream b_bfile, b_tfile, l_bfile, l_tfile, bl_bfile, bl_tfile;
	b_bfile<<"./Top1000/BestTraces/B_OH/"<<threshold<<"/"<<site_no<<"_"<<threshold<<"_"<<loaded_trials<<".size";
	l_bfile<<"./Top1000/BestTraces/L_OH/"<<threshold<<"/"<<site_no<<"_"<<threshold<<"_"<<loaded_trials<<".size";
	bl_bfile<<"./Top1000/BestTraces/BL_OH/"<<threshold<<"/"<<site_no<<"_"<<threshold<<"_"<<loaded_trials<<".size";
	b_tfile<<"./Top1000/BestTraces/B_OH/"<<threshold<<"/"<<site_no<<"_"<<threshold<<"_"<<loaded_trials<<".time";
	l_tfile<<"./Top1000/BestTraces/L_OH/"<<threshold<<"/"<<site_no<<"_"<<threshold<<"_"<<loaded_trials<<".time";
	bl_tfile<<"./Top1000/BestTraces/BL_OH/"<<threshold<<"/"<<site_no<<"_"<<threshold<<"_"<<loaded_trials<<".time";

	status = read_trace(b_bfile.str(), b_tfile.str(), &temp);
	stringstream b_stats;
	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/size.stats";
	ofstream b_oh_stats;
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<temp.total_bytes<<endl;
	b_oh_stats.close();
	b_stats.str("");

	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/length.stats";
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<temp.length<<endl;
	b_oh_stats.close();
	b_stats.str("");

	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/time.stats";
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<temp.ttc<<endl;
	b_oh_stats.close();
	b_stats.str("");

	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/up.stats";
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<temp.up_bytes<<endl;
	b_oh_stats.close();
	b_stats.str("");

	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/down.stats";
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<temp.down_bytes<<endl;
	b_oh_stats.close();
	b_stats.str("");

	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/rem_bytes.stats";
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<compute_remaining_bytes(loaded_traces, temp.packets)<<endl;
	b_oh_stats.close();
	b_stats.str("");

	b_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/B_OH/rem_times.stats";
	b_oh_stats.open(b_stats.str(), ios::app|ios::out);
	b_oh_stats<<site_no<<","<<compute_remaining_times(loaded_traces, temp.packets)<<endl;
	b_oh_stats.close();
	b_stats.str("");

	status = read_trace(l_bfile.str(), l_tfile.str(), &temp);
	stringstream l_stats;
	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/size.stats";
	ofstream l_oh_stats;
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.total_bytes<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/length.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.length<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/time.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.ttc<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/up.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.up_bytes<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/down.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.down_bytes<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/rem_bytes.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<compute_remaining_bytes(loaded_traces, temp.packets)<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/L_OH/rem_times.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<compute_remaining_times(loaded_traces, temp.packets)<<endl;
	l_oh_stats.close();
	l_stats.str("");


	status = read_trace(bl_bfile.str(), bl_tfile.str(), &temp);
	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/size.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.total_bytes<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/length.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.length<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/time.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.ttc<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/up.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.up_bytes<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/down.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<temp.down_bytes<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/rem_bytes.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<compute_remaining_bytes(loaded_traces, temp.packets)<<endl;
	l_oh_stats.close();
	l_stats.str("");

	l_stats<<"./ST_Stats/"<<threshold<<"_"<<loaded_trials<<"/BL_OH/rem_times.stats";
	l_oh_stats.open(l_stats.str(), ios::app|ios::out);
	l_oh_stats<<site_no<<","<<compute_remaining_times(loaded_traces, temp.packets)<<endl;
	l_oh_stats.close();
	l_stats.str("");

	log<<"Trace stats written to disk"<<endl;
	return 0;

}
