#ifndef FILE_OPS_H
#define FILE_OPS_H

#include <iostream>
#include <string>
#include <vector>
#include "traces.h"

int write_trace(trace src, std::string dest_bytes, std::string dest_times);
int read_trace(std::string src_bytes, std::string src_times, trace *dest);
trace read_trace(std::string src_bytes, std::string src_times);
const std::string currentDateTime();
long long int findFileSize(std::string filename);

#endif

