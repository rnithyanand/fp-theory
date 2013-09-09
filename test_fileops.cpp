#include <iostream>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "traces.h"
#include "packets.h"
#include "file_ops.h"

using namespace std;


/*
	test_fileops.cpp: input -- site number, trial number; output -- log files and trace info
	reads input traces into memory and writes them back into a log file - "file_test.txt".	
*/
int main(int argc, char *argv[])
{
	ofstream log;
	log.open("log_test_fileops.txt", ios::app|ios::out);
	if(!log.is_open())
		return -1;
	int site, trial;
	site = atoi(argv[1]);
	trial = atoi(argv[2]);
	stringstream bfile, tfile;
	bfile<<"./input_data/"<<site<<"_"<<trial<<".cap.txt";
	tfile<<"./input_data/timeseq_"<<site<<"_"<<trial<<".cap.txt";
	log<<"Reading files: "<<bfile.str()<<" and "<<tfile.str()<<endl;
	trace tr;
	int status;
	status = read_trace(bfile.str(), tfile.str(), &tr);
	//tr = read_trace(bfile.str(), tfile.str());
	printTraceShort(tr);
	//log<<"Status: "<<status<<endl<<"Writing trace to file"<<endl;
	status = write_trace(tr, "logfile_test_bytes.txt", "logfile_test_times.txt");
	log<<"Status: "<<status<<endl<<"Test complete for site "<<site<<" and trial "<<trial<<endl;
	log.close();
	return 0;
}
