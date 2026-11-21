---
title: Lengths read from input are bounded before allocating or subtracting; robustness runs cap memory so a huge allocation fails on every machine
author: Diego
date: 2026-11-07
branch: test_suite
cost: { minutes: 40, tokens: 305570 }
id: l-39fa7d9e
scope: repo
status: active
src: agent
touches:
  - ".github/workflows/tests.yml"
  - install/test/test.sh
  - tests/README.md
  - CMakeLists.txt
  - tests/coverage_min.txt
  - tests/fuzz/fuzz_parse.cpp
  - tests/tools/coverage_report.py
  - tests/tools/gcov_flush.c
  - tests/unit/minitest.h
  - tests/unit/test_bits.cpp
---

## Decided
Any length taken from the input (PCAP incl_len, FINAL byte count, HDLC block length, ASTERIX block/item lengths) is checked against what the format allows and what is left to read before it is used to allocate, subtract or advance. The robustness section of tests/run_tests.sh runs with MEMLIMIT_KB in CI, so an unbounded allocation aborts deterministically instead of depending on free memory.

## Tried and ruled out
Trusting the sanitizer runs alone: ASan did not report the 4 GB PCAP/FINAL allocations (they succeeded through overcommit), and the release binary passed; only a -O0 build on a busier moment, then the memory cap, showed the std::bad_alloc aborts. An unsigned "len - header" followed by "if (len <= 0)" never fires: check before subtracting.

## Watch out
- Fixing a reader can expose the next bug: once HDLC files under 4 KB were read, a block length 0 made proc_hdlc_frame loop forever.
- After a bad record, DataBlock continues from where that record stopped, so leftovers may decode as bogus records (ASTERIX records carry no length). Left as is, pending a decision.
- Every fixed input bug gets its input in tests/data/bad through tests/tools/gen_samples.py.

In the code: src/asterix/asterixpcapsubformat.cxx (incl_len), asterixfinalsubformat.cxx (byte count), asterixhdlcsubformat.cxx and asterixhdlcparsing.c (read size, block length), src/asterix/DataBlock.cpp (resync); tests/run_tests.sh (MEMLIMIT_KB, MUTANTS), tests/tools/mutate.py
