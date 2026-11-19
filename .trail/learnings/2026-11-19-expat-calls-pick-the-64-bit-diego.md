---
title: "Expat calls pick the 64-bit API on expat 2.9 or newer and keep the old call on older expat, so the build has no warnings on both"
author: Diego
date: 2026-11-19
branch: master
cost: { minutes: 2, tokens: 90191 }
id: l-02c57033
scope: repo
status: active
src: agent
touches:
  - src/asterix/XMLParser.cpp
  - setup.py
---

## Decided
Expat is a hard dependency (make, CMake and setup.py all link -lexpat; it parses the category XML definitions), so the code has to build cleanly on old and new expat. Expat 2.9.0 (Oct 2026) added XML_GetCurrentLineNumber64 (and Column/ByteIndex/ByteCount/InputContext 64 variants) and marked the old ones deprecated. We pick the call with a compile-time version check instead of requiring a minimum expat version.

## Tried and ruled out
Switching unconditionally to the *64 functions: they don't exist before 2.9.0, and Debian 13 stable ships 2.8.3 (dev machine has 2.6.1), so that breaks the build there.

## Watch out
The deprecation warning only appears with expat >= 2.9 (sid/backports/manual install), never with Debian 13 stable's package. If a warning is reported "on Debian 13", that machine has a newer expat. Any new expat location call needs the same guard.

In the code:
- src/asterix/XMLParser.cpp XMLParser::Error: `#if XML_MAJOR_VERSION > 2 || (XML_MAJOR_VERSION == 2 && XML_MINOR_VERSION >= 9)` -> XML_GetCurrentLineNumber64, else XML_GetCurrentLineNumber; printed as unsigned long long with %llu.
- Check the new path without installing expat: g++ -fsyntax-only against upstream master expat.h + expat_external.h.
