#!/usr/bin/env python3
"""
Consistency checks on the decoder output, for every case in tests/cases.txt.

-jx, every record:
  - fspec is well formed (FX bit set on every byte but the last) and fspec + hexdata has "length" bytes
  - "crc" is the CRC-32 of fspec + hexdata (zlib polynomial, initial value 1, as DataRecord computes it)
  - keys are unique in every JSON object
  - every field: hexdata[pos..] starts with "hex"; "mask" (when present) has one bit per hex bit and
    leaves at least one bit out; an integer "val" is the masked bits of hex (two's complement if negative)
Across formats, for every record:
  - fspec and hexdata are the same in -j, -jx, -jh, -je and the text output, and XML hexdata is
    fspec + hexdata
  - every field printed once in -j has the same value as "val" in -jx

usage: check_outputs.py <asterix binary>     (run from anywhere)
"""
import json
import os
import re
import subprocess
import sys
import zlib

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
DEFS = 'install/config/asterix.ini'
errors = []


def err(msg):
    errors.append(msg)


def cases():
    for line in open(os.path.join(ROOT, 'tests', 'cases.txt')):
        line = line.strip()
        if line and not line.startswith('#'):
            name, args, _ = line.split('|')
            yield name, args.split()


def run(binary, args, fmt):
    cmd = [binary, '-d', DEFS] + args + ([fmt] if fmt else [])
    r = subprocess.run(cmd, cwd=ROOT, stdout=subprocess.PIPE, stderr=subprocess.DEVNULL)
    return r.stdout.decode('latin-1')


def pairs_unique(pairs):
    keys = [k for k, _ in pairs]
    dup = {k for k in keys if keys.count(k) > 1}
    if dup:
        raise ValueError('duplicate keys %s' % sorted(dup))
    return dict(pairs)


def fspec_ok(fspec):
    b = bytes.fromhex(fspec)
    return len(b) > 0 and all(x & 1 for x in b[:-1]) and not (b[-1] & 1)


def check_field(where, f, hexdata):
    h, pos = f['hex'], f['pos']
    if hexdata[2 * pos:2 * pos + len(h)] != h:
        err('%s: hex %s is not at pos %d of hexdata' % (where, h, pos))
        return
    nbits = 4 * len(h)
    m = f.get('mask')
    if m is not None:
        if len(m) != nbits or set(m) - set('01') or '0' not in m or '1' not in m:
            err('%s: bad mask %s for hex %s' % (where, m, h))
            return
        mask = int(m, 2)
    else:
        mask = (1 << nbits) - 1
    v = f.get('val')
    if isinstance(v, int) and not isinstance(v, bool):
        shift = (mask & -mask).bit_length() - 1
        raw = (int(h, 16) & mask) >> shift
        width = bin(mask).count('1')
        if not (v == raw or (v < 0 and v == raw - (1 << width))):
            err('%s: val %d does not match bits %s of hex %s (%d)' % (where, v, m or 'all', h, raw))


def walk_jx(where, node, hexdata, count):
    if isinstance(node, dict):
        if 'hex' in node and 'pos' in node:
            count[0] += 1
            check_field(where, node, hexdata)
        else:
            for k, v in node.items():
                walk_jx(where + '/' + k, v, hexdata, count)
    elif isinstance(node, list):
        for i, v in enumerate(node):
            walk_jx('%s[%d]' % (where, i), v, hexdata, count)


def compare_j_jx(where, j, jx):
    """j: list of (key, value) pairs (duplicates kept); jx: parsed -jx node"""
    if isinstance(jx, dict) and 'hex' in jx and 'pos' in jx:
        if j != jx.get('val'):
            err('%s: -j value %r != -jx val %r' % (where, j, jx.get('val')))
        return 1
    n = 0
    if isinstance(j, list) and j and isinstance(j[0], tuple) and isinstance(jx, dict):
        keys = [k for k, _ in j]
        for k, v in j:
            if keys.count(k) == 1 and k in jx:  # repeated names are numbered in -jx: not comparable
                n += compare_j_jx(where + '/' + k, v, jx[k])
    elif isinstance(j, list) and isinstance(jx, list):
        if len(j) != len(jx):
            err('%s: -j has %d elements, -jx %d' % (where, len(j), len(jx)))
        for i, (a, b) in enumerate(zip(j, jx)):
            n += compare_j_jx('%s[%d]' % (where, i), a, b)
    return n


def headers(text, fspec_re, hex_re):
    """(fspec, hexdata) of each record of a text output, hexdata taken from the line after fspec"""
    out, lines = [], text.split('\n')
    for i, line in enumerate(lines):
        m = re.fullmatch(fspec_re, line)
        if m:
            h = re.fullmatch(hex_re, lines[i + 1]) if i + 1 < len(lines) else None
            out.append((m.group(1), h.group(1) if h else None))
    return out


def check_case(binary, name, args):
    jx_text = run(binary, args, '-jx')
    records = []
    nfields = 0
    for n, line in enumerate(jx_text.splitlines(), 1):
        where = '%s -jx record %d' % (name, n)
        try:
            rec = json.loads(line, object_pairs_hook=pairs_unique)
        except ValueError as e:
            err('%s: %s' % (where, e))
            continue
        fspec, hexdata = rec['fspec'], rec['hexdata']
        records.append((fspec, hexdata))
        if not fspec_ok(fspec):
            err('%s: malformed fspec %s' % (where, fspec))
        whole = bytes.fromhex(fspec + hexdata)
        if len(whole) != rec['length']:
            err('%s: fspec+hexdata has %d bytes, length is %d' % (where, len(whole), rec['length']))
        if zlib.crc32(whole, 1) != int(rec['crc'], 16):
            err('%s: crc %s does not match the record bytes' % (where, rec['crc']))
        count = [0]
        walk_jx(where, rec['CAT%03d' % rec['cat']], hexdata, count)
        nfields += count[0]
    if not records:
        err('%s: no -jx records' % name)
        return 0, 0, 0

    # -j: same header, same values
    j_lines = run(binary, args, '-j').splitlines()
    if [(json.loads(x)['fspec'], json.loads(x)['hexdata']) for x in j_lines] != records:
        err('%s: -j fspec/hexdata differ from -jx' % name)
    compared = 0
    for n, (a, b) in enumerate(zip(j_lines, jx_text.splitlines()), 1):
        ja = json.loads(a, object_pairs_hook=lambda p: p)  # keep -j duplicates as pairs
        jb = json.loads(b)
        cat = 'CAT%03d' % jb['cat']
        jcat = dict(ja)[cat]
        compared += compare_j_jx('%s record %d' % (name, n), jcat, jb[cat])

    # human readable outputs
    for fmt in ('-jh', '-je'):
        if headers(run(binary, args, fmt), r'"fspec":"([0-9A-F]*)",', r'"hexdata":"([0-9A-F]*)",') != records:
            err('%s: %s fspec/hexdata differ from -jx' % (name, fmt))
    if headers(run(binary, args, ''), r'Fspec: ([0-9A-F]*)', r'HexData: ([0-9A-F]*)') != records:
        err('%s: text Fspec/HexData differ from -jx' % name)
    xml = re.findall(r' hexdata="([0-9A-F]*)"', run(binary, args, '-x'))
    if xml != [f + h for f, h in records]:
        err('%s: XML hexdata is not fspec+hexdata' % name)
    return len(records), nfields, compared


def main():
    if len(sys.argv) != 2:
        sys.exit(__doc__)
    binary = os.path.abspath(sys.argv[1])
    total = [0, 0, 0]
    for name, args in cases():
        r = check_case(binary, name, args)
        for i, v in enumerate(r):
            total[i] += v
    print('%d records, %d -jx fields checked, %d -j values compared' % tuple(total))
    if errors:
        print('\n'.join(errors[:50]))
        if len(errors) > 50:
            print('... %d more' % (len(errors) - 50))
        sys.exit(1)


if __name__ == '__main__':
    main()
