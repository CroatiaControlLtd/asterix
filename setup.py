try:
  from setuptools import setup, Extension
except ImportError:
  from distutils.core import setup, Extension
import distutils.sysconfig
import shutil
import os.path
import re
import sys

CLASSIFIERS = filter(None, map(str.strip,
"""
Development Status :: 2 - Pre-Alpha
Intended Audience :: Developers
License :: OSI Approved :: GPL License
Programming Language :: C
Programming Language :: Python :: 2.4
Programming Language :: Python :: 2.5
Programming Language :: Python :: 2.6
Programming Language :: Python :: 2.7
Programming Language :: Python :: 3
Programming Language :: Python :: 3.2
""".splitlines()))

try:
    shutil.rmtree("./build")
except(OSError):
    pass

module1 = Extension('asterix',
                    sources = ['./asterix/asterix_c/asterix.c', 
                                './asterix/asterix_c/python_wrapper.c', 
                                './asterix/asterix_c/python_parser.cpp', 
                                './lib/asterix/src/asterix/AsterixDefinition.cpp',
                                './lib/asterix/src/asterix/AsterixData.cpp',
                                './lib/asterix/src/asterix/Category.cpp',
                                './lib/asterix/src/asterix/DataBlock.cpp',
                                './lib/asterix/src/asterix/DataRecord.cpp',
                                './lib/asterix/src/asterix/DataItem.cpp',
                                './lib/asterix/src/asterix/DataItemBits.cpp',
                                './lib/asterix/src/asterix/DataItemDescription.cpp',
                                './lib/asterix/src/asterix/DataItemFormat.cpp',
                                './lib/asterix/src/asterix/DataItemFormatCompound.cpp',
                                './lib/asterix/src/asterix/DataItemFormatExplicit.cpp',
                                './lib/asterix/src/asterix/DataItemFormatFixed.cpp',
                                './lib/asterix/src/asterix/DataItemFormatRepetitive.cpp',
                                './lib/asterix/src/asterix/DataItemFormatVariable.cpp',
                                './lib/asterix/src/asterix/InputParser.cpp',
                                './lib/asterix/src/asterix/Tracer.cpp',
                                './lib/asterix/src/asterix/UAP.cpp',
                                './lib/asterix/src/asterix/UAPItem.cpp',
                                './lib/asterix/src/asterix/Utils.cpp',
                                './lib/asterix/src/asterix/XMLParser.cpp',
                                './lib/asterix/src/asterix/asterixformat.cxx',
                                './lib/asterix/src/asterix/asterixrawsubformat.cxx',
                                './lib/asterix/src/asterix/asterixpcapsubformat.cxx',
                                './lib/asterix/src/asterix/asterixfinalsubformat.cxx',
                                './lib/asterix/src/asterix/asterixhdlcsubformat.cxx',
                                './lib/asterix/src/asterix/asterixhdlcparsing.c',
                                './lib/asterix/src/engine/converterengine.cxx',
                                './lib/asterix/src/engine/channelfactory.cxx',
                                './lib/asterix/src/engine/devicefactory.cxx',
                                './lib/asterix/src/engine/tcpdevice.cxx',
                                './lib/asterix/src/engine/udpdevice.cxx',
                                './lib/asterix/src/engine/serialdevice.cxx',
                                './lib/asterix/src/engine/diskdevice.cxx',
                                './lib/asterix/src/engine/stddevice.cxx',
                                './lib/asterix/src/engine/descriptor.cxx'
                               ],
                    
                    include_dirs = ['./asterix/asterix_c', './lib/asterix/src/asterix', './lib/asterix/src/engine', './lib/asterix/src/main'],
                    extra_compile_args=['-D_GNU_SOURCE', '-DPYTHON_WRAPPER', '-DLINUX'],
                    extra_link_args=['-lexpat'])

def get_version():
    filename = os.path.join(os.path.dirname(__file__), './asterix/asterix_c/version.h')
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
    
setup (name = 'asterix',
       version = get_version(),
       description = "ASTERIX decoder in Python",
       long_description = README,
       ext_modules = [module1],
       author="Damir Salantic",
       author_email="damir.salantic@crocontrol.hr",
       download_url="https://github.com/dsalantic/pyasterix",
       license="GPL",
       platforms=['any'],      
       url="https://github.com/dsalantic/pyasterix",
       classifiers=CLASSIFIERS,
       )

if sys.version_info[0] >= 3:
    print( "*" * 100)
    print("If you want to run the tests be sure to run 2to3 on them first, "
          "e.g. `2to3 -w tests/tests.py`.")
    print("*" * 100)

