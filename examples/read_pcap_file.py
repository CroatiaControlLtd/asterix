__author__ = 'dsalanti'

import asterix
#import dpkt
from pkg_resources import Requirement, resource_filename

# Read example file from packet resources
sample_filename = resource_filename(Requirement.parse("asterix"), "install/sample_data/cat_034_048.pcap")
f = open(sample_filename)
#pcap = dpkt.pcap.Reader(f)

#for ts, buf in pcap:
#    print ts, len(buf)

# Parse data
#parsed = asterix.parse(data)
#print(parsed)

# parse and print formatted packet
#formatted = asterix.parse(data, 'text')
#print(formatted)
