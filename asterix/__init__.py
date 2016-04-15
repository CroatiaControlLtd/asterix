import _asterix

from pkg_resources import Requirement, resource_filename
filename = resource_filename(Requirement.parse("asterix"),"config/asterix.ini")
print filename

