---
title: Decoder speed-ups keep the output byte-identical; stdout is still written once per packet on purpose
author: Diego
date: 2026-11-07
branch: master
cost: { minutes: 29, tokens: 227327 }
id: l-203555e9
scope: repo
status: active
src: agent
touches:
  - src/engine/diskdevice.cxx
---

## Decided
The -jx decode of a 51 MB GPS file went from 68 s to 8.4 s with the same output (md5 of the 2.7 GB result). Where the time went, in order: format() ran vsnprintf twice plus a heap copy for every fragment (~45%); per-field getBits/new[]/sprintf for hex and mask; the EJSONX unique-key std::set rebuilt for every field of every record; temporary strings copied at each level (DataItem, DataRecord, Variable, Compound); ftell+fstat syscalls in the disk device's BytesLeftToRead several times per packet.
Fixes: single-pass format() with a stack buffer; direct appends (hex table, integer writer, std::to_chars for %.7lf when C++17 is available, snprintf fallback under C++11); EJSONX key prefixes computed once per format and extent (they depend only on the format); writing straight into the caller's string and cutting it back when an item prints nothing; FRN->description and presence-bit caches; read position tracked in the disk device and file size cached (re-read with fstat when < 128 KB seem to be left, so growing files still work).

## Tried and ruled out
- Buffering stdout across packets: it would save ~1 write() per packet, but live inputs (multicast/TCP to stdout) need each packet written at once. Not done.
- Writing %.7lf by hand: rounding would not match printf in edge cases; std::to_chars(fixed, 7) is specified to match printf exactly.

## Watch out
- Verify with md5 of stdout only, or mask the wall-clock time on stderr ERROR lines (and the timestamp of raw inputs): stderr differs between runs of the same binary.
- What is left (~8 s) is mostly building the text in std::string; a raw output buffer through the whole getText chain would be the next step, and a large refactor.

In the code: src/asterix/Utils.{h,cpp} (format, appendHexBytes, writeLong, appendDouble7), DataItemBits::appendJsonHexMask, DataItemFormatFixed::makeJsonPrefixes, DataItemFormatVariable::getJsonPrefixes, UAP::getDataItemDescriptionByUAPfrn, CDiskDevice::BytesLeftToRead and Select.
