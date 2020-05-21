This directory contains example scripts to convert from .json to other text based formats.

Original asterix definition files are currently stored as .ast files
(custom syntax) in this project: https://github.com/zoranbosnjak/asterix-specs
A converter exists (as part of asterix-specs project), to convert between
.ast and .json syntaxes (both directions).
Except for some details (comments not being present in json), the .ast and .json
files are equivalent in the information they contain.
However, the JSON syntax is more convenient to be processed in python.

Pre-conveted JSON files are currently available online:
https://zoranbosnjak.github.io/asterix-specs/

JSON files are in turn converted to target format (XML in this case),
using scripts in this directory.

Remarks:

- Scripts require python36 or higher.
- json-to-xml is currently targeting some other dialect of xml (not related to this project).
- json-to-rst.py demonstrates the way to recalculate position of *FX* bits

To run the script inside nix environment:

```bash
nix-shell -p python38
python3 --version
python3 json-to-xml.py {input.json}
# or
curl https://zoranbosnjak.github.io/asterix-specs/specs/cat062/v1.18/cat062-v1.18.json | python3 json-to-xml.py
```

