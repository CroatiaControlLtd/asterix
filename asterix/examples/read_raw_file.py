__author__ = 'dsalanti'

import asterix

# Read example file from packet resources
sample_filename = asterix.get_sample_file('cat062cat065.raw')

with open(sample_filename, "rb") as f:
    data = f.read()

    # Parse data
    parsed = asterix.parse(data)
    print(parsed)

    # describe Asterix data
    formatted = asterix.describe(parsed)
    print(formatted)
