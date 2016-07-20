#!/usr/bin/python

import asterix

from pkg_resources import Requirement, resource_filename

def callback(arg):
	for a in arg:
		print(a)
asterix.set_callback(callback)

#filename = resource_filename(Requirement.parse("asterix"),"config/asterix.ini")
#print(filename)
#asterix.init(filename)

samplefilename = resource_filename(Requirement.parse("asterix"),"sample_data/cat_062_065.pcap")
print(samplefilename)
asterix.load(samplefilename)
