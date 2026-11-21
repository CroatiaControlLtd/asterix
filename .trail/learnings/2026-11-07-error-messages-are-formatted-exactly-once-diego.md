---
title: Error messages are formatted exactly once; text that may hold data or file names never goes in the format argument
author: Diego
date: 2026-11-07
branch: fix_codeql_alerts
cost: { minutes: 13, tokens: 33951 }
id: l-aabf3779
scope: repo
status: active
src: agent
touches:
  - ".github/workflows/codeql-analysis.yml"
  - HISTORY
  - README
  - README.md
  - src/main/version.h
  - src/engine/diskdevice.cxx
  - src/engine/diskdevice.hxx
  - src/asterix/DataItemFormatFixed.cpp
  - src/asterix/DataItemFormatFixed.h
  - src/asterix/DataRecord.cpp
---

## Decided
Tracer::Error formats its message once and hands it to the configured print function as ("%s", buffer). Callers pass text that comes from input files, XML errors or record bytes as an argument: Tracer::Error("%s", text), never Tracer::Error(text).

## Tried and ruled out
Fixing only the call that CodeQL flagged (XMLParser::Error): a definitions file whose name contained %s%s%n still aborted, because Tracer::Error passed the already formatted message to the print function (debug_trace, printf-like) as its format, so a '%' got interpreted a second time. The original code crashed (segfault) on that input.

## Watch out
The print functions set with Tracer::Configure (debug_trace in asterixformat.cxx, the Wireshark printf) are printf-like: any new caller of pPrintFunc/pPrintFunc2 must pass "%s" too.

In the code: src/asterix/Tracer.cpp (Tracer::Error), XMLParser::Error, DataRecord constructor (record bytes dump); test: a definitions XML named bad%s%s%n.xml with a syntax error, listed in a -d ini file
