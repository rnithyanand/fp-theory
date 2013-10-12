#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <sstream>
#include <math.h>
#include <vector>
#include <time.h>

using namespace std;

int main(int argc, char *argv[])
{
	long long int no_cl = atol(argv[1]);
	long long int no_sites = atol(argv[2]);
	ofstream members_file;
	ifstream input_file;
	stringstream input;
	vector<string> cluster_list;
	string temp, file_name;
	stringstream output;
	int cl_list_temp, index = 1;

	input<<"./cluster_data/"<<no_cl<<"_indexed.txt";
	input_file.open(input.str());

	while(!input_file.eof())
	{
		getline(input_file, temp);
		if(!temp.empty() && temp != " " && index!=no_sites && temp != "\r")
			cluster_list.push_back(temp);
		index++;
	}

	for(int i = 1 ; i <= no_cl ; i ++)
	{
		input<<"members_"<<i<<"."<<no_cl;
		members_file.open(input.str(), ios::trunc|ios::out);
		for(int j = 0 ; j < cluster_list.size() ; j ++)
		{
			cout<<cluster_list[j]<<", "<<endl;
			cl_list_temp = stoi(cluster_list[j]);
			if(cl_list_temp == i)
				members_file<<j<<endl;
		}
		members_file.close();
		input.str("");
	}	

	input_file.close();
	return 1;	
}
