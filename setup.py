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
'Development Status :: 2 - Pre-Alpha',
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

                    include_dirs = ['./asterix/python', './src/asterix', './src/engine', './src/main'],
                    extra_compile_args=['-D_GNU_SOURCE', '-DPYTHON_WRAPPER', '-DLINUX'],
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
    
#config_files = [os.path.join('./install/config/', f) for f in listdir('./install/config/') if os.path.isfile(os.path.join('./install/config/', f))]

config_files = ['./install/config/asterix_bds.xml',
                './install/config/asterix_cat001_1_1.xml',
                './install/config/asterix_cat002_1_0.xml',
                './install/config/asterix_cat004_1_8.xml',
                './install/config/asterix_cat008_1_0.xml',
                './install/config/asterix_cat010_1_1.xml',
                './install/config/asterix_cat019_1_2.xml',
                './install/config/asterix_cat020_1_7.xml',
                './install/config/asterix_cat021_1_8.xml',
                './install/config/asterix_cat023_1_2.xml',
                './install/config/asterix_cat030_6_2.xml',
                './install/config/asterix_cat031_6_2.xml',
                './install/config/asterix_cat032_7_0.xml',
                './install/config/asterix_cat034_1_26.xml',
                './install/config/asterix_cat048_1_21.xml',
                './install/config/asterix_cat062_1_16.xml',
                './install/config/asterix_cat063_1_3.xml',
                './install/config/asterix_cat065_1_3.xml',
                './install/config/asterix_cat247_1_2.xml',
                './install/config/asterix_cat252_7_0.xml'
                ]

#eager_files = config_files

#eager_files = [os.path.join('config/', f) for f in listdir('./install/config/') if os.path.isfile(os.path.join('./install/config/', f))]

sample_files = ['./install/sample_data/cat048.raw',
                './install/sample_data/cat062cat065.raw',
                './install/sample_data/cat_034_048.pcap',
                './install/sample_data/cat_062_065.pcap']

setup (name = 'asterix',
       packages = ['asterix'],
       version = get_version(),
       description = "ASTERIX decoder in Python",
       keywords = "asterix, eurocontrol, radar, track, croatiacontrol",
       long_description = README,
       ext_modules = [asterix_module],
       data_files = [('asterix/config', config_files), ('asterix/sample_data', sample_files)],
#       eager_resources = eager_files,
       author="Damir Salantic",
       author_email="damir.salantic@crocontrol.hr",
       download_url="https://github.com/CroatiaControlLtd/asterix",
       license="GPL",
       platforms=['any'],
       url="https://github.com/CroatiaControlLtd/asterix",
       classifiers=CLASSIFIERS,
       )
