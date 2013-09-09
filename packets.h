#ifndef PACKETS_H
#define PACKETS_H

#include <iostream>

struct packet
{
	long long int size;
	long long int time;
	int direction;
};

void printPacket(packet p);

packet initPacket(long long int size, long long int time, int direction);
packet initPacket();

#endif
