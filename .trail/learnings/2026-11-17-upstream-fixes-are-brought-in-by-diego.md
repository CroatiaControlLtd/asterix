---
title: "Upstream fixes are brought in by cherry-pick keeping the original author, and XML definitions are kept in step in both config copies"
author: Diego
date: 2026-11-17
branch: cat048-re-upstream-254
cost: { minutes: 5, tokens: 40690 }
id: l-f52d4ed9
scope: repo
status: active
src: agent
touches:
  - install/config/asterix_cat048_1_21.xml
  - install/sample_output/filter.txt
  - asterix/config/asterix_cat048_1_21.xml
  - asterix/test/test_parse.py
  - setup.py
---

## Decided
Changes from CroatiaControlLtd/asterix PRs are fetched by PR ref and cherry-picked with -x onto a branch, so the upstream author stays as Author and the origin commit is recorded in the message. Anything this fork has to add (syncing the other config copy, test data) goes in a separate commit of ours, never amended into the upstream one, so attribution stays clean. Merge into master without squash for the same reason.

## Tried and ruled out
Taking an upstream XML fix as-is: upstream often edits only the Python package copy, so the C++ binary (and test.sh) kept the old definition and still failed on the record (CAT048 v1.21 RE: "DataItem format not defined for CAT048/IRE").

## Watch out
- The two copies of the definitions are meant to be identical (cmp them); after any XML change in one, copy it to the other.
- A new or renamed item means regenerating filter.txt (see the filter-list learning) and running test.sh.
- Fetching the upstream URL also pulls its tags locally; don't push --tags blindly.

In the code: asterix/config/*.xml (Python module, packaged by setup.py) and install/config/*.xml (C++ binary, test.sh); git fetch https://github.com/CroatiaControlLtd/asterix.git pull/<N>/head:upstream-pr-<N> && git cherry-pick -x <sha>; example: branch cat048-re-upstream-254 (PR #254).
