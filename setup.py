try:
  from setuptools import setup, Extension
except ImportError:
  from distutils.core import setup, Extension
import distutils.sysconfig
import shutil
import os.path
from os import listdir
import re
import sys


CLASSIFIERS = [
'Development Status :: 3 - Alpha',
'Intended Audience :: Developers',
'License :: OSI Approved :: GNU General Public License (GPL)',
'Programming Language :: C',
'Programming Language :: C++',
'Programming Language :: Python :: 2.6',
'Programming Language :: Python :: 2.7',
'Programming Language :: Python :: 3.2',
'Programming Language :: Python :: 3.3',
'Programming Language :: Python :: 3.4',
'Programming Language :: Python :: 3.5'
]

try:
    shutil.rmtree("./build")
except(OSError):
    pass


# Remove the "-Wstrict-prototypes" compiler option, which isn't valid for C++.
import distutils.sysconfig
cfg_vars = distutils.sysconfig.get_config_vars()
for key, value in cfg_vars.items():
    if type(value) == str:
        cfg_vars[key] = value.replace("-Wstrict-prototypes", "")
# ==================================

asterix_module = Extension('_asterix',
                    sources = ['./src/python/asterix.c',
                                './src/python/python_wrapper.c', 
                                './src/python/python_parser.cpp', 
                                './src/asterix/AsterixDefinition.cpp',
                                './src/asterix/AsterixData.cpp',
                                './src/asterix/Category.cpp',
                                './src/asterix/DataBlock.cpp',
                                './src/asterix/DataRecord.cpp',
                                './src/asterix/DataItem.cpp',
                                './src/asterix/DataItemBits.cpp',
                                './src/asterix/DataItemDescription.cpp',
                                './src/asterix/DataItemFormat.cpp',
                                './src/asterix/DataItemFormatCompound.cpp',
                                './src/asterix/DataItemFormatExplicit.cpp',
                                './src/asterix/DataItemFormatFixed.cpp',
                                './src/asterix/DataItemFormatRepetitive.cpp',
                                './src/asterix/DataItemFormatVariable.cpp',
                                './src/asterix/DataItemFormatBDS.cpp',
                                './src/asterix/InputParser.cpp',
                                './src/asterix/Tracer.cpp',
                                './src/asterix/UAP.cpp',
                                './src/asterix/UAPItem.cpp',
                                './src/asterix/Utils.cpp',
                                './src/asterix/XMLParser.cpp',
                               ],

                    include_dirs = ['./asterix/python', './src/asterix', './src/engine'],
                    extra_compile_args=['-DPYTHON_WRAPPER'],
                    extra_link_args=['-lexpat'])


def get_version():
    filename = os.path.join(os.path.dirname(__file__), './src/python/version.h')
    file = None
    try:
        file = open(filename)
        header = file.read()
    finally:
        if file:
            file.close()
    m = re.search(r'#define\s+PYASTERIX_VERSION\s+"(\d+\.\d+(?:\.\d+)?)"', header)
    assert m, "version.h must contain PYASTERIX_VERSION macro"
    return m.group(1)

f = open('README.rst')
try:
    README = f.read()
finally:
    f.close()    
    
config_files = [os.path.join('./asterix/config/', f) for f in listdir('./asterix/config/') if os.path.isfile(os.path.join('./asterix/config/', f))]

sample_files = ['./asterix/sample_data/cat048.raw',
                './asterix/sample_data/cat062cat065.raw',
                './asterix/sample_data/cat_034_048.pcap',
                './asterix/sample_data/cat_062_065.pcap']

setup (name = 'asterix_decoder',
       packages = ['asterix'],
       version = get_version(),
       description = "ASTERIX decoder in Python",
       keywords = "asterix, eurocontrol, radar, track, croatiacontrol",
       long_description = README,
       ext_modules = [asterix_module],
       #data_files = [('asterix/config', config_files), ('asterix/sample_data', sample_files)],
       include_package_data = True,
       package_data = {'asterix' : config_files+sample_files},
       zip_safe=False,
#       eager_resources = eager_files,
       author="Damir Salantic",
       author_email="damir.salantic@crocontrol.hr",
       download_url="https://github.com/CroatiaControlLtd/asterix",
       license="GPL",
       platforms=['any'],
       url="https://github.com/CroatiaControlLtd/asterix",
       classifiers=CLASSIFIERS,
       )
