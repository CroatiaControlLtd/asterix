import _asterix
import os
from pkg_resources import Requirement, resource_filename


def set_callback(callback):
    return _asterix.set_callback(callback)


def hello(world):
    return _asterix.hello(world)


def init(filename):
    return _asterix.init(filename)


def parse(data):
    return _asterix.parse(data)


# default callback function
def callback(arg):
    for a in arg:
        print(a)

# initialize asterix with default configuration files

filepath = resource_filename(Requirement.parse("asterix"), "install/config")

for fn in sorted(os.listdir(filepath)):
     f = os.path.join(filepath, fn)
     if os.path.isfile(f) and f.endswith(".xml"):
        _asterix.init(f)

# set default callback
_asterix.set_callback(callback)
