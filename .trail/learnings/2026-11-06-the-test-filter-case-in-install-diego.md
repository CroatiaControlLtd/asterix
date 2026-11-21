---
title: "The 'Test filter' case in install/test fails on master and its expected file was left alone on purpose"
author: Diego
date: 2026-11-06
branch: add_ip_to_gps
cost: { minutes: 6, tokens: 101626 }
id: l-2ef9bc59
scope: repo
status: active
src: agent
touches:
  - src/asterix/DataBlock.cpp
  - src/python/python_parser.cpp
  - src/engine/udpdevice.cxx
  - src/engine/basedevice.hxx
  - install/test/test.sh
  - install/test/updatetests.sh
  - install/sample_output/filter.txt
---

## Decided
When the source-IP work changed the decoded output, only the data outputs in install/sample_output were regenerated. filter.txt was not, even though "Test filter" fails.

## Watch out
"Test filter" (asterix -L compared with sample_output/filter.txt) already fails on master. The XML definitions in install/config moved on (CAT015 added, CAT001 I100 QB1/QD1 renamed QB4/QD4), but filter.txt was never refreshed. filter.txt is also the -LF input for the CAT001/002 and CAT034/048 filtered tests. Regenerating it changes which items those tests print, so do that in its own change and review the filtered outputs, not as a side effect of updatetests.sh.
Do not run updatetests.sh wholesale for an unrelated change: it rewrites filter.txt too.

In the code: install/test/test.sh (Test filter), install/test/updatetests.sh, install/sample_output/filter.txt
