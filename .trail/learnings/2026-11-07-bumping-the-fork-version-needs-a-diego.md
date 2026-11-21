---
title: "Bumping the fork version needs a forced rebuild of the main object, or -V keeps showing the old one"
author: Diego
date: 2026-11-07
branch: jsonx_roundtrip
cost: { minutes: 2, tokens: 46462 }
id: l-b9b7da47
scope: repo
status: active
src: agent
touches:
  - README
  - README.md
  - install/sample_output/cat_001_002_json.txt
  - install/sample_output/cat_062_065_json.txt
  - src/asterix/DataRecord.cpp
  - src/main/version.h
---

## Decided
Fork versions are <upstream>-ifsnop.<n>. A bump touches src/main/version.h, the version line in README and README.md, and a new HISTORY entry (marked "python_v0.7.11 unchanged", since the Python module is not maintained in this fork). Bump n when a change is incompatible with the previous fork release: 2.8.11-ifsnop.2 came from the -j header change (fspec split from hexdata).

## Watch out
The make build does not track header dependencies (the .depend generation in src/Makefile is commented out). After editing version.h, `make` reports success but `asterix -V` still prints the old version. Delete the main object first.

In the code:
- find obj -name asterix.o -path '*main*' -delete; cd src && make
- check with obj/main/release/asterix -V
- grep -rn "ifsnop\.<old n>" outside obj/ and .git/ to find every place that names the version
