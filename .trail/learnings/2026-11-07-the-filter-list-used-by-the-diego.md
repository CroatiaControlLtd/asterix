---
title: "The filter list used by the tests must be regenerated whenever the XML definitions change, then the filtered outputs reviewed"
author: Diego
date: 2026-11-07
branch: fix_test_filter
cost: { minutes: 14, tokens: 27024 }
id: l-5bec4220
scope: repo
status: active
replaces: l-2ef9bc59
src: agent
touches:
  - HISTORY
  - README
  - README.md
  - src/main/asterix.cpp
  - src/main/version.h
  - install/sample_output/cat_001_002.txt
  - install/sample_output/cat_001_002_jsonh.txt
  - install/sample_output/cat_034_048.txt
  - install/sample_output/cat_034_048_filtered.txt
  - install/sample_output/parsegps.txt
---

## Decided
filter.txt is the expected output of `asterix -L` (every configured item) and also the -LF input of the filtered CAT001/002 and CAT034/048 tests. It is regenerated with -L whenever install/config definitions change (new category, renamed field), and the filtered expected outputs are regenerated right after and reviewed. With this, install/test/test.sh passes completely.

## Tried and ruled out
Leaving filter.txt stale after a definitions change: "Test filter" failed on master for a long time (CAT015 missing, CAT001 I100 QB1/QD1 renamed QB4/QD4), and the stale names silently dropped the renamed fields from every -LF output.

## Watch out
- A field renamed in the XML disappears from -LF outputs until filter.txt has the new name: check `git diff install/sample_output` after regenerating, an unexpected change there is a field that was being dropped.
- The samples have no CAT001 I100 and no CAT015, so this refresh changed no filtered output; a definitions change that touches items present in the samples will.
- updatetests.sh rewrites filter.txt too: only run it whole when the definitions are meant to change.

In the code: install/test/test.sh ("Test filter" and the -LF tests), install/test/updatetests.sh, install/sample_output/filter.txt; regenerate with: cd install/test && ../../obj/main/release/asterix -d ../config/asterix.ini -L > ../sample_output/filter.txt
