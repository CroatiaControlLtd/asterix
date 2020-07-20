# Asterix specification converter

This directory contains a script to convert asterix definitions from .json to .xml format.

Original asterix definition files are currently stored as .ast files
(custom syntax) in this project:

https://github.com/zoranbosnjak/asterix-specs

A converter exists (as part of asterix-specs project), to convert between
.ast and .json syntaxes in both directions.
Except for some details (comments not being present in json), the .ast and .json
files are equivalent in the information they contain.
However, the JSON syntax is more convenient to be processed in python.

Pre-conveted JSON files are currently available online:
https://zoranbosnjak.github.io/asterix-specs/

JSON files are in turn converted to target format (XML in this case),
using the scripts in this directory.

Remarks:

- `json-to-xml.py` script requires python36 or higher.

# Running the script

To run the script inside nix environment:

```bash
nix-shell -p python38
python3 --version
python3 json-to-xml.py < input.json
# or
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/cats/cat1.18/definition.json | python3 json-to-xml.py
```

To combine CAT+REF definition:

```bash
python3 json-to-xml.py --cat category.json --ref ref.json --outfile out.xml
```

