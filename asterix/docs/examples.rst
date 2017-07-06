.. _examples:

Examples
========

Parse the binary Asterix packet::

    import asterix

    # This is binary presentation of asterix packet of CAT048
    #                  len  cat 048
    #                 ---- ---------
    asterix_packet = bytearray([0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9, 0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
                            0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10, 0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
                            0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb, 0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5])


    # parse and print packet
    parsed = asterix.parse(asterix_packet)
    print(parsed)

    # print formatted packet
    # If you want to see textual presentation of asterix packet use describe::
	print(asterix.describe(parsed))


Parse the raw Asterix file::

    import asterix

    # Read example file from packet resources
    sample_filename = asterix.get_sample_file('cat062cat065.raw')

    with open(sample_filename, "rb") as f:
        data = f.read()

        # Parse data
        parsed = asterix.parse(data)
        print(parsed)

        # print formatted packet
		print(asterix.describe(parsed))

		
Parse Pcap file with the help of dpkt module (only available for Python 2)::

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


Listen to UDP multicast stream and print parsed Asterix data::

    import socket
    import struct
    import asterix

    sock = socket.socket(socket.AF_INET, socket.SOCK_DGRAM, socket.IPPROTO_UDP)
    sock.setsockopt(socket.SOL_SOCKET, socket.SO_REUSEADDR, 1)
    sock.bind(('', 21111))
    mreq = struct.pack("=4sl", socket.inet_aton("232.1.1.11"), socket.INADDR_ANY)
    sock.setsockopt(socket.IPPROTO_IP, socket.IP_ADD_MEMBERSHIP, mreq)

    while True:
        asterix_packet = sock.recv(10240)
        data = asterix.parse(asterix_packet)
        print(data)
