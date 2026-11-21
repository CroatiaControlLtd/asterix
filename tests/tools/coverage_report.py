#!/usr/bin/env python3
"""
Line coverage of src/ from the .gcda files of a --coverage build, after running the tests.

Prints a table per file and the total; with $GITHUB_STEP_SUMMARY set, also writes it there; with
$COVERAGE_BADGE set, writes a shields.io endpoint JSON file there (the README coverage badge).
Fails when the total is under the minimum in tests/coverage_min.txt (raise that file when coverage
goes up, so it cannot silently go down again).

usage: coverage_report.py <build directory>
"""
import os
import re
import subprocess
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SRC = os.path.join(ROOT, 'src') + os.sep
# not part of the command line tool
EXCLUDE = ('src/asterix/WiresharkWrapper', 'src/python/', 'src/asterix/wireshark-plugin', 'src/asterix/ethereal-plugin')


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    build = os.path.abspath(sys.argv[1])
    files = {}
    for d, _, names in os.walk(build):
        for n in names:
            if not n.endswith('.gcda'):
                continue
            out = subprocess.run(['gcov', '-n', '-o', d, os.path.join(d, n)], cwd=ROOT,
                                 stdout=subprocess.PIPE, stderr=subprocess.DEVNULL).stdout.decode()
            for path, pct, total in re.findall(r"File '([^']+)'\nLines executed:([0-9.]+)% of (\d+)", out):
                full = os.path.normpath(os.path.join(ROOT, path))
                if not full.startswith(SRC) or not full.endswith(('.cpp', '.cxx', '.c')):
                    continue
                rel = os.path.relpath(full, ROOT)
                if rel.startswith(EXCLUDE):
                    continue
                covered = round(float(pct) * int(total) / 100)
                if rel not in files or covered > files[rel][0]:
                    files[rel] = (covered, int(total))
    if not files:
        sys.exit('no coverage data under %s' % build)
    covered = sum(c for c, _ in files.values())
    total = sum(t for _, t in files.values())
    pct = 100.0 * covered / total
    minimum = float(open(os.path.join(ROOT, 'tests', 'coverage_min.txt')).read().split()[0])

    lines = ['| File | Lines | Covered |', '|---|---:|---:|']
    for rel, (c, t) in sorted(files.items(), key=lambda kv: kv[1][0] / max(kv[1][1], 1)):
        lines.append('| %s | %d | %.1f%% |' % (rel, t, 100.0 * c / t if t else 100))
    lines.append('| **total** | **%d** | **%.1f%%** |' % (total, pct))
    report = '\n'.join(lines)
    print(report)
    print('\nline coverage %.1f%% (minimum %.1f%%)' % (pct, minimum))
    summary = os.environ.get('GITHUB_STEP_SUMMARY')
    if summary:
        with open(summary, 'a') as f:
            f.write('## Line coverage: %.1f%% (minimum %.1f%%)\n\n%s\n' % (pct, minimum, report))
    badge = os.environ.get('COVERAGE_BADGE')
    if badge:  # shields.io endpoint JSON, published by CI for the README badge
        color = ('brightgreen' if pct >= 80 else 'green' if pct >= 70 else 'yellowgreen' if pct >= 60
                 else 'yellow' if pct >= 50 else 'orange' if pct >= 40 else 'red')
        with open(badge, 'w') as f:
            f.write('{"schemaVersion": 1, "label": "coverage", "message": "%.1f%%", "color": "%s"}\n' % (pct, color))
    if pct < minimum:
        sys.exit('coverage %.1f%% is under the minimum %.1f%% (tests/coverage_min.txt)' % (pct, minimum))


if __name__ == '__main__':
    main()
