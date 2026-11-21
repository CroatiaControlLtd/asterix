---
title: The hex JSON output lets a value be edited and written back to ASTERIX by position; text and every JSON output print the FSPEC apart from the item bytes
author: Diego
date: 2026-11-07
branch: fspec_all_outputs
cost: { minutes: 26, tokens: 58344 }
id: l-565293c9
scope: repo
status: active
replaces: l-5bf7ab58
src: agent
touches:
  - ".github/workflows/python-publish.yml"
  - src/asterix/DataBlock.cpp
  - src/asterix/DataItem.cpp
  - src/asterix/DataItemFormatExplicit.cpp
  - src/asterix/DataRecord.cpp
  - src/asterix/Tracer.cpp
  - src/asterix/XMLParser.cpp
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemFormatFixed.cpp
---

## Decided
-jx exists so a consumer can change a value and regenerate the ASTERIX record at minimal cost. Each field is {"val","hex","mask"?,"pos"}. A value is written back with hexdata[pos..] = (hex & ~mask) | (value & mask), and the record is fspec + hexdata.
- pos is a byte offset in the record's "hexdata", which holds only the data items after the FSPEC: its first byte is pos 0.
- "fspec" is printed from DataRecord::m_pFSPECData/m_nFSPECLength (computed once at parse). The FSPEC rule is not recomputed when printing.
- The same record header (FSPEC apart, hexdata without it) is used by -j, -jx, -jh, -je and the default text output ("Fspec:" and "HexData:" lines), by the user's choice; it breaks compatibility with the original, where hexdata was the whole record. Only XML (-x/-xh) keeps the whole record in hexdata. -l has no record hex.
- mask is present whenever the field does not fill its hex bytes. Keys are unique (FX0, FX1... per extent; spare, spare1...).

## Tried and ruled out
- Rebuilding records from hex + mask without pos: Compound primary subfields are not in the JSON, and Repetitive lists look like single-part Variable lists.
- The -je mask rule (only when the bit length is not a multiple of 8): it loses BDS res1.

## Watch out
- Edits must keep the record length. FX bits, UAP selectors (CAT001 I020 TYP) and the BDS register number change how the rest decodes.
- pos = DataItem::m_nOffset (set in the DataRecord constructor) + pointer offset inside the item's copy (DataItemBits::JsonPosition).
- The data block header line "HexData:" in the text output (DataBlock) is the block's CAT+LEN bytes, not a record: it has no Fspec.
- A header change is checked by rebuilding the old output from the new one (old hexdata == new fspec + new hexdata, every other line equal), not by eye.

In the code: src/asterix/DataRecord.cpp (DataRecord::getText: strFSPEC/pItemsHex computed once for text and JSON formats, m_nOffset), src/asterix/DataItem.cpp, src/asterix/DataItemBits.cpp (appendJsonHexMask), src/asterix/DataItemFormatFixed.cpp (jsonUniqueKey)
