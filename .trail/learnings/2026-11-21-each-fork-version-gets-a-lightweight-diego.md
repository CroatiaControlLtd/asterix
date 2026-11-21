---
title: "Each fork version gets a lightweight tag on master and, from ifsnop.6 on, a GitHub release with notes taken from HISTORY"
author: Diego
date: 2026-11-21
branch: master
cost: { minutes: 10, tokens: 227750 }
id: l-b29de63e
scope: repo
status: active
src: agent
touches:
  - HISTORY
  - README
  - README.md
  - src/main/version.h
  - src/asterix/Category.cpp
  - src/asterix/DataItem.cpp
  - src/asterix/DataItem.h
  - src/asterix/DataItemBits.cpp
  - src/asterix/DataItemDescription.cpp
  - src/asterix/DataItemDescription.h
---

## Decided
- Versions up to ifsnop.5 only had a git tag. From 2.8.11-ifsnop.6 on, each version also gets a GitHub release, because the user asked for one. Its notes are the HISTORY entry written for users: what changed, any output difference from the previous fork version, and "python_v0.7.11 unchanged".
- Tags stay lightweight, named v<upstream>-ifsnop.<n>, like the earlier ones. They go on the master commit that contains the version change; when it came in through a PR, that is the merge commit.
- No binaries are attached to the release unless asked; GitHub's source archives are enough.

## Watch out
- Create the release only after the PR with the version change is merged and its CI is green, so the tag points at tested code.

In the code: git tag v2.8.11-ifsnop.<n> <master commit>; git push origin <tag>; gh release create <tag> --verify-tag --title <tag> --notes-file -
