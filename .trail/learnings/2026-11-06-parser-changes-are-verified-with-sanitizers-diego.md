---
title: "Parser changes are verified with sanitizers plus mutated inputs, because the sample files are all valid data"
author: Diego
date: 2026-11-06
branch: fix_overflows
cost: { minutes: 19, tokens: 144913 }
id: l-ad2be6b9
scope: repo
status: active
src: agent
touches:
  - src/asterix/DataItem.cpp
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemBits.h
  - src/asterix/DataItemFormat.h
  - src/asterix/DataItemFormatBDS.cpp
  - src/asterix/DataItemFormatBDS.h
  - src/asterix/DataItemFormatCompound.cpp
  - src/asterix/DataItemFormatCompound.h
  - src/asterix/DataItemFormatExplicit.cpp
  - src/asterix/DataItemFormatExplicit.h
---

## Decided
Bounds and overflow fixes in the decoder are checked in three ways:
1. Byte-for-byte comparison of the outputs for every sample in every format, before and after the change (valid data must not change).
2. A build with -fsanitize=address,undefined run over the same samples.
3. Mutated copies of the samples (byte flips, truncation, extreme 16-bit length values) run under the sanitizer build with a timeout.
Only step 3 exercises the error paths: on master, 522 of 960 mutants hit sanitizer errors that the valid samples never showed.
getLength(pData, nMaxLength) contract: it never reads past nMaxLength, and it returns a value greater than nMaxLength when the item does not fit, so the existing "needed > available" checks reject the item.

## Tried and ruled out
- Trusting test.sh and valgrind: they only feed valid samples and passed while master was reading past buffers.
- gcc -fanalyzer on this C++: about 70 warnings, almost all false positives ("operator new may return NULL", "uninitialized <unknown>" inside the STL). It found the getASCII bug but not the overflows.

## Watch out
- Raw/ORADIS inputs (-f on .raw/.ast) stamp records with wall-clock time, so mask the timestamp before comparing outputs.
- Check that the baseline outputs are not empty before trusting a "differs" or "identical" result.
- A "LeakSanitizer" report whose stack goes through __cxa_demangle / ubsan_diag is UBSan's own allocation while printing a downcast report, not a program leak.
- An item whose sub-format is BDS goes through Repetitive; never assume DataItemFormatFixed there.

In the code: src/asterix/DataItemFormat.h (getLength contract), DataItem::parse, DataRecord constructor (length checks), install/sample_data (inputs used); no -Werror or sanitizer build exists in the makefiles, it was built by hand with g++ over the sources listed in src/*/Makefile
