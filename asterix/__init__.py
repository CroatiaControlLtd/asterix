import _asterix
import os
import sys

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


def describe(category, item=None, field=None, value=None):
    """
    Return the description of specific Category, Item, Field and Value in Asterix specification
    :param category: Asterix category (e.g. 62)
    :param item: Asterix Item ID (e.g. 'I010')
    :param field: Asterix field name (e.g. 'SAC')
    :param value: Asterix field value meaning (e.g. 1)
    :return:
    """
    if value:
        return _asterix.describe(category, item, field, value)
    elif field:
        return _asterix.describe(category, item, field)
    elif item:
        return _asterix.describe(category, item)
    return _asterix.describe(category)

def parse(data):
    """ Parse raw asterix data
    :param data: Bytes to be parsed
    :return: list of asterix records
    """
    if sys.version_info <= (2, 7):
        return _asterix.parse(buffer(data))
    return _asterix.parse(bytes(data))


def describe(parsed):
    """
    Describe all elements in Asterix data in textual format.
    :param parsed: Parsed Asterix packet returned by ateris.parse
    :return: Formatted string describing all Asterix data
    """
    i = 0
    txt = ''
    for record in parsed:
        i+=1
        txt += '\n\nAsterix record: %d ' % i
        cat = record['category']
        txt += '\nCategory: %d (%s)' % (cat, _asterix.describe(cat))
        for key, value in record.items():
            if key != 'category':
                txt += '\nItem: %s (%s)' % (str(key), _asterix.describe(cat, str(key)))
                if isinstance(value, dict):
                    for ikey, ival in value.items():
                        txt += '\n\t%s (%s): %s %s' % (ikey, _asterix.describe(cat, str(key), ikey), str(ival['val']), _asterix.describe(cat, str(key), ikey, str(ival['val'])))
                elif isinstance(value, list):
                    for it in value:
                        for ikey, ival in it.items():
                            txt += '\n\t%s (%s): %s %s' % (ikey, _asterix.describe(cat, str(key), ikey), str(ival['val']), _asterix.describe(cat, str(key), ikey, str(ival['val'])))
                else:
                    txt += str(value)

    return txt


def list_sample_files():
    """
    Return the list of Asterix format sample files from the package
    :return: list of Asterix sample files
    """
    sample_files = []
    filepath = os.path.join(os.path.dirname(__file__), 'sample_data')
    for fn in sorted(os.listdir(filepath)):
         f = os.path.join(filepath, fn)
         if os.path.isfile(f):
            sample_files.append(f)
    return sample_files


def get_sample_file(match):
    """
    Returns first Asterix sample file matching the parameter string
    :param match: Search string for sample file
    :return: Sample file path
    """
    filepath = os.path.join(os.path.dirname(__file__), 'sample_data')
    for fn in sorted(os.listdir(filepath)):
         f = os.path.join(filepath, fn)
         if os.path.isfile(f) and match in fn:
            return f


def list_configuration_files():
    """
    Return the list of Asterix configuration files from the package
    :return: list of Asterix configuration files
    """
    sample_files = []
    filepath = os.path.join(os.path.dirname(__file__), 'config')
    for fn in sorted(os.listdir(filepath)):
         f = os.path.join(filepath, fn)
         if os.path.isfile(f):
            sample_files.append(f)
    return sample_files


def get_configuration_file(match):
    """
    Returns first Asterix configuration file matching the parameter string
    :param match: Search string for configuration file
    :return: Configuration file path
    """
    filepath = os.path.join(os.path.dirname(__file__), 'config')
    for fn in sorted(os.listdir(filepath)):
         f = os.path.join(filepath, fn)
         if os.path.isfile(f) and match in fn:
            return f


# default callback function
#def callback(arg):
#    for a in arg:
#        print(a)

# initialize asterix with default configuration files

filepath = os.path.join(os.path.dirname(__file__), 'config')
for fn in sorted(os.listdir(filepath)):
     f = os.path.join(filepath, fn)
     if os.path.isfile(f) and f.endswith(".xml"):
        _asterix.init(f)

# set default callback
#_asterix.set_callback(callback)
