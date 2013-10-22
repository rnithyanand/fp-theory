#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <stdlib.h>
#include <math.h>
#include <limits.h>
#include "file_ops.h"
#include "supertrace_func.h"
#include "supertrace_helpers.h"

using namespace std;

trace frontierMax(long long threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_frontierMax.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMax @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";

		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size;

		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
			down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
			percentile_time = min(up_time, down_time);
			curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
			//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
			//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
				curr_direction = 1;
			else
				curr_direction = -1;
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}

		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMaxPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMaxPT.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMaxPT @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}	
	
trace frontierMaxPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMaxPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMaxPT_UP @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}	
	
trace frontierMaxPT_DOWN(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMaxPT_DOWN.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMaxPT_DOWN @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMin(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_frontierMin.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMin @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
			down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
			percentile_time = min(up_time, down_time);
			curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
			if(curr_direction == -1)
				last_up = st_packets.size();
			else
				last_down = st_packets.size();
			//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
			//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMinPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMinPT.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMinPT @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace frontierMinPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMinPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMinPT_UP @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace frontierMinPT_DOWN(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMinPT_DOWN.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMinPT_DOWN @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace trByteWtMax(long long threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_trByteMax.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteMax @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";

		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size;

		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
			down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
			percentile_time = min(up_time, down_time);
			curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
			//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
			//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
				curr_direction = 1;
			else
				curr_direction = -1;
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}

		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trByteWtMaxPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trByteMaxPT.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteMaxPT @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

	
trace trByteWtMaxPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trByteMaxPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteMaxPT_UP @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}	
	
trace trByteWtMaxPT_DOWN(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trByteMaxPT_DOWN.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteMaxPT_DOWN @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trByteWtMin(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_trByteWtMin.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteWtMin @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
			down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
			percentile_time = min(up_time, down_time);
			curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
			if(curr_direction == -1)
				last_up = st_packets.size();
			else
				last_down = st_packets.size();
			//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
			//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trByteWtMinPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trByteWtMinPT.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteWtMinPT @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace trByteWtMinPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trByteWtMinPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trByteWtMinPT_UP @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace trByteWtMinPT_DOWN(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_frontierMinPT_DOWN.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : frontierMinPT_DOWN @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityByteWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMax(long long threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_trLenMax.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenMax @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";

		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size;

		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
			down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
			percentile_time = min(up_time, down_time);
			curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
			//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
			//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
				curr_direction = 1;
			else
				curr_direction = -1;
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}

		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMaxPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trLenMaxPT.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenMaxPT @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

	
trace trLenWtdMaxPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trLenMaxPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenMaxPT_UP @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}	
	
trace trLenWtdMaxPT_DOWN(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trLenMaxPT_DOWN.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenMaxPT_DOWN @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = t[i].packets[frontier[i]].size - curr_size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			while(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] >= t[i].packets[frontier[i]].size)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] += ipt;
				deficit[i] = curr_size - t[i].packets[frontier[i]].size;
				//log<<"(Full) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				frontier[i]++;
			}
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time && deficit[i] < t[i].packets[frontier[i]].size)
			{
				ipt = 0;
				t[i].packets[frontier[i]].size += deficit[i];
				//log<<"(Part) Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", DEF: "<<deficit[i]<<endl;
				deficit[i] = 0;
			}
			next_available_time[i] = curr_time + ipt;
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMin(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_trLenWtdMin.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenWtdMin @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
			down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
			percentile_time = min(up_time, down_time);
			curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
			if(curr_direction == -1)
				last_up = st_packets.size();
			else
				last_down = st_packets.size();
			//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
			//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMinPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trLenWtdMinPT.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenWtdMinPT @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_down = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace trLenWtdMinPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trLenWtdMinPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenWtdMinPT_UP @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_up + p_thresh)
			{
				curr_direction = -1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (UP). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace trLenWtdMinPT_DOWN(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_trLenMinPT_DOWN.txt";
	ofstream log;
	log.open(log_file, ios::app| ios::out);
	//log<<"Function call : trLenMinPT_DOWN @ "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"ERROR: Threshold higher than number of traces in input vector!"<<endl;
		return t[0];
	}
	
	vector<long long int> deficit(t.size()), frontier(t.size()), next_available_time(t.size()), completion_times(t.size()), bytes_remaining(t.size());
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		next_available_time.push_back(t[i].packets[frontier[i]].time);
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	}

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<endl<<"-----------------------------------------"<<endl;
		//log<<"ST Packet #"<<st_packets.size()<<", Current Threshold: "<<threshold;
		//log<<", Completion Count: "<<countCompleteTraces(complete_flags)<<", ";
		int both_directions_valid = 0, curr_direction = 0;
		long long int percentile_time, up_time, down_time, curr_time, ipt = 0, curr_size, last_up = 0, last_down = 0;
		if(isPacketInDirection(frontier, t, 1) && isPacketInDirection(frontier, t, -1))
		{
			if(st_packets.size() > last_down + p_thresh)
			{
				curr_direction = 1;
				percentile_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);
				last_up = st_packets.size();
				//log<<"\nThreshold Reached (DOWN). Median Time: "<<percentile_time;
				//log<<", Current ST Direction: "<<curr_direction;
			}
			else
			{
				up_time = findPercentileTime(time_multiplier, frontier, t, 1, next_available_time);;
				down_time = findPercentileTime(time_multiplier, frontier, t, -1, next_available_time);;
				percentile_time = min(up_time, down_time);
				curr_direction = findMajorityLengthWtdDirection(frontier, t, percentile_time, next_available_time);;
				if(curr_direction == -1)
					last_up = st_packets.size();
				else
					last_down = st_packets.size();
				//log<<"\nMulti-D Frontier, Median UP Time:"<<up_time<<", Median Down Time:"<<down_time;
				//log<<", Chosen median time: "<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
			}
		}
		else
		{
			if(isPacketInDirection(frontier, t, 1))
			{
				curr_direction = 1;
				last_down = st_packets.size();
			}
			else
			{
				curr_direction = -1;
				last_up = st_packets.size();
			}
			percentile_time = findPercentileTime(time_multiplier, frontier, t, curr_direction, next_available_time);;
			//log<<"\nSingle-D Frontier, Median Time:"<<percentile_time<<endl<<"Current ST Direction: "<<curr_direction;
		}
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, percentile_time, next_available_time);;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);;
		//log<<", Current ST Time: "<<curr_time<<", Current ST Size: "<<curr_size<<endl;
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);

		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				bytes_remaining[i] -= curr_size;
				if(bytes_remaining[i] <= 0)
				{
					if(frontier[i] < t[i].packets.size() - 1)
						ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;	
					next_available_time[i] = curr_time + ipt;
					bytes_remaining[i] = t[i].packets[frontier[i]].size;
					frontier[i]++;
				}
				//log<<"Trace "<<i<<", Frontier @ "<<frontier[i]<<"/"<<t[i].packets.size();
				//log<<", NAT: "<<next_available_time[i]<<", IPT: "<<ipt<<", REM: "<<bytes_remaining[i]<<endl;
			}
		
		}
		complete_flags = whichTracesComplete(frontier, t);
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


