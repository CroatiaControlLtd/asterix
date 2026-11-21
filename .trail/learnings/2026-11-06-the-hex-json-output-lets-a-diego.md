---
title: "The hex JSON output lets a value be edited and written back to ASTERIX by position, without knowing the item structure"
author: Diego
date: 2026-11-06
branch: jsonx_roundtrip
cost: { minutes: 6, tokens: 42767 }
id: l-8de272a3
scope: repo
status: active
replaces: l-ff1c7d28
src: agent
touches:
  - src/asterix/DataRecord.cpp
  - src/asterix/DataItem.cpp
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemFormatFixed.cpp
  - src/asterix/DataItemFormatVariable.cpp
---

## Decided
-jx exists so a consumer can change a value and regenerate the ASTERIX record at minimal cost. Each record has "fspec" (bytes while the last bit is 1) and "hexdata" (the items after it), so the record is fspec + hexdata. Each field is {"val","hex","mask"?,"pos"}: pos is the byte offset of hex in hexdata, and mask is present whenever the field does not fill its hex bytes. Writing a value back is hexdata[pos..] = (hex & ~mask) | (value & mask). Keys are unique in every object (FX0, FX1... per extent; spare, spare1...), because -j repeats keys and standard parsers drop them. -j, -jh and -je are unchanged.

## Tried and ruled out
- Rebuilding records from hex + mask alone: Compound primary subfields are not in the JSON, and Repetitive lists (count byte) look like single-part Variable lists (no count byte). pos makes the structure irrelevant.
- The -je mask rule (mask only when the bit length is not a multiple of 8): it loses 8*n-bit fields that start mid-byte (BDS res1).

## Watch out
- Edits must keep the record length. FX bits, UAP selectors (CAT001 I020 TYP) and the BDS register number change how the rest decodes. A round trip over all sample fields gave 10268 exact edits and 140 structural ones, all of these kinds.
- pos comes from DataItem::m_nOffset, set in the DataRecord constructor, plus pointer arithmetic inside the item's copy (DataItemBits::JsonPosition). Every format must keep passing pointers into that same buffer.
- -j and -jx do not JSON-escape ASCII values.

In the code: src/asterix/DataRecord.cpp (fspec split, m_nOffset), src/asterix/DataItem.cpp (JsonPosition), src/asterix/DataItemBits.cpp (appendJsonHexMask, getText with pJsonKey), src/asterix/DataItemFormatFixed.cpp (jsonUniqueKey), src/asterix/DataItemFormatVariable.cpp
