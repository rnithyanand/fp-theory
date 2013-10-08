#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h>
#include <algorithm>

#include "file_ops.h"

using namespace std;

int main(int argc, char *argv[])
{
	ofstream log;
	log.open("log_stats.input.txt", ios::app|ios::out);
	long long int site_no = atol(argv[1]);
	long long int trial_no = atol(argv[2]);

	int status = -1;
	stringstream bfile, tfile;
	trace temp;

	bfile<<"./input_data/"<<site_no<<"_"<<trial_no<<".cap.txt";
	tfile<<"./input_data/timeseq_"<<site_no<<"_"<<trial_no<<".cap.txt";
	status = read_trace(bfile.str(), tfile.str(), &temp);
	log<<"Input trace load status = "<<status<<endl;
	bfile.str("");
	tfile.str("");

	stringstream input_stats;
	input_stats<<"./input_stats/total_bytes.stats";
	ofstream data;
	data.open(input_stats.str(), ios::app|ios::out);
	data<<site_no<<", "<<trial_no<<", "<<temp.total_bytes<<endl;
	input_stats.str("");
	data.close();

	input_stats<<"./input_stats/time.stats";
	data.open(input_stats.str(), ios::app|ios::out);
	data<<site_no<<", "<<trial_no<<", "<<temp.ttc<<endl;
	input_stats.str("");
	data.close();

	input_stats<<"./input_stats/up_bytes.stats";
	data.open(input_stats.str(), ios::app|ios::out);
	data<<site_no<<", "<<trial_no<<", "<<temp.up_bytes<<endl;
	input_stats.str("");
	data.close();

	input_stats<<"./input_stats/trace_lengths.stats";
	data.open(input_stats.str(), ios::app|ios::out);
	data<<site_no<<", "<<trial_no<<", "<<temp.down_bytes<<endl;
	input_stats.str("");
	data.close();
	
	log<<"Stats computed for site: "<<site_no<<" and trial number: "<<trial_no<<endl;
	return 0;	
}
