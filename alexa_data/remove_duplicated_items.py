#!/usr/bin/env python

"""
	remove duplicated lines in the input file fname
	dup_fname stores the duplicated lines
	dup_removed_fname is the output file
"""

import sys


def detect_dup(fname):
	new = list()
	hs = set()
	dup = set()

	f = open(fname, 'r')
	for line in f:
		l = line.strip().lower()
		if(l in hs):
			dup.add(l)
		else:
			hs.add(l)
			new.append(l)
	f.close()

	if len(dup) == 0:
		print 'no duplicated items'
		return

	fclean = open('dup_removed_'+fname, 'w')
	for x in new:
		fclean.write(x+'\n')
	fclean.close()

	print str(len(dup))+' duplicated items detected'

	fdup = open('dup_'+fname, 'w')
	for x in dup:
		fdup.write(x+'\n')
	fdup.close()


if __name__ == '__main__':
	if len(sys.argv) != 2:
		print "usage: python remove_duplicated_items.py input_file"
		sys.exit(-1)
	detect_dup(sys.argv[1])
