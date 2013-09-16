#include <iostream>
#include <vector>
#include <stdlib.h>
#include <math.h>
#include "packets.h"
#include "traces.h"
#include <limits.h>
#include <algorithm>
using namespace std;

void printTraceShort(trace tr)
{
	cout<<"Total packets: "<<tr.length<<", time: "<<tr.ttc<<", bytes: "<<tr.total_bytes<<", upbytes: "<<tr.up_bytes<<", downbytes: "<<tr.down_bytes<<endl;
}

void printSTShort(trace tr)
{
	cout<<"Total packets: "<<tr.length<<", time: "<<tr.ttc<<", bytes: "<<tr.total_bytes<<", upbytes: "<<tr.up_bytes<<", downbytes: "<<tr.down_bytes;
	cout<<", BW OH: "<<tr.bw_oh<<", L OH: "<<tr.latency_oh<<endl;
}

void printTrace(trace tr)
{
	for(int i = 0 ; i < tr.packets.size() ; i ++)
		cout<<"p# "<<i<<", "<<tr.packets[i].size*tr.packets[i].direction<<" at "<<tr.packets[i].time<<endl;
	cout<<"Total packets: "<<tr.length<<", time: "<<tr.ttc<<", bytes: "<<tr.total_bytes<<", upbytes: "<<tr.up_bytes<<", downbytes: "<<tr.down_bytes<<endl;
}

void printST(trace tr)
{
	for(int i = 0 ; i < tr.packets.size() ; i ++)
		cout<<"p# "<<i<<", "<<tr.packets[i].size*tr.packets[i].direction<<" at "<<tr.packets[i].time<<endl;
	cout<<"Total packets: "<<tr.length<<", time: "<<tr.ttc<<", bytes: "<<tr.total_bytes<<", upbytes: "<<tr.up_bytes<<", downbytes: "<<tr.down_bytes<<endl;
	cout<<"BW OH: "<<tr.bw_oh<<", BW UP OH: "<<tr.up_bw_oh<<", BW DN OH: "<<tr.down_bw_oh<<", L OH: "<<tr.latency_oh<<endl;
}


trace initTrace()
{
	trace t;
	packet p = initPacket();
	
	t.is_supertrace = 0;
	t.packets.push_back(p);
	t.length = t.packets.size();
	t.total_bytes = 0;
	t.up_bytes = 0;
	t.down_bytes = 0;
	t.ttc = 0;	
}

trace initTrace(trace tr)
{
	trace t;
	t.is_supertrace = 0; //any call to this function will create a NON-SUPERTRACE! Even if tr is a ST!
	t.length = tr.length;
	t.total_bytes = tr.total_bytes;
	t.up_bytes = tr.up_bytes;
	t.down_bytes = tr.down_bytes;
	t.ttc = tr.ttc;
	t.packets = tr.packets;
	return t;
}

trace initTrace(long long int tr_length, packet *p)//NOTE: tr_length is the #packets, not the index of the last element!
{
	trace t;
	for(int i = 0 ; i < tr_length ; i ++)
		t.packets.push_back(p[i]);

	t.is_supertrace = 0;
	t.length = t.packets.size();
	t.ttc = t.packets[t.length-1].time;

	t.total_bytes = 0;
	t.up_bytes = 0;
	t.down_bytes = 0;

	for(int i = 0 ; i < tr_length ; i ++)
	{
		t.total_bytes += t.packets[i].size;
		if(t.packets[i].direction == 1)
			t.up_bytes += t.packets[i].size;
		else
			t.down_bytes += t.packets[i].size;
	}
	
	return t;
}

trace initTrace(vector<packet> p)
{
	trace t;
	t.is_supertrace = 0;
	t.length = p.size();
	t.packets = p;
	t.ttc = p[t.length - 1].time;
	
	t.total_bytes = 0;
	t.up_bytes = 0;
	t.down_bytes = 0;

	for(int i = 0 ; i < t.length ; i ++)
	{
		t.total_bytes += t.packets[i].size;
		if(t.packets[i].direction == 1)
			t.up_bytes += t.packets[i].size;
		else
			t.down_bytes += t.packets[i].size;
	}
	
	return t;
}

trace initST(trace tr)
{
	trace t;
	if(tr.is_supertrace == 0)
	{
		t = initTrace(tr);
		return t;
	}
	else
	{
		t.is_supertrace = 1;
		t.length = tr.length;
		t.total_bytes = tr.total_bytes;
		t.up_bytes = tr.up_bytes;
		t.down_bytes = tr.down_bytes;
		t.ttc = tr.ttc;
		t.packets = tr.packets;
		t.no_contained = tr.no_contained;
		t.tr_completion_time = tr.tr_completion_time;
		t.bw_oh = tr.bw_oh;
		t.up_bw_oh = tr.up_bw_oh;
		t.down_bw_oh = tr.down_bw_oh;
		t.latency_oh = tr.latency_oh;
		t.lbound_bw_oh = tr.lbound_bw_oh;
		return t;
	}
}

trace initST(vector<packet> p, vector<trace> tr)
{
	trace t = initTrace(p);
	t.is_supertrace = 1;
	t.no_contained = tr.size();//NOT THE INDEX OF LAST TRACE! ACTUAL NUMBER CONTAINED!
	t.tr_completion_time = compute_completion_times(p, tr);
	t.bw_oh = compute_bw_oh(p, tr);
	t.up_bw_oh = compute_up_bw_oh(p, tr);
	t.down_bw_oh = compute_down_bw_oh(p, tr);
	t.lbound_bw_oh = compute_lbound_bw_oh(tr);
	t.latency_oh = compute_latency_oh(t.tr_completion_time, tr);
	return t;
}

trace initST(long long int tr_length, packet *p, long long int no_contained, trace *tr)
{
	trace t = initTrace(tr_length, p);
	t.is_supertrace = 1;
	t.no_contained = no_contained;
	vector<trace> tr_vector;
	for(int i = 0 ; i < no_contained ; i ++)
		tr_vector.push_back(tr[i]);
	t.tr_completion_time = compute_completion_times(t.packets, tr_vector);
	t.bw_oh = compute_bw_oh(t.packets, tr_vector);
	t.up_bw_oh = compute_up_bw_oh(t.packets, tr_vector);
	t.down_bw_oh = compute_down_bw_oh(t.packets, tr_vector);
	t.lbound_bw_oh = compute_lbound_bw_oh(tr_vector);
	t.latency_oh = compute_latency_oh(t.tr_completion_time, tr_vector);
	return t;
}

/*trace initST(long long int tr_length, packet *p, vector<long long int> tr_completion_time)
{
	
}

trace initST(vector<packet> p, vector<long long int> completion_time)
{
	
}
*/

double compute_bw_oh(vector<packet> p, vector<trace> tr)
{
	long long int st_bytes = 0, tr_bytes = 0; 
	double overhead = 0;
	long long int additional_bytes = 0;

	for(long long int i = 0 ; i < p.size() ; i ++)
		st_bytes += p[i].size;

	vector<long long int> min_bytes = compute_min_bytes(p, tr);
	st_bytes = 0;
	for(long long int i = 0 ; i < tr.size() ; i ++)
		st_bytes += min_bytes[i];

	for(long long int i = 0 ; i < tr.size() ; i ++)
		tr_bytes += tr[i].total_bytes;
	/*
	for(int i = 0 ; i < tr.size() ; i ++)
		for(int j = 0 ; j < tr[i].packets.size(); j++)
			tr_bytes += tr[i].packets[j].size;
	*/
	overhead = (double)((double)st_bytes/(double)tr_bytes);
	return overhead;
}

double compute_up_bw_oh(vector<packet> p, vector<trace> tr)
{
	long long int st_upbytes = 0, tr_upbytes = 0; 
	double overhead = 0;

	for(int i = 0 ; i < p.size() ; i ++)
		if(p[i].direction == 1)
			st_upbytes += p[i].size;

	for(int i = 0 ; i < tr.size() ; i ++)
		for(int j = 0 ; j < tr[i].packets.size() ; j ++)
			if(tr[i].packets[j].direction == 1)
				tr_upbytes += tr[i].packets[i].size;

	overhead = (double)((double)st_upbytes*(double)tr.size()/(double)tr_upbytes);
	return overhead;
}

double compute_down_bw_oh(vector<packet> p, vector<trace> tr)
{
	long long int st_downbytes = 0, tr_downbytes = 0;
	double overhead = 0;

	for(int i = 0 ; i < p.size() ; i ++)
		if(p[i].direction == -1)
			st_downbytes += p[i].size;

	for(int i = 0 ; i < tr.size() ; i ++)
		for(int j = 0 ; j < tr[i].packets.size() ; j ++)
			if(tr[i].packets[j].direction == -1)
				tr_downbytes += tr[i].packets[i].size;

	overhead = (double)((double)st_downbytes*(double)tr.size()/(double)tr_downbytes);
	return overhead;
}

double compute_latency_oh(vector<long long int> completion_times, vector<trace> tr)
{
	long long int st_latency = 0, tr_latency = 0;
	double overhead = 0;

	for(int i = 0 ; i < completion_times.size() ; i ++)
		st_latency += completion_times[i];

	for(int i = 0 ; i < tr.size() ; i ++)
		if(completion_times[i] != -1)
			tr_latency += tr[i].ttc;

	overhead = (double)((double)st_latency/(double)tr_latency);
	return overhead;
}

double compute_lbound_bw_oh(vector<trace> tr)
{
	long long int max_up = LLONG_MIN, max_down = LLONG_MIN;
	long long int tr_bytes = 0;
	//find the largest upstream and downstream traces
	for(int i = 0 ; i < tr.size() ; i++)
	{
		tr_bytes += tr[i].total_bytes;
		if(tr[i].up_bytes >= max_up)
			max_up = tr[i].up_bytes;
		if(tr[i].down_bytes >= max_down)
			max_down = tr[i].down_bytes;
	}
	long long int st_bytes = (max_up + max_down)*tr.size();
	double overhead = (double)((double)st_bytes/(double)tr_bytes);
	return overhead;
}
//This function goes through a series of packets $p$ and returns the times that each trace in $tr$ is completed//
vector<long long int> compute_completion_times(vector<packet> p, vector<trace> tr)
{
	vector<long long int> completion_times;
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
	for(int i = 0 ; i < no_traces ; i ++) //For this to work, all traces must have their first packet sent at time t=1 (NOT t=0).
		if(time[i] == -1)
		{
			time[i] = curr_time + 3*(tr[i].packets[tr[i].packets.size()-1].time - tr[i].packets[current_packet[i]-1].time);
//			cout<<"Trace "<<i<<" was not complete. Current Packet: "<<current_packet[i]<<", Current Time: "<<curr_time<<", Last Packet: "<<tr[i].packets.size()-1<<", Min time required: "<<tr[i].packets[tr[i].packets.size()-1].time-tr[i].packets[current_packet[i]].time<<", # remaining packets: "<<tr[i].packets.size()-1 - current_packet[i]<<endl;
//			cout<<"Total Time Required (LB) : "<<time[i]<<", Original Trace Time: "<<tr[i].packets[tr[i].packets.size()-1].time<<", Net extra time: "<<time[i]-tr[i].packets[tr[i].packets.size()-1].time<<endl;
		}
	for(int i = 0 ; i < no_traces ; i ++)
		completion_times.push_back(time[i]);
	return completion_times;
}
//This function goes through a series of packets $p$ and returns the min number of bytes transmitted for every trace in the system//
vector<long long int> compute_min_bytes(vector<packet> p, vector<trace> tr)
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
		{
			if(remaining_bytes[j] > 0 && tr[j].packets[current_packet[j]].direction == curr_direction && tr[j].packets[current_packet[j]].time <= curr_time)
			{
				completed_bytes[j] += curr_size;
				remaining_bytes[j] -= curr_size;
			}
		}
	}
	vector<long long int> min_bytes;
	for(long long int i = 0 ; i < no_traces ; i ++)
	{
		min_bytes.push_back(tr[i].total_bytes + (3*remaining_bytes[i]));
	}
	return min_bytes;
}
