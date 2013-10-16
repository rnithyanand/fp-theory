#include <iostream>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>

#include <stdlib.h>
#include <limits.h>
#include <math.h>

#include "traces.h"
#include "supertrace_helpers.h"

using namespace std;

/* 
        Looks at all packets in the upstream/downstream direction in the current frontier and finds the median time. If there are no packets in the given direction, returns -1.
*/
long long int findPercentileTime(double percentile, vector<long long int> frontier, vector<trace> t, int direction, vector<long long int> next_available_time)
{
        vector<long long int> times;
        for(long long int i = 0 ; i < t.size() ; i ++)
                if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == direction)
                        times.push_back(next_available_time[i]);
        sort(times.begin(), times.end());
	if(times.size() == 0)
		return -1;
	else
        	return (times[percentile*(times.size()-1)]);
}

/*
	Function that checks if there are any packets in the given direction in the current frontier. Returns 1 if there are, and 0 otherwise.
*/
int isPacketInDirection(vector<long long int> frontier, vector<trace> t, int direction)
{
	for(long long int i = 0 ; i < t.size() ; i ++)
		if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == direction)
			return 1;
	return 0;
}

/* 
        Function that looks through all the frontier packets that are within the given threshold time and finds the majority direction. 
        Returns 1 for upstream and -1 downstream. In case of a tie, goes with upstream (since these are fewer in general).
*/
int findMajorityDirection(vector<long long int> frontier, vector<trace> t, long long int thresh_time, vector<long long int> next_available_time)
{
        long long int up_count = 0, down_count = 0;
        for(long long int i = 0 ; i < t.size() ; i ++)
                if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == 1 && next_available_time[i] <= thresh_time)
                        up_count++; 
                else if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == -1 && next_available_time[i] <= thresh_time)
                        down_count++;
        if(up_count >= down_count)
                return 1;       
        else    
                return -1;
}       

/*
	Function that looks through all the frontier packets that are within the given threshold time and finds the majority direction weighted by trace length.
	Returns 1 for upstream and -1 for downstream. In case of a tie, it goes with upstream.
*/
int findMajorityLengthWtdDirection(vector<long long int> frontier, vector<trace> t, long long int thresh_time, vector<long long int> next_available_time)
{
	long long int up = 1, down = 1;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == 1 && next_available_time[i] <= thresh_time)
			up += t[i].packets.size() - frontier[i];
		else if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == -1 && next_available_time[i] <= thresh_time)
			down += t[i].packets.size() - frontier[i];
	}
	if(up >= down)
		return 1;
	else
		return -1;
}

/*
	Function that looks through all the frontier packets that are within the gien threshold time and finds the majority direction weighted by the number of bytes that are still to be sent.
*/
int findMajorityByteWtdDirection(vector<long long int> frontier, vector<trace> t, long long int thresh_time, vector<long long int> next_available_time)
{
	long long int up = 1, down = 1;
	for(long long int i = 0 ; i < t.size() ; i ++)
	{
		if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == 1 && next_available_time[i] <= thresh_time)
			for(long long int p_no = frontier[i] ; p_no < t[i].packets.size() ; p_no++)
				up+=t[i].packets[p_no].size;
		else if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == -1 && next_available_time[i] <= thresh_time)
			for(long long int p_no = frontier[i]; p_no < t[i].packets.size() ; p_no++)
				down += t[i].packets[p_no].size;
	}
	if(up >= down)
		return 1;
	else
		return -1;
}

/*
        Function to find the maximum size packet from all the frontier packets in the given direction.
*/
long long int findMaxSize(vector<long long int> frontier, vector<trace> t, int direction, long long int thresh_time, vector<long long int> next_available_time)
{
        long long int max = INT_MIN;
        for(long long int i = 0 ; i < t.size() ; i ++)
                if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == direction && t[i].packets[frontier[i]].size >= max && next_available_time[i] <= thresh_time)
                        max = t[i].packets[frontier[i]].size;
        return max;
}
/*
	Function to find the minimum size packet from all the frontier packets in the given direction.
*/
long long int findMinSize(vector<long long int> frontier, vector<trace> t, int direction, long long int thresh_time, vector<long long int> next_available_time)
{
	long long int min = INT_MAX;
	for(long long int i = 0 ; i < t.size() ; i ++)
		if(frontier[i] <t[i].packets.size() && t[i].packets[frontier[i]].direction == direction && t[i].packets[frontier[i]].size <= min && next_available_time[i] <= thresh_time)
			min = t[i].packets[frontier[i]].size;
	return min;
}

/*
        Function to find the time at which a packet should be sent.
*/
long long int findPacketTime(vector<long long int> frontier, vector<trace> t, int direction, double time_multiplier, long long int median_time, vector<long long int> next_available_time)
{
	long long int max_time = INT_MIN;
	for(long long int i = 0 ; i < t.size() ; i ++)
		if(frontier[i] < t[i].packets.size() && t[i].packets[frontier[i]].direction == direction  && next_available_time[i] <= median_time && next_available_time[i]> max_time)
			max_time = next_available_time[i];
	return max_time;
}

/*
	Function that counts the number of traces that are complete by counting the number of set flags.
*/
long long int countCompleteTraces(vector<int> complete_flags)
{
	long long int count = 0;
	for(long long int i = 0 ; i < complete_flags.size() ; i ++)
		if(complete_flags[i] == 1)
			count++;
	return count;
}

/*
	Function that returns which traces are complete (i.e., returns a vector of flags)
*/
vector<int> whichTracesComplete(vector<long long int> frontier, vector<trace> t)
{
	vector<int> flags(t.size());
	for(long long int i = 0 ; i < t.size() ; i ++)
		if(frontier[i] == t[i].packets.size())
			flags[i] = 1;
	return flags;
}
