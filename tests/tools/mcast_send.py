#!/usr/bin/env python3
"""
Send a file as UDP multicast datagrams on the loopback interface (input test of 'asterix -i').

usage: mcast_send.py <group> <port> <file> [count]    the whole file is one datagram, sent count times
"""
import socket
import sys
import time


def main():
    if len(sys.argv) not in (4, 5):
        sys.exit(__doc__)
    group, port, path = sys.argv[1], int(sys.argv[2]), sys.argv[3]
    count = int(sys.argv[4]) if len(sys.argv) == 5 else 1
    data = open(path, 'rb').read()
    s = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_IF, socket.inet_aton('127.0.0.1'))
    s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_LOOP, 1)
    s.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 1)
    for _ in range(count):
        s.sendto(data, (group, port))
        time.sleep(0.1)


if __name__ == '__main__':
    main()
