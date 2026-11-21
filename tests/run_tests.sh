#!/bin/bash
#
# Regression tests of the asterix command line tool. See tests/README.md.
#
# usage: tests/run_tests.sh [asterix binary] [section...]
#   binary    default obj/main/release/asterix (make), relative to the repository root
#   sections  golden consistency roundtrip inputs robustness cli (default: all)
# environment:
#   UPDATE=1      rewrite the golden files instead of comparing (review the diff before committing)
#   TIMEOUT=n     seconds allowed per run in the robustness section (default 20)
#   MEMLIMIT_KB=n address space limit of each robustness run (not with sanitizer builds)
#   MUTANTS=dir   also run the mutated inputs in dir (tests/tools/mutate.py) in the robustness section
#   VERBOSE=1     print every passed check too
#
# Exit status: number of failed checks (0 = all passed), capped at 100.

cd "$(dirname "$0")/.." || exit 100
ROOT=$(pwd)
BIN=${1:-obj/main/release/asterix}
shift 2>/dev/null
SECTIONS=${*:-golden consistency roundtrip inputs robustness cli}
case $BIN in /*) ;; *) BIN="$ROOT/$BIN" ;; esac
[ -x "$BIN" ] || { echo "asterix binary not found: $BIN"; exit 100; }

DEFS="install/config/asterix.ini"
FILTER="tests/data/filter_subset.txt"
GOLDEN="tests/golden"
TIMEOUT=${TIMEOUT:-20}
TMP=$(mktemp -d)
trap 'rm -rf "$TMP"' EXIT

# output formats: golden file name and option
FORMATS="txt: line:-l xml:-x xmlh:-xh json:-j jsonh:-jh jsone:-je jsonx:-jx"

passed=0
failed=0
ok() { passed=$((passed + 1)); [ -n "$VERBOSE" ] && echo "OK      $1"; }
fail() { failed=$((failed + 1)); echo "FAILED  $1"; [ -n "$2" ] && echo "$2" | head -40 | sed 's/^/        /'; }

# wall-clock timestamps of raw/ORADIS file input (and of stdin/multicast) are replaced by T
mask_time() { sed -E 's/("timestamp":)[0-9.]+/\1T/g; s/(timestamp=")[0-9.]+/\1T/g; s/(Timestamp: )[0-9.]+/\1T/g'; }
# stderr messages start with the wall-clock time
mask_log() { sed -E 's/^([A-Z]+: +)[A-Z][a-z]{2} +[0-9]+ [0-9:]{8} [0-9]{4} /\1/'; }

cases() { grep -v '^#' tests/cases.txt | grep -v '^[[:space:]]*$'; }

section_golden() {
    echo "== golden outputs (every case x every output format, with and without -LF)"
    local name input mask fmt opt suffix lf out expected
    while IFS='|' read -r name input mask; do
        mkdir -p "$GOLDEN/$name"
        for f in $FORMATS; do
            fmt=${f%%:*}; opt=${f#*:}
            for suffix in "" ".filtered"; do
                lf=""; [ -n "$suffix" ] && lf="-LF $FILTER"
                out="$TMP/out.txt"
                # shellcheck disable=SC2086
                "$BIN" -d $DEFS $input $opt $lf 2>/dev/null > "$out"
                [ "$mask" = "mask" ] && { mask_time < "$out" > "$out.m"; mv "$out.m" "$out"; }
                expected="$GOLDEN/$name/$fmt$suffix.txt"
                if [ -n "$UPDATE" ]; then
                    cp "$out" "$expected"
                elif [ ! -s "$out" ]; then
                    fail "golden $name $fmt$suffix" "no output"
                elif cmp -s "$out" "$expected"; then
                    ok "golden $name $fmt$suffix"
                else
                    fail "golden $name $fmt$suffix: $BIN -d $DEFS $input $opt $lf" "$(diff -u "$expected" "$out" 2>&1)"
                fi
            done
        done
    done < <(cases)
}

section_consistency() {
    echo "== consistency of -jx (fspec/hexdata/crc, hex/mask/pos/val, unique keys) and across formats"
    local r
    if r=$(python3 tests/tools/check_outputs.py "$BIN" 2>&1); then ok "consistency"; [ -n "$VERBOSE" ] && echo "$r"
    else fail "consistency" "$r"; fi
}

section_roundtrip() {
    echo "== write-back: edit values with hex/mask/pos, rebuild the records and decode them again"
    local r
    if r=$(python3 tests/tools/roundtrip.py "$BIN" 2>&1); then ok "roundtrip"; [ -n "$VERBOSE" ] && echo "$r"
    else fail "roundtrip" "$r"; fi
}

section_inputs() {
    echo "== input channels: stdin, UDP multicast"
    local f a b
    # stdin (no -f / -i) must decode a raw stream like file input
    for f in install/sample_data/cat062cat065.raw tests/data/multicat.raw; do
        a=$("$BIN" -d $DEFS -jx -f "$f" 2>/dev/null | mask_time)
        b=$("$BIN" -d $DEFS -jx < "$f" 2>/dev/null | mask_time)
        if [ -n "$a" ] && [ "$a" = "$b" ]; then ok "stdin $f"; else fail "stdin $f" "$(diff <(echo "$a") <(echo "$b") | head)"; fi
    done
    # loop mode (-o): the file is read again from the start, so the same records keep coming
    a=$("$BIN" -d $DEFS -jx -f install/sample_data/cat062cat065.raw 2>/dev/null | mask_time)
    b=$(timeout 2 "$BIN" -d $DEFS -jx -o -f install/sample_data/cat062cat065.raw 2>/dev/null | head -9 | mask_time)
    if [ -n "$a" ] && [ "$b" = "$(printf '%s\n%s\n%s' "$a" "$a" "$a")" ]; then ok "loop mode -o"
    else fail "loop mode -o" "$(echo "$b" | head -4 | cut -c1-120)"; fi
    # UDP multicast on the loopback interface: 3 datagrams of cat062cat065.raw = 9 records
    local port=$((21000 + RANDOM % 1000)) out="$TMP/mcast.txt" pid n
    "$BIN" -d $DEFS -jx -i "232.1.1.11:127.0.0.1:$port" > "$out" 2>"$TMP/mcast.err" &
    pid=$!
    sleep 1
    python3 tests/tools/mcast_send.py 232.1.1.11 "$port" install/sample_data/cat062cat065.raw 3
    for _ in 1 2 3 4 5 6 7 8 9 10; do
        n=$(wc -l < "$out"); [ "$n" -ge 9 ] && break; sleep 0.5
    done
    kill "$pid" 2>/dev/null; wait "$pid" 2>/dev/null
    a=$("$BIN" -d $DEFS -jx -f install/sample_data/cat062cat065.raw 2>/dev/null | mask_time | sed -E 's/"(id|srcip)":[^,]*,//g')
    b=$(head -3 "$out" | mask_time | sed -E 's/"(id|srcip)":[^,]*,//g')
    if [ "$n" -eq 9 ] && [ "$a" = "$b" ] && grep -q '"srcip":"127.0.0.1"' "$out"; then ok "multicast"
    else fail "multicast ($n records, expected 9)" "$(cat "$TMP/mcast.err"; diff <(echo "$a") <(echo "$b") | head)"; fi
}

# a run must end in time, without a signal and without sanitizer reports
run_guarded() {
    local label=$1; shift
    # MEMLIMIT_KB caps the address space, so that a huge allocation fails the same way on every machine
    # (not with sanitizers: they reserve a lot of virtual memory)
    ( if [ -n "$MEMLIMIT_KB" ]; then ulimit -v "$MEMLIMIT_KB"; fi
      exec timeout "$TIMEOUT" "$@" ) > /dev/null 2> "$TMP/err.txt"
    local rc=$?
    if [ $rc -eq 124 ]; then fail "$label: timeout after ${TIMEOUT}s"; return; fi
    if [ $rc -ge 128 ]; then fail "$label: killed by signal $((rc - 128))" "$(tail -20 "$TMP/err.txt")"; return; fi
    if grep -qE "AddressSanitizer|runtime error|LeakSanitizer|UndefinedBehaviorSanitizer" "$TMP/err.txt"; then
        fail "$label: sanitizer report" "$(grep -m5 -E 'ERROR|runtime error|SUMMARY|#[0-9] ' "$TMP/err.txt")"; return
    fi
    ok "$label"
}

input_option() {
    case $1 in *.rpcap) echo -R ;; *.pcap) echo -P ;; *.oradis) echo -O ;; *.final) echo -F ;; *.hdlc) echo -H ;; *.gps) echo -G ;; *) echo "" ;; esac
}

section_robustness() {
    echo "== robustness: malformed inputs and every case in every format end cleanly"
    local f opt name input mask fmt
    for f in tests/data/bad/*; do
        opt=$(input_option "$f")
        for fmt in "" -jx -je -xh; do
            # shellcheck disable=SC2086
            run_guarded "bad $(basename "$f") ${fmt:-txt}" "$BIN" -d $DEFS $opt -f "$f" $fmt
        done
    done
    # optional mutated corpus (tests/tools/mutate.py), used by the sanitizer job
    if [ -n "$MUTANTS" ] && [ -d "$MUTANTS" ]; then
        for f in "$MUTANTS"/*; do
            opt=$(input_option "$f")
            # shellcheck disable=SC2086
            run_guarded "mutant $(basename "$f")" "$BIN" -d $DEFS $opt -f "$f" -jx
            # shellcheck disable=SC2086
            run_guarded "mutant $(basename "$f") -LF" "$BIN" -d $DEFS $opt -f "$f" -je -LF $FILTER
        done
    fi
    # whatever the input, -j and -jx must be valid JSON (text fields only keep characters that need no escaping)
    local jsonbad
    jsonbad=$(for f in tests/data/bad/* ${MUTANTS:+"$MUTANTS"/*}; do
                  opt=$(input_option "$f")
                  for fmt in -j -jx; do
                      # shellcheck disable=SC2086
                      timeout "$TIMEOUT" "$BIN" -d $DEFS $opt -f "$f" $fmt 2>/dev/null
                  done
              done | python3 -c '
import json, sys
bad = 0
for line in sys.stdin:
    try:
        json.loads(line)
    except ValueError:
        bad += 1
        if bad <= 3:
            print(line[:200])
print(bad)' )
    if [ "$(echo "$jsonbad" | tail -1)" = "0" ]; then ok "-j/-jx output of malformed inputs is valid JSON"
    else fail "-j/-jx output of malformed inputs is not valid JSON" "$jsonbad"; fi
    # a '%' in the definitions file name or in the XML error used to be read as a printf format
    mkdir -p "$TMP/defs"
    printf '<?xml version="1.0"?>\n<Category id="1">\n<Bad%%s%%s%%n>\n</Category>\n' > "$TMP/defs/bad%s%s%n.xml"
    echo "$TMP/defs/bad%s%s%n.xml" > "$TMP/defs/bad.ini"
    run_guarded "definitions file with %s%n in its name and a syntax error" "$BIN" -d "$TMP/defs/bad.ini" -f install/sample_data/cat048.raw
}

section_cli() {
    echo "== command line"
    local out rc
    out=$("$BIN" -h 2>&1)
    if echo "$out" | grep -q "(default)" && echo "$out" | grep -q -- "-xh,--xmlh" && echo "$out" | grep -q "\[-l|-x|-xh|-j|-jh|-je|-jx\]"; then
        ok "-h documents the default output and every format"; else fail "-h text" "$out"; fi
    out=$("$BIN" -V 2>&1)
    if echo "$out" | grep -qE "Asterix [0-9]+\.[0-9]+\.[0-9]+-ifsnop\.[0-9]+"; then ok "-V"; else fail "-V" "$out"; fi
    "$BIN" -d $DEFS -j -x -f install/sample_data/cat048.raw > /dev/null 2>&1; rc=$?
    if [ $rc -eq 1 ]; then ok "two output formats are rejected"; else fail "two output formats: exit $rc, expected 1"; fi
    "$BIN" -d "$TMP/missing.ini" -f install/sample_data/cat048.raw > /dev/null 2>&1; rc=$?
    if [ $rc -eq 2 ]; then ok "missing definitions file"; else fail "missing definitions file: exit $rc, expected 2"; fi
    # a block with a malformed record: same stdout, a warning with the whole block on stderr
    out=$("$BIN" -d $DEFS -jx -f tests/data/bad/raw_broken_record_in_block.raw 2>&1 >/dev/null | mask_log)
    local hex
    hex=$(od -An -tx1 -v tests/data/bad/raw_broken_record_in_block.raw | tr -d ' \n' | tr 'a-f' 'A-F')
    if echo "$out" | grep -q "^WARNING: .*malformed record .*Block: $hex\$"; then ok "warning with the block of a malformed record"
    else fail "no warning with the block of a malformed record" "$out"; fi
    out=$("$BIN" -d $DEFS -L 2>/dev/null)
    if [ "$out" = "$(cat install/sample_output/filter.txt)" ]; then ok "-L matches install/sample_output/filter.txt"
    else fail "-L differs from install/sample_output/filter.txt (definitions changed? regenerate it)" "$(diff <(echo "$out") install/sample_output/filter.txt | head)"; fi
}

for s in $SECTIONS; do "section_$s"; done
echo
if [ -n "$UPDATE" ]; then echo "golden files updated in $GOLDEN: review 'git diff $GOLDEN' before committing"; fi
echo "passed: $passed  failed: $failed"
[ $failed -gt 100 ] && exit 100
exit $failed
