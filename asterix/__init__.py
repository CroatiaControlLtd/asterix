
import _asterix

def set_callback(callback):
	return _asterix.set_callback(callback)

def hello(world):
	return _asterix.hello(world)

def init(filename):
	return _asterix.init(filename)

#def load(filename):
#	_asterix.parse(filename)

def parse(data):
	return _asterix.parse(data)

# default callback function
def callback(arg):
	for a in arg:
		print(a)

# initialize asterix with default configuration
from pkg_resources import Requirement, resource_filename
filename = resource_filename(Requirement.parse("asterix"),"install/config")
_asterix.init(filename)

# set default callback
_asterix.set_callback(callback)
