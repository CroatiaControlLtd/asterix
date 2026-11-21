---
title: "With -LF, items with no field in the filter are only measured, never copied or printed; stdin is read in blocks"
author: Diego
date: 2026-11-21
branch: master
cost: { minutes: 34, tokens: 225987 }
id: l-eb07e93f
scope: repo
status: active
src: agent
touches:
  - src/engine/stddevice.cxx
  - src/asterix/DataItem.cpp
  - src/asterix/DataRecord.cpp
  - src/asterix/DataItemFormatFixed.cpp
  - src/asterix/DataItemBits.cpp
---

## Decided
- With -LF, an item whose fields are all outside the filter is stepped over while the record is parsed: only its length is read (no DataItem, no copy of its bytes) and getText never visits it. Profiling `-G -j -LF` on a real 2.7 GB GPS capture showed most of the time went to decoding, copying, walking and freeing items that print nothing (unfiltered Compound items alone were 10%).
- The per-record JSON header (-j/-jx) is written with append helpers instead of format(): the two printf calls per record were 8% of the run.
- stdin is read in 64 KB blocks. Before, every packet cost 3 read() + 1 select() (GPS input via `bzip2 -dc | asterix` was ~60 s of sys time on the full file). A read() on a pipe returns what is there, so live input is not delayed. stdout is still one write() per packet on purpose: buffering it would delay live output.
- Result on the full capture: 5m05s -> 2m59s, output byte for byte identical apart from the fix below.

## Watch out
- A Repetitive item with 0 repetitions returns true from getText even when unfiltered. Before this change -LF printed such items anyway (CAT062 I390 "TOD":[] in tests/golden/pcap_062/*.filtered.txt); now they are left out, as -LF intends. Those 7 golden files were regenerated for that reason only.
- With -LF, DataRecord::m_lDataItems holds only the items that have a filtered field, so getItem() cannot find the others. Python and Wireshark never filter (gFiltering=false), so they are not affected.
- stderr messages from inside getText of unfiltered items (e.g. "Wrong length in Repetitive") no longer appear with -LF. Length errors found while parsing still appear, with the same text (DataItem::getLength).

## In the code
- DataItemDescription::m_bFiltered, set in Category::filterOutItem
- DataRecord::DataRecord (skip) and DataItem::getLength (shared length + errors)
- DataItem::getText / DataItemFormatFixed::getText (early filter checks)
- Utils.h appendDoubleFixed/appendHex32/appendInt03/appendIPv4, DataRecord::getText EJSON header
- src/engine/stddevice.cxx read buffer
- Measure with a decompressed sample and -f, plus the same through a pipe: stdin and -f had very different costs
