---
title: "The hex JSON output is the compact JSON plus hex and mask per field, not a one-line version of the extensive JSON"
author: Diego
date: 2026-11-06
branch: add_ip_to_gps
cost: { minutes: 9, tokens: 73988 }
id: l-ce031d62
scope: repo
status: active
src: agent
touches:
  - install/sample_output/cat_001_002.txt
  - install/sample_output/cat_001_002.xml
  - install/sample_output/cat_001_002_json.txt
  - install/sample_output/cat_001_002_jsonh.txt
  - install/sample_output/cat_034_048.txt
  - install/sample_output/cat_034_048_filtered.txt
  - install/sample_output/cat_062_065_json.txt
  - install/sample_output/parsegps.txt
  - src/asterix/DataBlock.cpp
  - src/engine/basedevice.hxx
---

## Decided
A new output, -jx / --json-hex, was added for consumers that parse one ASTERIX record per line. It follows the -j structure exactly: one line per record, ending in "}}" with no trailing comma. Each field becomes {"val":..,"hex":".."}, and "mask" is added only when the field's bit length is not a multiple of 8, the same rule -je uses. Stripping hex/mask from a -jx line gives exactly the -j line, and that is the invariant to keep.
It was added at the end of the format enum (after GPS) so existing format numbers don't move.

## Tried and ruled out
- Reusing or fixing -je: it spans several lines, ends each record with "}},", writes "val"= (not valid JSON), adds name/meaning, and can print " ( ?????? )" outside quotes. It was left untouched so its current users don't break.
- Always emitting "mask" (a uniform schema): the user chose the -je rule to keep lines shorter.
- Other option names (-jp, -jc): the user chose -jx.

## Watch out
-j and -jx do not JSON-escape ASCII values. A field containing a quote or backslash would produce an invalid line.
Any new JSON code path must list EJSONX next to EJSON (compact branch), except in DataItemBits, where EJSONX has its own value form.

In the code: src/asterix/DataItemBits.cpp (appendJsonHexMask, EJSONX cases), src/asterix/asterixformat.hxx (EJSONX after EGPS), src/main/asterix.cpp (-jx), install/sample_output/cat_062_065_jsonx.txt
