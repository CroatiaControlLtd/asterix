# Tests

Everything here runs automatically on every push and pull request (`.github/workflows/tests.yml`).
A change is ready when all jobs are green; when a golden output changes on purpose, the diff of
`tests/golden` is part of the review.

## Layers

| What | Where | Catches |
|---|---|---|
| Expected outputs (legacy) | `install/test/test.sh` | changes in 11 reference outputs, memory leaks (valgrind) |
| Golden outputs | `tests/run_tests.sh` section `golden` | any change in any output format: 13 inputs (`tests/cases.txt`) × 8 formats × with/without `-LF` |
| Consistency | `tests/tools/check_outputs.py` | `-jx` fields whose hex/mask/pos/val do not match the record bytes, wrong crc or fspec, duplicate keys; `-j`, `-jh`, `-je`, text and XML disagreeing with `-jx` |
| Write-back | `tests/tools/roundtrip.py` | `-jx` values that cannot be edited with `hexdata[pos..] = (hex & ~mask) \| (value & mask)` and decoded back |
| Input channels | `run_tests.sh` section `inputs` | stdin, UDP multicast (loopback), loop mode `-o` |
| Robustness | `run_tests.sh` section `robustness` | crashes, hangs and sanitizer reports on malformed inputs (`tests/data/bad`), mutated inputs (`MUTANTS=dir`), `%` in file names |
| Command line | `run_tests.sh` section `cli` | `-h`, `-V`, option errors, `-L` vs `install/sample_output/filter.txt` |
| Unit tests | `tests/unit` (CMake, `ctest`) | bit extraction and encodings, `getLength` contract of every item of every category, parser edge cases, UAP selection, record headers, printf-equivalent helpers, regressions |
| Fuzzing | `tests/fuzz/fuzz_parse.cpp` (libFuzzer) | crashes and sanitizer reports on arbitrary packets, every output format |
| Coverage | `tests/tools/coverage_report.py` | coverage under `tests/coverage_min.txt` |

CI jobs: `make` (make build, `install/test`, `run_tests.sh`), `cmake` (C++11 build with gcc and
clang, unit tests and `run_tests.sh` through ctest), `sanitizers` (ASan+UBSan build: unit tests,
`run_tests.sh` with 520 mutated inputs), `fuzz` (60 s per push, 30 min weekly), `coverage`.
The README badges show the state of this workflow on `master` and its coverage: on each push to
`master` the coverage job writes `coverage.json` to the `badges` branch (one file, one commit,
rewritten every time), which shields.io reads.

## Running locally

```sh
make -C src && make -C src debug          # binaries used by install/test and run_tests.sh
(cd install/test && bash test.sh)
tests/run_tests.sh                        # all sections; or e.g. tests/run_tests.sh obj/main/release/asterix golden cli
VERBOSE=1 tests/run_tests.sh              # list every check

cmake -S . -B build -DASTERIX_TESTS=ON && cmake --build build && ctest --test-dir build --output-on-failure

# sanitizers and mutated inputs, as in CI
F="-fsanitize=address,undefined -fno-sanitize-recover=all -fno-omit-frame-pointer -O1 -g"
cmake -S . -B build-asan -DASTERIX_TESTS=ON -DCMAKE_C_FLAGS="$F" -DCMAKE_CXX_FLAGS="$F" -DCMAKE_EXE_LINKER_FLAGS="-fsanitize=address,undefined"
cmake --build build-asan
python3 tests/tools/mutate.py /tmp/mutants 40 1
MUTANTS=/tmp/mutants tests/run_tests.sh build-asan/install/asterix
```

The tools only need Python 3 (standard library). The Python module (`asterix/`) is not covered yet.

## When something changes on purpose

- **Output format change**: `UPDATE=1 tests/run_tests.sh obj/main/release/asterix golden`, review
  `git diff tests/golden`, and regenerate the affected files of `install/sample_output` the same way
  (see `install/test/updatetests.sh`).
- **Definitions change** (`install/config`): regenerate `install/sample_output/filter.txt` with
  `asterix -L` and review the filtered outputs; check that `tests/data/filter_subset.txt` still names
  existing fields.
- **New input format or sample**: add a line to `tests/cases.txt`, then `UPDATE=1` the golden files.
  Synthetic inputs are built by `tests/tools/gen_samples.py` from the real samples (CI checks that the
  committed files match the script).
- **A bug fixed**: add the input that triggered it to `tests/data/bad` (through `gen_samples.py`) or a
  unit test in `tests/unit/test_regressions.cpp`.
- **Coverage went up**: raise `tests/coverage_min.txt`.

## Not covered

- TCP and serial input: not reachable from the command line options.
- KML output (`-k`): not documented, not tested.
- The Python module and the Wireshark plugin.
