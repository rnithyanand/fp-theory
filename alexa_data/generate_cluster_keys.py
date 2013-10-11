#!/usr/bin/env python

"""
	take each url in the inputfile,
	find the first level domain name
"""

import sys

def convert(fname):

	d = dict()
	converted = list()

	f = open(fname, 'r')
	web = 0
	for line in f:
		web += 1
		l = line.strip().lower()
		arr = l.split('/')
		tmp = arr[2].split('.')
		i = 0
		if(len(tmp) >= 3 and tmp[0] == 'www'):
			i = 1
		url = ''
		for x in range(i, len(tmp)-1):
			url += tmp[x]+'.'
		url += tmp[len(tmp)-1]
		
		converted.append(url)
		if(url in d):
			d[url].append(web)
		else:
			d[url] = [web]

	f.close()

	with open('./cluster_keys.txt', 'w') as f:
		for x in converted:
			f.write(x+'\n')

#	for k,v in d.items():
#		if(len(v) > 1):
#			print k
#			for x in v:
#				print x

if __name__ == '__main__':
	if len(sys.argv) != 2:
		print "usage: python generate_cluster_keys.py input_file"
		sys.exit(-1)
	convert(sys.argv[1])
