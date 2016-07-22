__author__ = 'dsalanti'

import asterix
from pkg_resources import Requirement, resource_filename

# Read example file from packet resources
sample_filename = resource_filename(Requirement.parse("asterix"), "install/sample_data/cat062cat065.raw")
f = open(sample_filename, "rb")
data = f.read()

# Parse data
parsed = asterix.parse(data)
print(parsed)

# parse and print formatted packet
formatted = asterix.parse(data, 'text')
print(formatted)
