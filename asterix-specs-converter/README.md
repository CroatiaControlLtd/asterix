# Asterix specifications

[eurocontrol]: http://www.eurocontrol.int/services/asterix
[converter]: https://zoranbosnjak.github.io/asterix-specs/bin/converter-static
[asterix-specs]: https://github.com/zoranbosnjak/asterix-specs
[asterix-specs-out]: https://zoranbosnjak.github.io/asterix-specs/
[syntax]: https://zoranbosnjak.github.io/asterix-specs/syntax/index.html

This project uses `.xml` files for asterix definitions. The `.xml` files
can be created and maintained by editing the files directly, however a
prefered approach is to *convert* existing definition files instead.

Basic conversion flow:

`(unstructured).pdf` &rarr; `.ast` &harr; `.json` &rarr; `.xml`

This directory contains a script to convert asterix definitions
from `.json` to `.xml` format.

## Asterix specification formats

* Original asterix definitions are defined as unstructured (free text)
`.pdf` files by [eurocontrol][eurocontrol].

* A [structured version][asterix-specs-out] of the specifications is available
in `.ast` and `.json` formats. Custom `.ast` format uses a [low overhead
syntax][syntax] and is used to maintain the specifications.
A [converter][converter] exists, as part of
[asterix-specs project][asterix-specs], to convert between
`.ast` and `.json` syntaxes in both directions transparently.

* `.json` files are in turn converted to the target `.xml` format,
using a script from this directory.

In case of problems in asterix definitions, users of this project are
encouraged to report problems upstream.

## Running `asterix2json.py` script

> **_NOTE:_** `asterix2json.py` script requires `python36` or higher.

### Basic conversion example:

```bash
# check python version
python3 --version

# download definition in .json format
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/cats/cat1.18/definition.json > input.json

# convert json to xml
python3 asterix2json.py < input.json > output.xml

# or directly
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/cats/cat1.18/definition.json | python3 asterix2json.py
```

### Combine CAT+REF definition to a single xml:

```bash
python3 asterix2json.py --cat category.json --ref ref.json --outfile out.xml
```
## Running `update-specs.py` script
This scripts autimatically reads latest Asterix specifications from 
[asterix-specs-out] and creates all XML definitions in specs folder.


## Conversion workflow when .json file is available

- download `.json` file;
- convert `.json` to `.xml`;
- review generated `.xml` file;
- compare new `.xml` file with an old one (if exists);
- commit new/changed `.xml` file to this repository;

> **_NOTE:_** If the resulting `.xml` file in not as expected, check/modify
the `asterix2json.py` script.

> **_NOTE:_** Do not manually edit converted `.xml` files if possible.

## Updating asterix specifications

Asterix specifications shall be extended/modified in the upstream
[asterix-specs project][asterix-specs].

Procedure:

- clone [asterix-specs project][asterix-specs];
- add/modify `.ast` file(s);
- validate file(s) using a [converter][converter];
- the same converter can be used to convert to `.json` for local testing;
- pretify modified `.ast` files, using a converter.
- review changes by peer reviewer, make sure that changes are correct and complete;
- create pull request to make the changes available to others;

