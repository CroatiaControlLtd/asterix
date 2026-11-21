---
title: "Strings returned by the bit-field text getters are always allocated with new[], error results included"
author: Diego
date: 2026-11-06
branch: add_ip_to_gps
cost: { minutes: 2, tokens: 20630 }
id: l-8064f477
scope: repo
status: active
src: agent
touches:
  - src/asterix/DataItemFormatCompound.cpp
  - src/asterix/DataItemBits.cpp
---

## Decided
The bit-field getters that return text (six-bit, hex, full-byte hex, mask, octal, ASCII) allocate with new[] on every path. Their "???" error result now uses new[] too, through a small template, so callers can release any result with delete[] or hold it in std::unique_ptr<T[]>. New code should use unique_ptr: SonarCloud flags explicit delete ("Rewrite the code so that you no longer need this delete").

## Tried and ruled out
Wrapping the result in unique_ptr<T[]> alone: the error paths returned strdup("???"), which is malloc memory, so the unique_ptr would have run delete[] on it. That is undefined behaviour, and the explicit delete[] callers (-j, -je...) already had it. The error paths had to change first.

## Watch out
- Never free() these results, and never use plain delete. The Wireshark-only getData code still does "delete pStr" on the ASCII result (it should be delete[]). That is pre-existing and not compiled into the CLI.
- The -je output code still has explicit delete[]. It is correct now, but Sonar would flag it if those lines are touched.

In the code: src/asterix/DataItemBits.cpp (newErrorString<T>, appendJsonHexMask, getASCII and the other get*String getters; Wireshark getData ASCII case)
