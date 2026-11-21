---
title: The hex JSON output lets a value be edited and written back to ASTERIX by position; -j and -jx print the FSPEC apart from the item bytes
author: Diego
date: 2026-11-06
branch: jsonx_roundtrip
cost: { minutes: 5, tokens: 31305 }
id: l-5bf7ab58
scope: repo
status: active
replaces: l-8de272a3
src: agent
touches:
  - README
  - README.md
  - install/sample_output/cat_062_065_jsonx.txt
  - src/asterix/DataItem.cpp
  - src/asterix/DataItem.h
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemBits.h
  - src/asterix/DataItemFormatFixed.cpp
  - src/asterix/DataItemFormatFixed.h
  - src/asterix/DataItemFormatVariable.cpp
---

## Decided
-jx exists so a consumer can change a value and regenerate the ASTERIX record at minimal cost. Each field is {"val","hex","mask"?,"pos"}. A value is written back with hexdata[pos..] = (hex & ~mask) | (value & mask), and the record is fspec + hexdata.
- pos is a byte offset in the record's "hexdata", which holds only the data items after the FSPEC: its first byte is pos 0.
- "fspec" is printed from DataRecord::m_pFSPECData/m_nFSPECLength (computed once at parse). The FSPEC rule is not recomputed when printing.
- -j uses the same header (fspec apart, hexdata without the FSPEC), by the user's choice. This breaks compatibility with the original, where the hexdata of -j was the whole record. -jh, -je, XML and TXT keep the whole record in hexdata.
- mask is present whenever the field does not fill its hex bytes. Keys are unique (FX0, FX1... per extent; spare, spare1...).

## Tried and ruled out
- Rebuilding records from hex + mask without pos: Compound primary subfields are not in the JSON, and Repetitive lists look like single-part Variable lists.
- The -je mask rule (only when the bit length is not a multiple of 8): it loses BDS res1.

## Watch out
- Edits must keep the record length. FX bits, UAP selectors (CAT001 I020 TYP) and the BDS register number change how the rest decodes.
- pos = DataItem::m_nOffset (set in the DataRecord constructor) + pointer offset inside the item's copy (DataItemBits::JsonPosition).

In the code: src/asterix/DataRecord.cpp (EJSON/EJSONX header, m_nOffset), src/asterix/DataItem.cpp, src/asterix/DataItemBits.cpp (appendJsonHexMask), src/asterix/DataItemFormatFixed.cpp (jsonUniqueKey)
