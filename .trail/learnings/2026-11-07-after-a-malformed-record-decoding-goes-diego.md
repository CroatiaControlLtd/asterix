---
title: "After a malformed record decoding goes on and a warning prints the whole block; text fields keep only A-Z, 0-9, space and dot"
author: Diego
date: 2026-11-07
branch: ascii_fields_and_block_warning
cost: { minutes: 20, tokens: 82045 }
id: l-6e821244
scope: repo
status: active
src: agent
touches:
  - src/asterix/DataBlock.cpp
  - src/asterix/Tracer.cpp
  - src/asterix/asterixformat.cxx
  - tests/data/bad/raw_broken_record_in_block.raw
  - tests/run_tests.sh
  - tests/tools/gen_samples.py
  - tests/unit/test_parser.cpp
  - src/asterix/DataItemBits.cpp
  - tests/golden/multicat/json.txt
  - tests/golden/multicat/jsone.txt
---

## Decided
- After a malformed record the output stays as it was: DataBlock goes on from where that record stopped. Measured on 1950 mutated inputs, 88% of the records printed after a broken one are real records, 12% garbage. A WARNING on stderr gives the whole data block in hex (CAT, LEN, data) so it can be analysed later. The user chose this over stopping the block (loses the good records) or marking records in the output.
- ASCII-encoded fields print only A-Z, 0-9, space and '.'; any other byte becomes '@', so no output format needs escaping (a quote or backslash used to make -j/-jx invalid JSON). Per the specs these fields are upper-case alphanumeric padded with spaces (I062/390 notes, ICAO aircraft identification); '.' is kept for I205/090 radio channel names (121.100). -jx "hex" keeps the raw bytes.

## Tried and ruled out
- Stopping the block at the first bad record: drops about 7 good records for each garbage one.
- Keeping the rest of the block only if it decodes cleanly to its end (prototype): removed 621 doubtful records but 3765 good ones.

## Watch out
- Real captures pad some text fields with control characters (callsign DLHEGN + 0x01, runway 0x00): those now print '@' where they used to print a space.
- CAT240 I030 is free text by spec: its symbols also become '@'.
- Tracer and debug_trace messages have no length limit (blocks can be 64 KB of hex); debug_trace prints messages starting with "Warning: " as WARNING.

In the code: src/asterix/DataItemBits.cpp (getASCII), src/asterix/DataBlock.cpp (warning), src/asterix/Tracer.cpp, src/asterix/asterixformat.cxx (debug_trace); tests: tests/data/bad/raw_broken_record_in_block.raw, run_tests.sh robustness (JSON validity of mutated inputs)
