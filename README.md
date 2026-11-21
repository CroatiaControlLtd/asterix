asterix
=======

[![build](https://img.shields.io/github/actions/workflow/status/ifsnop/asterix/tests.yml?branch=master&label=build)](https://github.com/ifsnop/asterix/actions/workflows/tests.yml)
[![coverage](https://img.shields.io/endpoint?url=https%3A%2F%2Fraw.githubusercontent.com%2Fifsnop%2Fasterix%2Fbadges%2Fcoverage.json)](https://github.com/ifsnop/asterix/actions/workflows/tests.yml)
[![Build Status](https://travis-ci.org/CroatiaControlLtd/asterix.svg?branch=master)](https://travis-ci.org/CroatiaControlLtd/asterix)
[![Documentation badge](https://readthedocs.org/projects/asterix/badge/?version=latest)](http://asterix.readthedocs.io/en/latest/)
[![Upload Python Package](https://github.com/CroatiaControlLtd/asterix/actions/workflows/python-publish.yml/badge.svg)](https://github.com/CroatiaControlLtd/asterix/actions/workflows/python-publish.yml)
[![PyPI](https://img.shields.io/pypi/v/asterix_decoder.svg)](https://pypi.python.org/pypi/asterix_decoder)

> [!IMPORTANT]
> **This is a fork of [CroatiaControlLtd/asterix](https://github.com/CroatiaControlLtd/asterix).**
> The original project has not been updated since December 2025 (last commit for 2.8.11 on 2025-12-03, last release v2.8.10). Its decoder had memory-safety problems that needed to be fixed, and they are fixed here. ASTERIX data usually comes from the network, so malformed or truncated packets made the original code read past its buffers, which can crash it.
>
> Fork versions are numbered `<upstream version>-ifsnop.<n>`. The current one is **2.8.11-ifsnop.6**, based on upstream 2.8.11 (`asterix -V` shows both).
>
> Changes in this fork (see [HISTORY](HISTORY)):
> - **Security fixes**: out-of-bounds reads on truncated or malformed records (item lengths, PCAP and ORADIS headers), a stack overflow with `-LF` filters on BDS items, memory leaks with ORADIS input, invalid pointer casts, and string overflows in file naming. Found with AddressSanitizer/UBSan and fuzzing.
> - **Source IP** of each record in the TXT, JSON and XML outputs (`srcip`), taken from UDP multicast input and GPS files.
> - **New `-jx` / `--json-hex` output**: compact one-line-per-record JSON made to change a value and write it back to ASTERIX at minimal cost:
>   - each record has `fspec` (the FSPEC bytes: while a byte ends in 1 the next one belongs to it) and `hexdata` (the data items after it), so the record is `fspec + hexdata`;
>   - each field is `{"val":..., "hex":"...", "mask":"...", "pos":N}`: `pos` is the byte offset of `hex` in `hexdata` and `mask` (present when the field does not fill its hex bytes) the field's bits, so a field is rewritten with `hexdata[pos..] = (hex & ~mask) | (value & mask)`, without knowing the item structure;
>   - keys are unique in every object, so standard JSON parsers keep all fields: FX bits are numbered by extent (`FX0`, `FX1`, ...) and other repeated names get a number (`spare`, `spare1`).
>
>   Edits must keep the record length. FX bits, UAP selectors (e.g. CAT001 I020 TYP) and the BDS register number change how the rest is decoded.
> - **Record header with the FSPEC apart**: `-j`, `-jh`, `-je` and the default text output print the FSPEC bytes separately (`"fspec"` in JSON, a `Fspec:` line in text), and `hexdata` / `HexData:` holds only the data items after the FSPEC (same as `-jx`). **This differs from the original**, where hexdata was the whole record: use `fspec + hexdata` to get it. XML (`-x`, `-xh`) keeps the whole record.
> - **`-h`** documents the default text output (used when no output format option is given).
> - **Text fields** (callsign, aircraft type, airports, stands...) only print `A-Z`, `0-9`, space and `.`; any other byte, including control characters used as padding, is printed as `@`, so JSON and XML outputs are always valid.
> - **Malformed records**: decoding goes on after them as before, and a `WARNING` on stderr gives the whole data block in hex for later analysis.
> - **Faster decoding, same output**: on a 51 MB GPS file, `-jx` takes 8.5 s instead of 68 s (8 times faster), `-j` 7.2 s instead of 40 s (5.5 times), and `-jh`, `-x`, `-xh` and the other formats are 2 to 5 times faster. The output is byte for byte the same as before. With `-LF`, items with no field in the filter are skipped instead of decoded, and stdin is read in blocks: piping a 2.7 GB GPS capture through `bzip2 -dc | asterix -LF ... -G -j` takes 2m59s instead of 5m05s.
> - **Build**: `make` now rebuilds the objects that include a header when it changes (the original makefiles never did). Run `make clean` once after updating. CMake needs version 3.10 or later.
> - **Test suite run on every push and pull request** (GitHub Actions, see [tests/README.md](tests/README.md)): golden outputs of every input format and output format, consistency and write-back checks of `-jx`, stdin/multicast input, malformed and mutated inputs under AddressSanitizer/UBSan, unit tests, libFuzzer and a line coverage floor (about 65%, from 48%).
> - **Fixes found by the test suite**: HDLC files under 4 KB decoded nothing and longer ones lost their end; an HDLC block length of 0 looped forever; corrupt PCAP packet lengths and FINAL byte counts allocated up to 4 GB and could abort; a definitions file that failed to parse leaked memory.
>
> The **build** and **coverage** badges above are this fork's (GitHub Actions, `tests.yml`). The other badges and the `asterix_decoder` package on PyPI belong to the original project; the PyPI package does not include these changes.

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

To run the tests (see [tests/README.md](tests/README.md) for every layer and what to update when an output changes on purpose):
```console
make -C src && make -C src debug
(cd install/test && bash test.sh)
tests/run_tests.sh
cmake -S . -B build -DASTERIX_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure
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

For asterix specification handling, see [asterix-specs-converter](./asterix-specs-converter/README.md#asterix-specifications) directory.

For more information about the tool or Asterix protocol feel free to contact the author:
<html>
<div class="LI-profile-badge"  data-version="v1" data-size="medium" data-locale="en_US" data-type="horizontal" data-theme="light" data-vanity="damirsalantic"><a class="LI-simple-link" href='https://hr.linkedin.com/in/damirsalantic?trk=profile-badge'>Damir Salantic</a></div>
</html>
