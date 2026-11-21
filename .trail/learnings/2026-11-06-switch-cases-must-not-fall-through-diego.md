---
title: Switch cases must not fall through into the next label; Sonar rejects comment-only annotations and the code is C++11
author: Diego
date: 2026-11-06
branch: add_ip_to_gps
cost: { minutes: 5, tokens: 12172 }
id: l-1d964299
scope: repo
status: active
src: agent
touches:
  - README
  - install/sample_output/cat_062_065_jsonx.txt
  - install/test/test.sh
  - install/test/updatetests.sh
  - src/asterix/DataItem.cpp
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemBits.h
  - src/asterix/DataItemFormatCompound.cpp
  - src/asterix/DataItemFormatExplicit.cpp
  - src/asterix/DataItemFormatFixed.cpp
---

## Decided
When two output formats share most of a switch branch but one needs extra text, stack all the labels on a single block and add the difference with an if on formatType. Do not let one case fall through into the next.

## Tried and ruled out
- A "/* no break */" comment before the next label: SonarCloud still reports "unannotated fall-through between switch labels". It only showed up when a new label (EJSONX) was added after the comment, because Sonar reports issues on new code only, so older fall-throughs can still be lurking.
- [[fallthrough]]: that is C++17, and the project builds as C++11 (CMAKE_CXX_STANDARD 11).

## Watch out
Adding a new format usually means appending a case next to an existing one. Check whether the case before it ends without a break; if so, restructure as above instead of inserting the label.

In the code: src/asterix/DataItemFormatCompound.cpp (JSON branch of the secondary-part switch in getText), CMakeLists.txt (CMAKE_CXX_STANDARD 11)
