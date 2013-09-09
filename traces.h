#ifndef TRACES_H
#define TRACES_H

#include <iostream>
#include <vector>
#include "packets.h"

struct trace
{
	int is_supertrace; //flag to indicate if trace is a supertrace
	long long int length; //number of packets contained in trace (always use vector.size()!)
	long long int total_bytes; //number of bytes in entire trace
	long long int up_bytes, down_bytes; //number of bytes in each direction
	long long int ttc; //time to completion
	std::vector<packet> packets; //packets contained in trace

	//Set the following fields iff is_supertrace == 1
	long long int no_contained; //number of contained traces
	std::vector<long long int> tr_completion_time; //time at which each trace is completed by the supertrace
	double bw_oh, up_bw_oh, down_bw_oh, latency_oh; //overheads of the supertrace
	double lbound_bw_oh; //lower bound overhead for the supertrace
};

void printTrace(trace tr);
void printST(trace st);
void printTraceShort(trace tr);
void printSTShort(trace tr);

void initTrace(trace tr, trace *t);
trace initTrace(long long int tr_length, packet *p);
trace initTrace(std::vector<packet> p);
trace initTrace();

trace initST(trace tr);
trace initST(std::vector<packet> p, std::vector<trace> tr);
trace initST(long long int tr_length, packet *p, long long int no_contained, trace *tr);
//trace initST(long long int tr_length, packet *p, std::vector<long long int> tr_completion_time);
//trace initST(std::vector<packet> p, std::vector<long long int> tr_completion_time);

std::vector<long long int> compute_completion_times(std::vector<packet> p, std::vector<trace> tr);
std::vector<long long int> compute_min_bytes(std::vector<packet> p, std::vector<trace> tr);
double compute_bw_oh(std::vector<packet> p, std::vector<trace> tr);
double compute_up_bw_oh(std::vector<packet> p, std::vector<trace> tr);
double compute_down_bw_oh(std::vector<packet> p, std::vector<trace> tr);
double compute_lbound_bw_oh(std::vector<trace> tr);
double compute_latency_oh(std::vector<long long int> completion_times, std::vector<trace> tr);


#endif
