#!/usr/bin/python

__author__ = 'dsalanti'

import socket
import struct
import asterix

sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
sock.bind(('', 21111))
mreq = struct.pack("=4sl", socket.inet_aton("232.1.1.11"), socket.INADDR_ANY)
sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

counter=1

while True:
    asterix_packet = sock.recv(10240)
    parsed = asterix.parse(asterix_packet)
    print('%d. Receiver received = %s' % (counter, parsed))
    counter += 1

