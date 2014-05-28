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
Development Status :: 5 - Production/Stable
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
                    sources = ['./asterix.c', 
                               './python_wrapper.c', 
                               '../asterix/AsterixDefinition.cpp',
                               '../asterix/AsterixData.cpp',
                               '../asterix/Category.cpp',
                               '../asterix/DataBlock.cpp',
                               '../asterix/DataRecord.cpp',
                               '../asterix/DataItem.cpp',
                               '../asterix/DataItemBits.cpp',
                               '../asterix/DataItemDescription.cpp',
                               '../asterix/DataItemFormat.cpp',
                               '../asterix/DataItemFormatCompound.cpp',
                               '../asterix/DataItemFormatExplicit.cpp',
                               '../asterix/DataItemFormatFixed.cpp',
                               '../asterix/DataItemFormatRepetitive.cpp',
                               '../asterix/DataItemFormatVariable.cpp',
                               '../asterix/InputParser.cpp',
                               '../asterix/Tracer.cpp',
                               '../asterix/UAP.cpp',
                               '../asterix/UAPItem.cpp',
                               '../asterix/Utils.cpp',
                               '../asterix/XMLParser.cpp',
                               '../asterix/WiresharkWrapper.cpp'], 
                    
                    include_dirs = ['./python', './lib', '../asterix', '../engine'],
                    extra_compile_args=['-D_GNU_SOURCE', '-DWIRESHARK_WRAPPER', '-DLINUX'],
                    extra_link_args=['-lexpat'])

def get_version():
    filename = os.path.join(os.path.dirname(__file__), './version.h')
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
       download_url="http://github.com/CroatiaControlLtd/asterix",
       license="GPL",
       platforms=['any'],      
       url="http://github.com/CroatiaControlLtd/asterix",
       classifiers=CLASSIFIERS,
       )

if sys.version_info[0] >= 3:
    print( "*" * 100)
    print("If you want to run the tests be sure to run 2to3 on them first, "
          "e.g. `2to3 -w tests/tests.py`.")
    print("*" * 100)

