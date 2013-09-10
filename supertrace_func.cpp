#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "file_ops.h"
#include "supertrace_helpers.h"
#include "supertrace_func.h"

using namespace std;

trace frontierMax(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_supertrace_func.frontierMax.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	////log<<"Function call: frontierMax() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		////log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		////log<<"ST Packet number: "<<st_packets.size()<<endl;
		////log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			////log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			////log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			////log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		////log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		////log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		////log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMaxPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.frontierMaxPT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	////log<<"Function call: frontierMaxPT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		////log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		////log<<"ST Packet number: "<<st_packets.size()<<endl;
		////log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			////log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			////log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else if(st_packets.size() > last_down + p_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			last_down = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			////log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			////log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		////log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		////log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		////log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}


trace frontierMaxTT(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.frontierMaxTT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMaxTT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else if(last_time > last_down + t_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			last_down = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		last_time = curr_time;
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace frontierMaxPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.frontierMaxPTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMaxPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMaxTT_UP(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.frontierMaxTTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMaxTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		last_time = curr_time;
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}
trace frontierMin(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_supertrace_func.frontierMin.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMin() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMinPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.frontierMinPT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMinPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else if(st_packets.size() > last_down + p_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Downstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_down = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMinTT(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.frontierMinTT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMinTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else if(last_time > last_down + t_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Downstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_down = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;
		last_time = curr_time;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMinPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.frontierMinPTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMinPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace frontierMinTT_UP(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.frontierMinTTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: frontierMinTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;
		last_time = curr_time;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMin(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_supertrace_func.trLenWtdMin.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMin() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trLenWtdMinPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMinPT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMinPT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else if(st_packets.size() > last_down + p_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Downstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_down = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trLenWtdMinTT(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMinTT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMinTT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else if(last_time > last_down + t_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Downstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_down = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;
		last_time = curr_time;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trLenWtdMinPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMinPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMinPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMinTT_UP(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMinTT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMinTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;
		last_time = curr_time;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trByteWtMin(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_supertrace_func.trByteWtdMin.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMin() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trByteWtMinPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMinPT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMinPT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else if(st_packets.size() > last_down + p_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Downstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_down = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;


}

trace trByteWtMinTT(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMinTT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMinTT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else if(last_time > last_down + t_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Downstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_down = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;
		last_time = curr_time;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;


}

trace trByteWtMinPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMinPT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMinPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trByteWtMinTT_UP(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMinTT_UP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMinTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	vector<long long int> bytes_remaining;
	for(long long int i = 0 ; i < t.size() ; i ++)
		bytes_remaining.push_back(t[i].packets[frontier[i]].time);
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;
		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;
		last_time = curr_time;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMinSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
				bytes_remaining[i] -= curr_size;
			
			if(bytes_remaining[i] <= 0 && frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
				bytes_remaining[i] = t[i].packets[frontier[i]].size;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trLenWtdMax(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_supertrace_func.trLenWtdMax.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMax() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trLenWtdMaxPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMaxPT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMaxPT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else if(st_packets.size() > last_down + p_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			last_down = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trLenWtdMaxTT(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMaxTT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMaxTT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else if(last_time > last_down + t_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			last_down = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		last_time = curr_time;
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;


}

trace trLenWtdMaxPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMaxPTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMaxPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trLenWtdMaxTT_UP(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trLenWtdMaxTTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trLenWtdMaxTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityLengthWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		last_time = curr_time;
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trByteWtMax(long long int threshold, vector<trace> t, double time_multiplier)
{
	string log_file = "log_supertrace_func.trByteWtdMax.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMax() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/

	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;

	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;
}

trace trByteWtMaxPT(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMaxPT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMaxPT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else if(st_packets.size() > last_down + p_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			last_down = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trByteWtMaxTT(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.ByteWtdMaxTT.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMaxTT() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else if(last_time > last_down + t_thresh)
		{
			curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			last_down = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		last_time = curr_time;
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;


}

trace trByteWtMaxPT_UP(long long int threshold, vector<trace> t, double time_multiplier, int p_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMaxPTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMaxPT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(st_packets.size() > last_up + p_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = st_packets.size();
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}

trace trByteWtMaxTT_UP(long long int threshold, vector<trace> t, double time_multiplier, int t_thresh)
{
	string log_file = "log_supertrace_func.trByteWtdMaxTTUP.txt";
	ofstream log;
	log.open(log_file, ios::app|ios::out);
	//log<<"Function call: trByteWtdMaxTT_UP() at time: "<<currentDateTime()<<endl;
	if(threshold > t.size())
	{
		//log<<"Error: Threshold higher than number of traces in input vector! The returned supertrace is garbage."<<endl;
		return t[0];
	}
	/*
		frontier: points to the next packet that needs to be considered by the supertrace.
		starts at 0. If the frontier is at trace.packets.size(), it means that all packets
		in the trace have been covered by the supertrace.
	*/
	vector<long long int> frontier(t.size());
	vector<long long int> next_available_time;/*time at which the next packet will be available for a trace*/
	for(long long int i = 0 ; i < t.size() ; i ++)
		next_available_time.push_back(t[i].packets[frontier[i]].time);
	vector<int> complete_flags(t.size());
	vector<packet> st_packets;
	vector<long long int> completion_times;
	long long int last_up = 0, last_down = 0, last_time = 0;
	while(countCompleteTraces(complete_flags) <= threshold)
	{
		//log<<"ST Packet number: "<<st_packets.size()<<endl;
		//log<<"Current Threshold: "<<threshold<<", Current Completion Count: "<<countCompleteTraces(complete_flags)<<endl;
		/*	1. Check if there are packets in both directions. If there are not, then set the direction for the current packet.
			   Check if the threshold has been reached for either direction.
			   If there are, then find the median times for next available packets in both directions.
			   Find the majority direction using the lower median time * fixed multiplier. This is the direction.	*/
		int both_directions_valid = 0;
		int curr_direction = 0;
		long long int median_time = 0;
		if(isPacketInDirection(frontier, t, 1) == 1 && isPacketInDirection(frontier, t, -1) == 1)
			both_directions_valid = 1;
		if(both_directions_valid == 0)
		{
			if(isPacketInDirection(frontier, t, 1) == 1)
				curr_direction = 1;
			else
				curr_direction = -1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Single direction frontier, Median time: "<<median_time<<endl;
			
		}
		else if(last_time > last_up + t_thresh)
		{
			curr_direction = 1;
			median_time = findMedianTime(frontier, t, curr_direction, next_available_time);
			//log<<"Upstream packets have waited too long -- threshold exceeded. Inserting upstream packet now"<<endl;
			last_up = last_time;
		}
		else
		{
			long long int up_time, down_time;
			up_time = findMedianTime(frontier, t, 1, next_available_time);
			//log<<"Multi-direction frontier."<<endl;
			down_time = findMedianTime(frontier, t, -1, next_available_time);
			median_time = min(up_time, down_time);
			//log<<"Median UP Time: "<<up_time<<", Median DN Time: "<<down_time<<", Chosen Median Time: "<<median_time<<endl;
			curr_direction = findMajorityByteWtdDirection(frontier, t, median_time * time_multiplier, next_available_time);
		}
		//log<<"Current ST packet direction: "<<curr_direction<<endl;

		/*	2. Find the actual packet time given this direction and threshold time.		*/
		long long int curr_time;
		curr_time = findPacketTime(frontier, t, curr_direction, time_multiplier, median_time, next_available_time);
		last_time = curr_time;
		//log<<"Current ST packet time: "<<curr_time<<endl;

		/*	3. Find the max packet size in the same direction and within the determined send time.		*/
		long long int curr_size;
		curr_size = findMaxSize(frontier, t, curr_direction, curr_time, next_available_time);
		//log<<"Current ST packet size: "<<curr_size<<endl;
		
		packet temp = initPacket(curr_size, curr_time, curr_direction);
		st_packets.push_back(temp);
	
		/*	4. Compute next available packet time.	5. Update Frontiers	*/
		long long int ipt = 0;
		for(long long int i = 0 ; i < t.size() ; i ++)
		{
			if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == curr_direction && next_available_time[i] <= curr_time)
			{
				if(frontier[i] < t[i].packets.size()-1)
					ipt = t[i].packets[frontier[i]+1].time - t[i].packets[frontier[i]].time;
				next_available_time[i] = curr_time + ipt;
				frontier[i]++;
			}
			//next_available_time[i] = curr_time + ipt;
		}
		/*	6. Check if any traces are completed. Update the completion times and flags if needed.		*/
		complete_flags = whichTracesComplete(frontier, t);		
	}
	trace st = initST(st_packets, t);
	printSTShort(st);
	log.close();
	return st;

}
