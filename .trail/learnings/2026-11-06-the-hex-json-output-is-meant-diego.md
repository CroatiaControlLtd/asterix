---
title: "The hex JSON output is meant for editing a value and writing it back to ASTERIX, so its keys are unique and every partial field has a mask"
author: Diego
date: 2026-11-06
branch: jsonx_roundtrip
cost: { minutes: 8, tokens: 59525 }
id: l-ff1c7d28
scope: repo
status: active
replaces: l-ce031d62
src: agent
touches:
  - HISTORY
  - README
  - README.md
  - src/main/asterix.cpp
  - src/main/version.h
  - src/engine/diskdevice.cxx
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemFormatFixed.cpp
  - src/asterix/DataItemFormatVariable.cpp
---

## Decided
-jx / --json-hex exists so a consumer can change a value and regenerate the ASTERIX bytes at minimal cost. It follows the -j structure (one record per line, no trailing comma) and each field is {"val":..,"hex":".."[,"mask":".."]}, so a field is rewritten with new = (old & ~mask) | (value & mask).
- "mask" is emitted whenever the field does not fill its hex bytes completely. It was first the -je rule (mask only when the bit length is not a multiple of 8), which dropped the mask of 8*n-bit fields that start mid-byte (BDS res1, CAT252 spare): the byte they share with the next field could not be told apart.
- Keys are unique within every object: FX bits are numbered by extent (FX0, FX1, ...), and other repeated names get the first free number (spare, spare1). -j still repeats FX/spare (110 of 281 sample records), and standard JSON parsers keep only the last one.
- -j, -jh and -je are unchanged. EJSONX is the last value of the format enum.

## Tried and ruled out
- Reusing or fixing -je: it is multi-line, writes "val"= (not valid JSON), and adds name/meaning.
- Renaming keys in -j too: it would break existing consumers.

## Watch out
- A field shares its first hex byte with the previous field only if the previous field's mask ends in 0 AND its own mask starts with 0. A field can start mid-byte with no previous field, because some bits are not defined as fields (CAT048 I161).
- Rebuilding a whole record from -jx alone still needs: the record FSPEC (the start of "hexdata"), the primary subfield of Compound items (not in the JSON), and a way to tell Repetitive lists (count byte) from single-part Variable lists (no count byte).
- -j and -jx do not JSON-escape ASCII values.

In the code: src/asterix/DataItemBits.cpp (appendJsonHexMask, getText with pJsonKey), src/asterix/DataItemFormatFixed.cpp (jsonUniqueKey), src/asterix/DataItemFormatVariable.cpp (keys shared across parts, extent index)
