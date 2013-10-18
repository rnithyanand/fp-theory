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
	ofstream stats;
	stats.open("selected.stats", ios::app|ios::out);
	int site = atoi(argv[1]), status = -1;
	trace temp;
	stringstream byte_file, time_file;

	byte_file<<"./Top500C/Selected/BOPT"<<site<<".size";
	time_file<<"./Top500C/Selected/BOPT"<<site<<".time";
	status = read_trace(byte_file.str(), time_file.str(), &temp);

	byte_file.str("");
	time_file.str("");
	stats<<"Site: "<<site<<", BOPT B: "<<temp.total_bytes<<", BOPT T: "<<temp.ttc<<", BOPT L: "<<temp.length;
	byte_file<<"./Top500C/Selected/LOPT"<<site<<".size";
	time_file<<"./Top500C/Selected/LOPT"<<site<<".time";
	status = read_trace(byte_file.str(), time_file.str(), &temp);
	stats<<", LOPT B: "<<temp.total_bytes<<", LOPT T: "<<temp.ttc<<", LOPT L: "<<temp.length<<endl;

	stats.close();

	return 0;
}
