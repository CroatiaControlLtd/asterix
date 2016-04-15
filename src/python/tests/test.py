#!/usr/bin/python

import asterix

from pkg_resources import Requirement, resource_filename
filename = resource_filename(Requirement.parse("asterix"),"config/asterix.ini")
print filename

def callback(arg):
	print('Callback')
	for a in arg:
		print(a)

asterix.set_callback(callback)
asterix.hello("World")
asterix.init(filename)
#asterix.init("/home/damir/asterix_python2/asterix/install/config/asterix.ini")
#asterix.parse("/home/damir/asterix_python2/asterix/install/sample_data/cat_062_065.pcap")
#asterix.init(_config_location)
asterix.parse("/home/dsalanti/workspace/python_asterix/asterix/install/sample_data/cat_062_065.pcap")
