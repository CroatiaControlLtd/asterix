
import _asterix

from pkg_resources import Requirement, resource_filename
filename = resource_filename(Requirement.parse("asterix"),"config/asterix.ini")

def callback(arg):
	print('Callback')
	for a in arg:
		print(a)

_asterix.set_callback(callback)

_asterix.init(filename)

def load(filename):
	_asterix.parse(filename)
