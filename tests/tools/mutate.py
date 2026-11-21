#!/usr/bin/env python3
"""
Write mutated copies of the test inputs (byte flips, inserted/deleted bytes, truncation, extreme length
fields) to a directory, for the robustness run under sanitizers (MUTANTS=dir tests/run_tests.sh ...).
The valid samples never reach the error paths: mutants do. Deterministic for a given seed.

usage: mutate.py <output dir> [mutants per input, default 40] [seed, default 1]
"""
import os
import random
import sys

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))


def inputs():
    for line in open(os.path.join(ROOT, 'tests', 'cases.txt')):
        line = line.strip()
        if line and not line.startswith('#'):
            args = line.split('|')[1].split()
            yield args[args.index('-f') + 1], '-R' in args


def mutate(data, rnd):
    d = bytearray(data)
    kind = rnd.randrange(6)
    if kind == 0:    # flip a few bytes
        for _ in range(rnd.randint(1, 8)):
            d[rnd.randrange(len(d))] = rnd.randrange(256)
    elif kind == 1:  # flip single bits
        for _ in range(rnd.randint(1, 16)):
            d[rnd.randrange(len(d))] ^= 1 << rnd.randrange(8)
    elif kind == 2:  # truncate
        d = d[:rnd.randrange(1, len(d))]
    elif kind == 3:  # insert or delete a run of bytes
        p = rnd.randrange(len(d))
        if rnd.random() < 0.5:
            d[p:p] = bytes(rnd.randrange(256) for _ in range(rnd.randint(1, 16)))
        else:
            del d[p:p + rnd.randint(1, 16)]
    elif kind == 4:  # extreme 16-bit values (length fields)
        for _ in range(rnd.randint(1, 4)):
            p = rnd.randrange(max(1, len(d) - 1))
            d[p:p + 2] = rnd.choice([b'\x00\x00', b'\x00\x01', b'\x00\x03', b'\xff\xff', b'\x7f\xff', b'\x80\x00'])
    else:            # every FX bit set from a random point (FSPEC / Variable items that never end)
        p = rnd.randrange(len(d))
        for i in range(p, min(len(d), p + rnd.randint(1, 32))):
            d[i] |= 1
    return bytes(d)


def main():
    if len(sys.argv) < 2:
        sys.exit(__doc__)
    out = sys.argv[1]
    per = int(sys.argv[2]) if len(sys.argv) > 2 else 40
    rnd = random.Random(int(sys.argv[3]) if len(sys.argv) > 3 else 1)
    os.makedirs(out, exist_ok=True)
    n = 0
    for path, oradis_pcap in inputs():
        data = open(os.path.join(ROOT, path), 'rb').read()
        base, ext = os.path.splitext(os.path.basename(path))
        if oradis_pcap:
            ext = '.rpcap'  # read with -R (PCAP with ORADIS inside)
        for i in range(per):
            # keep the extension: run_tests.sh picks the input option from it
            with open(os.path.join(out, '%s_m%03d%s' % (base, i, ext)), 'wb') as f:
                f.write(mutate(data, rnd))
            n += 1
    print('%d mutants in %s' % (n, out))


if __name__ == '__main__':
    main()
