#! /usr/bin/env python
"""
Example to parse pcap file
     ./parse_pcap.py filename
"""

import sys
#from pcapfile import savefile
import string
import time
import socket
import struct
import asterix


if __name__=='__main__':

    if len(sys.argv) < 2:
        print 'usage: parse_pcap.py <filename>'
        sys.exit(0)
        
    #p = pcap.pcapObject()
    filename = sys.argv[1]
    #p.open_offline(filename)
    
    asterix.init("/home/damir/workspace/asterix/install/config")

    exit(0)
    
    filecap = open(filename, 'rb')
    capfile = savefile.load_savefile(filecap, layers=4, verbose=True)

    print(capfile)
    print(capfile.packets[0].packet.payload)

    #lst = asterix.parse(capfile.packets[0].packet.payload)
    #for l in lst:
    #    print l

    
    if False:
        for i in range(0, len(capfile.packets)):
            try:
                print(capfile.packets[i].packet.payload)
                print(pkt)
                #lst = asterix.parse(pkt.packet.payload)
                #for l in lst:
                #    print l
                
                
            except KeyboardInterrupt:
                print '%s' % sys.exc_type
                print 'parsing canceled by user'
