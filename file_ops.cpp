#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#include <time.h>

#include "traces.h"

using namespace std;

long long int findFileSize(string filename)
{
	long long int begin, end;
	ifstream fname(filename);
	begin = fname.tellg();
	fname.seekg (0, ios::end);
	end = fname.tellg();
	fname.close();
	return (end-begin);
}

const string currentDateTime() 
{
    time_t     now = time(0);
    struct tm  tstruct;
    char       buf[80];
    tstruct = *localtime(&now);
    strftime(buf, sizeof(buf), "%Y-%m-%d.%X", &tstruct);
    return buf;
}

/*
	function writes the bytes and times in src to the two files
	returns: 0 -> error free, 1 -> empty trace written, 2-> file IO error
	meta-data/debug info in "log_fileops.txt"
*/
int write_trace(trace src, string dest_bytes, string dest_times)
{
	ofstream dst_bfile, dst_tfile, log_file;
	dst_bfile.open(dest_bytes, ios::trunc|ios::out);
	dst_tfile.open(dest_times, ios::trunc|ios::out);
	log_file.open("log_fileops.txt", ios::app|ios::out);

	if((!dst_bfile.is_open())||(!dst_tfile.is_open())||(!log_file.is_open()))
		return 2;

	log_file<<"Function call: write_trace() at time: "<<currentDateTime()<<endl;
	log_file<<"Writing trace to files: "<<dest_bytes<<"(bytes) and"<<dest_times<<"(times)"<<endl;

	if(src.length > 0)
	{
		for(int i = 0 ; i < src.length ; i ++)
		{
			dst_bfile<<src.packets[i].size*src.packets[i].direction<<endl;
			dst_tfile<<src.packets[i].time<<endl;
			log_file<<"p#:"<<i<<", "<<src.packets[i].size*src.packets[i].direction<<" at "<<src.packets[i].time<<endl;
		}
		log_file<<"End of files: "<<dest_bytes<<", "<<dest_times<<endl;
	}
	else
	{
		dst_bfile<<"0"<<endl;
		dst_tfile<<"0"<<endl;
		log_file<<"p#: 0, 0 at 0"<<endl<<"End of files: "<<dest_bytes<<", "<<dest_times<<endl;
		dst_bfile.close();
		dst_tfile.close();
		log_file.close();
		return 1;
	}
	dst_bfile.close();
	dst_tfile.close();
	log_file.close();
	return 0;
}

trace read_trace(string src_bytes, string src_times)
{
	trace tr = initTrace();
	tr.packets.pop_back();
	
	ifstream src_bfile, src_tfile;
	ofstream log_file;
	src_bfile.open(src_bytes);
	src_tfile.open(src_times);
	log_file.open("log_fileops.txt", ios::app|ios::out);
	int ret_val = 0;
	if((!src_bfile.is_open()) || (!src_tfile.is_open()) || (!log_file.is_open()))
		ret_val = 2;
	
	log_file<<"Function call: read_trace() at time: "<<currentDateTime()<<endl;
	log_file<<"Reading files: "<<src_bytes<<" (bytes) and "<<src_times<<" (times)"<<endl;

	string b, t;
	int p_no = 0;
	long long int start_time = 0;
	long long int time, size;
	int direction;
	vector<packet> p;
	packet temp;

	if(findFileSize(src_bytes) == 0 || findFileSize(src_times) == 0)
	{
		if(p_no == 0)
       		{
                	log_file<<"p#: 0, 0 at 0"<<endl;
	                temp = initPacket();
       		        p.push_back(temp);
                	ret_val =  1;
        	}
	
	}

	while(!src_bfile.eof() && !src_tfile.eof() && ret_val == 0)
	{
		getline(src_bfile, b);
		getline(src_tfile, t);
		if(p_no == 0)
			start_time = stoi(t);
		if(!b.empty() && !t.empty() && b != "" && t!="")
		{
			log_file<<"p#: "<<p_no<<", "<<b<<" at "<<t<<endl;
			time = stoi(t) - start_time;
			size = stoi(b);
			if(size < 0)
				direction = -1;
			else
				direction = 1;
			size = abs(size);
			temp = initPacket(size, time, direction);
			p.push_back(temp);
		}
		p_no++;
	}

	if(p_no == 0)
	{
		log_file<<"p#: 0, 0 at 0"<<endl;
		temp = initPacket();
		p.push_back(temp);
		ret_val =  1;
	}
	else 
		ret_val = 0;
	tr = initTrace(p);
	
	printTrace(tr);
	log_file<<"Finished Reading Files: "<<src_bytes<<" and "<<src_times<<endl;
	src_bfile.close();
	src_tfile.close();
	log_file.close();
	return tr;
	/*dst->packets = tr.packets;
	dst->is_supertrace = tr.is_supertrace;
	dst->length = tr.length;
	dst->total_bytes = tr.total_bytes;
	dst->up_bytes = tr.up_bytes;
	dst->down_bytes = tr.down_bytes;
	dst->ttc = tr.ttc;
	*/
	//return ret_val;

}

/* 
	function reads the bytes and times in the two input srcs and writes to memory (as a trace)
	returns: 0 -> error free, 1 -> empty trace read, 2 -> file IO error
	metadata/debug info in "log_fileops.txt"
*/
int read_trace(string src_bytes, string src_times, trace* dst)
{
	trace tr;
	//tr.packets.pop_back();
	
	ifstream src_bfile, src_tfile;
	ofstream log_file;
	src_bfile.open(src_bytes);
	src_tfile.open(src_times);
	log_file.open("log_fileops.txt", ios::app|ios::out);
	
	if((!src_bfile.is_open()) || (!src_tfile.is_open()) || (!log_file.is_open()))
		return 2;
	
	log_file<<"Function call: read_trace() at time: "<<currentDateTime()<<endl;
	log_file<<"Reading files: "<<src_bytes<<" (bytes) and "<<src_times<<" (times)"<<endl;

	int ret_val = 0;
	string b, t;
	int p_no = 0;
	long long int start_time = 0;
	long long int time, size;
	int direction;
	vector<packet> p;
	packet temp;

	if(findFileSize(src_bytes) == 0 || findFileSize(src_times) == 0)
	{
		if(p_no == 0)
       		{
                	log_file<<"p#: 0, 0 at 0"<<endl;
	                temp = initPacket();
       		        p.push_back(temp);
                	ret_val =  1;
        	}
	
	}

	while(!src_bfile.eof() && !src_tfile.eof() && ret_val == 0)
	{
		getline(src_bfile, b);
		getline(src_tfile, t);
		if(p_no == 0)
			start_time = stoi(t);
		if(!b.empty() && !t.empty() && b != "" && t!="")
		{
			log_file<<"p#: "<<p_no<<", "<<b<<" at "<<t<<endl;
			time = stoi(t) - start_time;
			size = stoi(b);
			if(size < 0)
				direction = -1;
			else
				direction = 1;
			size = abs(size);
			temp = initPacket(size, time, direction);
			p.push_back(temp);
		}
		p_no++;
	}

	if(p_no == 0)
	{
		log_file<<"p#: 0, 0 at 0"<<endl;
		temp = initPacket();
		p.push_back(temp);
		ret_val =  1;
	}
	else 
		ret_val = 0;

	tr = initTrace(p);
	p.erase(p.begin(), p.end());
	log_file<<"Finished Reading Files: "<<src_bytes<<" and "<<src_times<<endl;
	src_bfile.close();
	src_tfile.close();
	log_file.close();

	dst->packets = tr.packets;
	dst->is_supertrace = tr.is_supertrace;
	dst->length = tr.length;
	dst->total_bytes = tr.total_bytes;
	dst->up_bytes = tr.up_bytes;
	dst->down_bytes = tr.down_bytes;
	dst->ttc = tr.ttc;

	return ret_val;
}
