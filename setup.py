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
                                './src/asterix/InputParser.cpp',
                                './src/asterix/Tracer.cpp',
                                './src/asterix/UAP.cpp',
                                './src/asterix/UAPItem.cpp',
                                './src/asterix/Utils.cpp',
                                './src/asterix/XMLParser.cpp',
                                './src/asterix/asterixformat.cxx',
                                './src/asterix/asterixrawsubformat.cxx',
                                './src/asterix/asterixpcapsubformat.cxx',
                                './src/asterix/asterixfinalsubformat.cxx',
                                './src/asterix/asterixhdlcsubformat.cxx',
                                './src/asterix/asterixhdlcparsing.c',
                                './src/engine/converterengine.cxx',
                                './src/engine/channelfactory.cxx',
                                './src/engine/devicefactory.cxx',
                                './src/engine/tcpdevice.cxx',
                                './src/engine/udpdevice.cxx',
                                './src/engine/serialdevice.cxx',
                                './src/engine/diskdevice.cxx',
                                './src/engine/stddevice.cxx',
                                './src/engine/descriptor.cxx'
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
    
setup (name = 'asterix',
       version = get_version(),
       description = "ASTERIX decoder in Python",
       long_description = README,
       ext_modules = [module1],
       author="Damir Salantic",
       author_email="damir.salantic@crocontrol.hr",
       download_url="https://github.com/CroatiaControlLtd/asterix",
       license="GPL",
       platforms=['any'],      
       url="https://github.com/CroatiaControlLtd/asterix",
       classifiers=CLASSIFIERS,
       )

if sys.version_info[0] >= 3:
    print( "*" * 100)
    print("If you want to run the tests be sure to run 2to3 on them first, "
          "e.g. `2to3 -w tests/tests.py`.")
    print("*" * 100)

