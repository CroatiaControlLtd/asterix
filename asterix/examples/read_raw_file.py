__author__ = 'dsalanti'

import asterix

print(asterix.__version__)

# Read example file from packet resources
sample_filename = asterix.get_sample_file('cat062cat065.raw')

with open(sample_filename, "rb") as f:
    data = f.read()

    # Parse data description=True
    print('Items with description')
    print('----------------------')
    parsed = asterix.parse(data)
    for packet in parsed:
        for item in packet.items():
            print(item)

    print('Items without description')
    print('----------------------')
    # Parse data description=False
    parsed = asterix.parse(data, verbose=False)
    for packet in parsed:
        for item in packet.items():
            print(item)

    print('Textual description of data')
    print('----------------------')
    # describe Asterix data
    formatted = asterix.describe(parsed)
    print(formatted)
