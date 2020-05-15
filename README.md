asterix
=======

[![Build Status](https://travis-ci.org/CroatiaControlLtd/asterix.svg?branch=master)](https://travis-ci.org/CroatiaControlLtd/asterix)
[![Documentation badge](https://readthedocs.org/projects/asterix/badge/?version=latest)](http://asterix.readthedocs.io/en/latest/)
[![PyPI](https://img.shields.io/pypi/v/asterix_decoder.svg)](https://pypi.python.org/pypi/asterix_decoder)

Asterix is Python module and standalone application used for reading and parsing of EUROCONTROL ASTERIX protocol data from stdin, file or network multicast stream.

ASTERIX stands for All Purpose STructured EUROCONTROL SuRveillance Information EXchange.
It is an ATM Surveillance Data Binary Messaging Format which allows transmission of harmonised information between any surveillance and automation system.
ASTERIX defines the structure of the data to be exchanged over a communication medium, from the encoding of every bit of information up to the organisation of the data within a block of data - without any loss of information during the whole process.
More about ASTERIX protocol you can find here: http://www.eurocontrol.int/services/asterix

Asterix application was developed by Croatia Control Ltd.

This program comes with ABSOLUTELY NO WARRANTY. This is free software, and you are welcome to redistribute it under certain conditions. See COPYING file for details.

It can be installed in Linux, Windows or MAC iOS environments. Windows is supported only if you use cygwin tools (make and gcc) for compilation.

To install python module run:
```console
pip install asterix_decoder
```

To compile python module from source run:
```console
python setup.py install
```

Then include it in python as:
```console
import asterix
```

To install executable (c/c++) version run:
```console
cd asterix/src
make install 
```

or if you have CMake installed, run:
```console
cmake .
make 
```

Then run it from asterix/install folder:
```console
./asterix
```

If you are using Windows install it through Cygwin terminal.
```console
Install following cygwin packages:
	binutils
	cygwin-devel
	gcc-core
	gcc-g++
	make
	libexpat-devel
```

For more information about the tool or Asterix protocol feel free to contact the author:
<html>
<div class="LI-profile-badge"  data-version="v1" data-size="medium" data-locale="en_US" data-type="horizontal" data-theme="light" data-vanity="damirsalantic"><a class="LI-simple-link" href='https://hr.linkedin.com/in/damirsalantic?trk=profile-badge'>Damir Salantic</a></div>
</html>
