---
title: "Fork versions are <upstream>-ifsnop.<n>; the make build now rebuilds objects when a header changes"
author: Diego
date: 2026-11-07
branch: fix_make_depend
cost: { minutes: 6, tokens: 32719 }
id: l-949a9c1c
scope: repo
status: active
replaces: l-b9b7da47
src: agent
touches:
  - src/main/version.h
  - README.md
  - src/makefile.rules
---

## Decided
Fork versions are <upstream>-ifsnop.<n>. A bump touches src/main/version.h, the version line in README and README.md, and a new HISTORY entry (marked "python_v0.7.11 unchanged", since the Python module is not maintained in this fork). Bump n when a change is incompatible with the previous fork release: 2.8.11-ifsnop.2 came from the -j header change (fspec split from hexdata).

The make build tracks header dependencies with gcc -MMD -MP: touching version.h (or any header) rebuilds every object that includes it and relinks.

## Tried and ruled out
The old rules generated dependencies but never matched: a sed rewrote the gcc target as $(OBJ_DIR_SED)/X.o, and OBJ_DIR_SED was never defined, so the rules were for /X.o at the filesystem root. Objects only depended on their .cpp: an edited version.h left -V on the old version, and a changed class layout (UAP.h) linked objects with two layouts and crashed in malloc. Defining OBJ_DIR_SED was dropped for -MMD, which names the target after -o with no escaping.

## Watch out
- On a branch without this fix (or with .d files from before it), headers are still not tracked: make clean, or touch the sources, after editing a header.
- After switching to this fix, do one make clean so every object gets a valid .d.
- -MMD leaves system headers out on purpose: a compiler/libstdc++ upgrade needs make clean.

In the code: src/makefile.rules (the $(OBJ_DIR)/%.o rules, -include of the .d files at the end); check with obj/main/release/asterix -V; grep -rn "ifsnop\.<old n>" outside obj/ and .git/ to find every place that names the version
