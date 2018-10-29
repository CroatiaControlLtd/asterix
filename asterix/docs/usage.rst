.. _usage:

Usage
=====

Import the asterix module::

    import asterix

Prepare your asterix binary packet. E.g.::

    asterix_packet = bytearray([0x30, 0x00, 0x30, 0xfd, 0xf7, 0x02, 0x19, 0xc9, 0x35, 0x6d, 0x4d, 0xa0, 0xc5, 0xaf, 0xf1, 0xe0,
                        0x02, 0x00, 0x05, 0x28, 0x3c, 0x66, 0x0c, 0x10, 0xc2, 0x36, 0xd4, 0x18, 0x20, 0x01, 0xc0, 0x78,
                        0x00, 0x31, 0xbc, 0x00, 0x00, 0x40, 0x0d, 0xeb, 0x07, 0xb9, 0x58, 0x2e, 0x41, 0x00, 0x20, 0xf5])

Parse the packet with asterix module. It will return the list of Asterix records::

    parsed = asterix.parse(asterix_packet)
    print(parsed)

If you do not need description of items and meaning of bit values, you can disable the verbose mode, to just get values::

    parsed = asterix.parse(asterix_packet, verbose=false)
    print(parsed)

parse_with_offset(data, offset=0, blocks_count=1000):
	
	Parse raw asterix data with bytes offset with returning number of blocks of data passed with arguments
    Args:
        data: Bytes to be parsed
        offset: bytes offset
        blocks_count: number of blocks data to be returned
    Returns:
        tuple of two elements:
            list of asterix records
            bytes offset at ending of computation

If you want to receive textual presentation of asterix packet instead of list use 'text' parameter::

    formatted = asterix.parse(asterix_packet, 'text')
    print(formatted)

All Asterix categories are defined with XML configuration file.
You can get a list of configuration files ith following command::

    config = asterix.list_configuration_files()
    print(config)

And then get specific configuration file::

    configfile = asterix.get_configuration_file('cat048')
    with open(configfile, "rt") as f:
        config = f.read()
        print(config)

If you want to create your own Asterix definition file, create XML file using dtd definition::

    dtd = asterix.get_configuration_file('dtd')


And then initialize asterix with your configuration::

    asterix.init(path_to_your_config_file)

