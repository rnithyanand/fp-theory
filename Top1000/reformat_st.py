#!/usr/bin/env python

"""
	combine x.time and x.size to a single supertrace file
	need to provide cluster_id as an argument
"""

import sys

def reformat(cluster_id, sizename, timename, outname):
	sizes = list()
	times = list()

	fsize = open(sizename, 'r')
	for x in fsize:
		sizes.append(x.strip())
	fsize.close()

	ftime = open(timename, 'r')
	for x in ftime:
		times.append(x.strip())
	ftime.close()

	fst = open(outname, 'w')
	fst.write(cluster_id+','+str(len(sizes))+',\n')
	for x in sizes:
		fst.write(x+',')
	fst.write('\n')

	for x in times:
		fst.write(x+',')
	fst.write('\n')
	fst.close()

if __name__ == '__main__':
	if(len(sys.argv) != 5):
		print 'usage: python reformat_st.py clusterid size_fname time_fname outputname'
		sys.exit(-1)
	reformat(sys.argv[1], sys.argv[2], sys.argv[3], sys.argv[4])
