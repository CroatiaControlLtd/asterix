import _asterix
import os
from pkg_resources import Requirement, resource_filename

#def set_callback(callback):
#    return _asterix.set_callback(callback)


#def hello(world):
#    return _asterix.hello(world)


def init(filename):
    """ Initializes asterix with category definition

    :param filename: Path to XML definition file
    :return: 0 if successful
    """
    return _asterix.init(filename)


def parse(data, format=None):
    """ Parse raw asterix data
    :param data: Bytes to be parsed
    :param format: Format of data returned:
                    'list' - return list of asterix data (default)
                    'text' - formatted text
    :return: list of asterix records
    """
    parsed = _asterix.parse(data)
    if format == 'text':
        i = 0
        txt = ''
        for record in parsed:
            i+=1
            txt += '\n\nAsterix record: %d ' % i
            txt += '\nCategory: %d' % record['category']
            for key, value in record.items():
                if key != 'category':
                    txt += '\nItem '
                    txt += str(key)
                    if isinstance(value, dict):
                        for ikey, ival in value.items():
                            txt += '\n\t%s (%s): %s' % (ikey, ival['desc'], ival['val'])
                    elif isinstance(value, list):
                        for it in value:
                            for ikey, ival in it.items():
                                txt += '\n\t%s (%s): %s' % (ikey, ival['desc'], ival['val'])
                    else:
                        txt += str(value)

        return txt
    else:
        return parsed


# default callback function
#def callback(arg):
#    for a in arg:
#        print(a)

# initialize asterix with default configuration files
filepath = resource_filename(Requirement.parse("asterix"), "install/config")
for fn in sorted(os.listdir(filepath)):
     f = os.path.join(filepath, fn)
     if os.path.isfile(f) and f.endswith(".xml"):
        _asterix.init(f)

# set default callback
#_asterix.set_callback(callback)
