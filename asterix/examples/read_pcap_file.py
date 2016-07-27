__author__ = 'dsalanti'

import asterix
import dpkt

# Read example file from packet resources
sample_filename = asterix.get_sample_file('cat_034_048.pcap')
with open(sample_filename) as f:
    pcap = dpkt.pcap.Reader(f)

    cntr=1
    for ts, buf in pcap:
        eth = dpkt.ethernet.Ethernet(buf)
        data = eth.ip.udp.data

        hexdata = ":".join("{:02x}".format(ord(c)) for c in str(data))
        print('Parsing packet %d : %s' %(cntr, hexdata))
        cntr += 1

        # Parse data
        parsed = asterix.parse(data)
        print(parsed)

