#include <iostream>
#include <stdlib.h>
#include <string>
#include <fstream>
#include <vector>
#include <math.h>
#include "packets.h"

using namespace std;

packet initPacket(long long int size, long long int time, int direction)
{
	packet p;
	p.size = abs(size);
	p.time = abs(time);
	p.direction = direction;

	return p;
}

packet initPacket()
{
	packet p;
	p.size = 0;
	p.time = 0;
	p.direction = 1;
	
	return p;
}

void printPacket(packet p)
{
	cout<<"Packet Size: "<<p.size<<", Time: "<<p.time<<", Direction: "<<p.direction<<endl;
}
