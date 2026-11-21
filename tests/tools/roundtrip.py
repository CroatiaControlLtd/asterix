#!/usr/bin/env python3
"""
Write-back test of -jx, the reason that output exists (see README.md):

For every record of the raw cases in tests/cases.txt, every integer field that only describes data
(not FX, spare, or a UAP selector) gets a new value written with
    hexdata[pos..] = (hex & ~mask) | (value & mask)
The edited records (fspec + hexdata) are written as a raw ASTERIX stream, decoded again with -jx,
and every field must hold the new value while everything else stays the same.

usage: roundtrip.py <asterix binary>
"""
import json
import os
import subprocess
import sys
import tempfile

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFS = 'install/config/asterix.ini'

# fields whose value changes how the rest of the record decodes (UAP selection): never edited
STRUCTURAL = {('CAT001', 'I020', 'TYP')}


def decode(binary, path):
    r = subprocess.run([binary, '-d', DEFS, '-jx', '-f', path], cwd=ROOT,
                       stdout=subprocess.PIPE, stderr=subprocess.DEVNULL, check=False)
    return [json.loads(x) for x in r.stdout.decode('latin-1').splitlines()]


def leaves(node, path=()):
    """(path, field) of every -jx field under node"""
    if isinstance(node, dict):
        if 'hex' in node and 'pos' in node:
            yield path, node
        else:
            for k, v in node.items():
                yield from leaves(v, path + (k,))
    elif isinstance(node, list):
        for i, v in enumerate(node):
            yield from leaves(v, path + (i,))


def editable(cat, path, f):
    name = path[-1] if isinstance(path[-1], str) else ''
    return (isinstance(f.get('val'), int) and not isinstance(f['val'], bool) and f['val'] >= 0
            and not name.startswith('FX') and not name.startswith('spare')
            and len(path) == 2  # item / field: Fixed or Variable items, not Compound/Repetitive internals
            and (cat, path[0], name) not in STRUCTURAL)


def edit(rec):
    """new hexdata for rec and {path: new value}"""
    cat = 'CAT%03d' % rec['cat']
    data = bytearray.fromhex(rec['hexdata'])
    expected = {}
    for path, f in leaves(rec[cat]):
        if not editable(cat, path, f):
            continue
        nbytes = len(f['hex']) // 2
        mask = int(f['mask'], 2) if 'mask' in f else (1 << (8 * nbytes)) - 1
        shift = (mask & -mask).bit_length() - 1
        width = bin(mask).count('1')
        new = (f['val'] + 1) % (1 << width)  # always a different value that fits
        pos = f['pos']
        old = int.from_bytes(data[pos:pos + nbytes], 'big')
        word = (old & ~mask) | ((new << shift) & mask)
        data[pos:pos + nbytes] = word.to_bytes(nbytes, 'big')
        expected[path] = new
    return bytes(data), expected


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    binary = os.path.abspath(sys.argv[1])
    raw_cases = []
    for line in open(os.path.join(ROOT, 'tests', 'cases.txt')):
        line = line.strip()
        if line and not line.startswith('#'):
            name, args, _ = line.split('|')
            a = args.split()
            if a[0] == '-f':  # plain raw ASTERIX input: records can be written back the same way
                raw_cases.append((name, a[1]))

    errors, nrec, nfields = [], 0, 0
    with tempfile.TemporaryDirectory() as tmp:
        for name, path in raw_cases:
            original = decode(binary, path)
            stream, edits = b'', []
            for rec in original:
                hexdata, expected = edit(rec)
                record = bytes.fromhex(rec['fspec']) + hexdata
                stream += bytes([rec['cat']]) + (3 + len(record)).to_bytes(2, 'big') + record
                edits.append(expected)
            out = os.path.join(tmp, name + '.raw')
            with open(out, 'wb') as f:
                f.write(stream)
            again = decode(binary, out)
            if len(again) != len(original):
                errors.append('%s: %d records decoded back, expected %d' % (name, len(again), len(original)))
                continue
            for n, (a, b, expected) in enumerate(zip(original, again, edits), 1):
                cat = 'CAT%03d' % a['cat']
                va = dict(leaves(a[cat]))
                vb = dict(leaves(b[cat]))
                if va.keys() != vb.keys():
                    errors.append('%s record %d: fields changed: %s' % (name, n, sorted(set(va) ^ set(vb), key=str)[:5]))
                    continue
                for p in va:
                    want = expected.get(p, va[p]['val'])
                    if vb[p]['val'] != want:
                        errors.append('%s record %d %s: decoded %r, expected %r' % (name, n, '/'.join(map(str, p)),
                                                                                   vb[p]['val'], want))
                nrec += 1
                nfields += len(expected)
    print('%d records rewritten, %d field values edited and decoded back' % (nrec, nfields))
    if errors:
        print('\n'.join(errors[:40]))
        sys.exit(1)
    if nfields == 0:
        sys.exit('no field was edited')


if __name__ == '__main__':
    main()
