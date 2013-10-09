#!/usr/bin/env python

"""
	sort x.time and x.size according to x.time
"""

import sys

def sortst(sizename, timename):

	pairs = list()

	ftime = open(timename, 'r')
	for x in ftime:
		pairs.append([(int)(x.strip())])
	ftime.close()

	fsize = open(sizename, 'r')
	i = 0
	for x in fsize:
		pairs[i].append((int)(x.strip()))
		i += 1
	fsize.close()

	pairs = sorted(pairs)

	ftime = open(timename, 'w')
	for x in pairs:
		ftime.write(str(x[0])+"\n")
	ftime.close()

	fsize = open(sizename, 'w')
	for x in pairs:
		fsize.write(str(x[1])+"\n")
	fsize.close()

if __name__ == '__main__':
	
	if(len(sys.argv) != 3):
		print 'usage: python sort_st.py start_web end_web'
		sys.exit(-1)
	
	s = (int)(sys.argv[1])
	e = (int)(sys.argv[2])

	for x in range(s, e+1):
		sizename = str(x)+'.size'
		timename = str(x)+'.time'
		sortst(sizename, timename)
