#!/usr/bin/python

import asterix


def callback(arg):
	print('Callback')
	for a in arg:
		print(a)

asterix.set_callback(callback)
asterix.hello("World")
#asterix.init("/home/damir/asterix_python2/asterix/install/config/asterix.ini")
#asterix.parse("/home/damir/asterix_python2/asterix/install/sample_data/cat_062_065.pcap")
asterix.init("/home/dsalanti/workspace/python_asterix/asterix/install/config/asterix.ini")
asterix.parse("/home/dsalanti/workspace/python_asterix/asterix/install/sample_data/cat_062_065.pcap")
