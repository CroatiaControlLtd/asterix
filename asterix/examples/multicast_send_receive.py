#!/usr/bin/python

__author__ = 'dsalanti'

import threading
import time
import socket
import struct
import asterix

exitFlag = False

asterix_packet = bytes([0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9, 0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
                        0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10, 0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
                        0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb, 0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5])


class SenderThread(threading.Thread):

    def __init__(self, threadID, name, counter):
        """
        Initialize thread
        :param threadID:
        :param name:
        :param counter:
        :return:
        """
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.counter = counter
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_MULTICAST_TTL, 2)

    def run(self):
        """
        Send asterix packet
        :return:
        """
        print("Starting " + self.name)
        while self.counter:
            if exitFlag:
                self.name.exit()
            time.sleep(0.1)
            print("%d. Sender sending : %s" % (self.counter, str(asterix_packet)))
            self.sock.sendto(asterix_packet, ("224.51.105.104", 5000))
            self.counter -= 1
        print("Exiting " + self.name)


class ReceiverThread (threading.Thread):

    def __init__(self, threadID, name, counter):
        """
        Initialize receiver thread
        :param threadID:
        :param name:
        :param counter:
        :return:
        """
        threading.Thread.__init__(self)
        self.threadID = threadID
        self.name = name
        self.counter = counter
        self.sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
        self.sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)

    def run(self):
        """
        Receive asterix packets
        :return:
        """
        print("Starting " + self.name)

        self.sock.bind(('', 5000))
        mreq = struct.pack("=4sl", socket.inet_aton("224.51.105.104"), socket.INADDR_ANY)
        self.sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

        while self.counter:
            if exitFlag:
                self.name.exit()
            asterix_packet = self.sock.recv(10240)
            parsed = asterix.parse(asterix_packet)

            #print('%d. Receiver received = %s' % (self.counter, parsed))
            print(asterix.describe(parsed))
            self.counter -= 1

        print("Exiting " + self.name)

# Create new threads
repeat = 100000
sender_thread = SenderThread(1, "Sender", repeat)
receiver_thread = ReceiverThread(1, "Receiver", repeat)

# Start new Threads
sender_thread.start()
receiver_thread.start()

print("Exiting Main Thread")