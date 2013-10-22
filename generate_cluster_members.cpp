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
	stringstream input, output;
	vector<string> cluster_list;
	string temp, file_name;
	int cl_list_temp, index = 1;

	input<<"./cluster_data/method_3_69.txt";
	input_file.open(input.str());
	input.str("");

	while(!input_file.eof())
	{
		getline(input_file, temp);
		if(!temp.empty() && temp != " " && temp != "\r\n")
			cluster_list.push_back(temp);
		index++;
	}
	cout<<cluster_list.size()<<endl;	
	for(int i = 0 ; i < no_cl ; i ++)
	{
		output<<"./cluster_data/HT_3/members_"<<i<<"."<<no_cl;
		members_file.open(output.str(), ios::trunc|ios::out);
		output.str("");
		for(int j = 0 ; j < cluster_list.size() ; j ++)
		{
			if(cluster_list[j] == to_string(i))
				members_file<<j+1<<endl;
		}
		members_file.close();
	}	

	input_file.close();
	return 1;	
}
