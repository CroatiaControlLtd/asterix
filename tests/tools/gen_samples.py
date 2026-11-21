#!/usr/bin/env python3
"""
Generate the synthetic test inputs in tests/data from the real samples in install/sample_data.

The ASTERIX data blocks are taken from the real captures and only re-wrapped, so every new
input decodes to records that also exist in a real sample:
  cat_001_002.raw      CAT001/CAT002 blocks (from the ORADIS pcap) as a plain raw stream
  cat_001_002.oradis   the same, ORADIS framed (-O): [byte count 2][time 4][ASTERIX...]
  cat_001_002.final    the same, FINAL framed (-F): [byte count 2][board][line][day][time 3][ASTERIX...][4 pad]
  cat_001_002.hdlc     the same, HDLC framed (-H): [0x01][0x03][ASTERIX...][FCS-16, LSB first]
  multicat.raw         blocks of every category in the samples (001 002 021 034 048 062 065), interleaved
  bad/*                malformed inputs: each one must end quickly without a crash or sanitizer report

Run from anywhere: python3 tests/tools/gen_samples.py
The output is deterministic; the files are committed, rerun this only to change them.
"""
import os
import struct

ROOT = os.path.abspath(os.path.join(os.path.dirname(__file__), '..', '..'))
SAMPLES = os.path.join(ROOT, 'install', 'sample_data')
OUT = os.path.join(ROOT, 'tests', 'data')


def pcap_packets(path):
    """(timestamp in seconds, UDP payload) of every Ethernet/IPv4/UDP packet in a pcap file"""
    d = open(path, 'rb').read()
    magic = struct.unpack('<I', d[:4])[0]
    assert magic == 0xa1b2c3d4, 'only little-endian microsecond pcap files'
    p = 24
    while p + 16 <= len(d):
        sec, usec, incl, _ = struct.unpack('<IIII', d[p:p + 16])
        pkt = d[p + 16:p + 16 + incl]
        p += 16 + incl
        if pkt[12:14] != b'\x08\x00':
            continue
        ihl = (pkt[14] & 0x0f) * 4
        udp = 14 + ihl
        udp_len = (pkt[udp + 4] << 8) | pkt[udp + 5]  # short frames carry Ethernet padding after the UDP data
        yield sec + usec / 1e6, pkt[udp + 8:udp + udp_len]


def split_blocks(data):
    """ASTERIX data blocks ([CAT][LEN 2][...]) of a raw stream"""
    blocks, p = [], 0
    while p + 3 <= len(data):
        n = (data[p + 1] << 8) | data[p + 2]
        assert n >= 3 and p + n <= len(data), 'malformed raw stream'
        blocks.append(data[p:p + n])
        p += n
    return blocks


def oradis_blocks(payload):
    """ASTERIX blocks inside ORADIS chunks: [byte count 2][time 4][ASTERIX...]"""
    blocks, p = [], 0
    while p + 6 <= len(payload):
        n = (payload[p] << 8) | payload[p + 1]
        assert n > 6 and p + n <= len(payload), 'malformed ORADIS chunk'
        blocks += split_blocks(payload[p + 6:p + n])
        p += n
    return blocks


FCSTAB = []
for b in range(256):
    v = b
    for _ in range(8):
        v = (v >> 1) ^ 0x8408 if v & 1 else v >> 1
    FCSTAB.append(v)


def hdlc_frame(body):
    """HDLC frame as read by asterixhdlcparsing.c: address 0x01, control 0x03, body, FCS-16 (X.25)"""
    frame = b'\x01\x03' + body
    fcs = 0xffff
    for c in frame:
        fcs = (fcs >> 8) ^ FCSTAB[(fcs ^ c) & 0xff]
    fcs ^= 0xffff
    return frame + bytes([fcs & 0xff, fcs >> 8])


def final_record(body, seconds):
    t = int(round((seconds % 86400) * 100)) & 0xffffff  # FINAL time unit is 10 ms
    head = struct.pack('>HBBB', 8 + len(body) + 4, 0, 0, 0) + bytes([t >> 16, (t >> 8) & 0xff, t & 0xff])
    return head + body + b'\x00\x00\x00\x00'


def oradis_chunk(body, seconds):
    return struct.pack('>HI', 6 + len(body), int(seconds * 1000) & 0xffffffff) + body


def write(name, data):
    path = os.path.join(OUT, name)
    os.makedirs(os.path.dirname(path), exist_ok=True)
    with open(path, 'wb') as f:
        f.write(data)
    print('%-28s %6d bytes' % (name, len(data)))


def main():
    # CAT001/002: blocks from the ORADIS pcap, with the capture time
    timed = []
    for ts, payload in pcap_packets(os.path.join(SAMPLES, 'cat_001_002.pcap')):
        timed += [(ts, b) for b in oradis_blocks(payload)]
    # HDLC frames start with [01][03][CAT<=7][LEN MSB 0]: every block must be shorter than 256 bytes
    assert all(b[0] <= 7 and len(b) < 256 for _, b in timed)

    write('cat_001_002.raw', b''.join(b for _, b in timed))
    write('cat_001_002.oradis', b''.join(oradis_chunk(b, ts + i * 0.01) for i, (ts, b) in enumerate(timed)))
    write('cat_001_002.final', b''.join(final_record(b, ts + i * 0.01) for i, (ts, b) in enumerate(timed)))
    write('cat_001_002.hdlc', b''.join(hdlc_frame(b) for _, b in timed))

    # every category of the samples in one raw stream, interleaved
    groups = [
        [b for _, b in timed],
        split_blocks(open(os.path.join(SAMPLES, 'cat21_re.ast'), 'rb').read()),
        [b for _, p in pcap_packets(os.path.join(SAMPLES, 'cat_034_048.pcap')) for b in split_blocks(p)][:20],
        split_blocks(open(os.path.join(SAMPLES, 'cat062cat065.raw'), 'rb').read()),
        [b for _, p in pcap_packets(os.path.join(SAMPLES, 'asterix.pcap')) for b in split_blocks(p)][:10],
    ]
    mixed, i = [], 0
    while any(groups):
        g = groups[i % len(groups)]
        if g:
            mixed.append(g.pop(0))
        i += 1
    write('multicat.raw', b''.join(mixed))

    # malformed inputs (regressions of the overflow fixes): they must end without crash, hang or sanitizer report
    good = b''.join(b for _, b in timed)
    first = timed[0][1]
    write('bad/raw_len_too_big.raw', first[:1] + b'\xff\xff' + first[3:])            # block length past the end
    write('bad/raw_len_too_small.raw', first[:1] + b'\x00\x02' + first[3:])           # block length < 3
    write('bad/raw_truncated_record.raw', first[:3] + first[3:len(first) // 2])       # record cut in an item
    write('bad/raw_truncated_stream.raw', good[:len(good) - 7])                       # stream cut in a block
    rec = bytes.fromhex('F7C619C9A00EB2767F189408AA42D8033405C8800D400E')  # one CAT001 record of first
    body = rec + rec[:10] + rec                                                  # a cut record between two good ones
    write('bad/raw_broken_record_in_block.raw', bytes([1]) + (3 + len(body)).to_bytes(2, 'big') + body)
    write('bad/raw_fspec_only_fx.raw', first[:1] + b'\x00\x08' + b'\xff' * 5)          # FSPEC that never ends
    write('bad/oradis_count_zero.oradis', b'\x00\x00\x00\x00\x00\x00' + first)        # used to loop forever
    write('bad/oradis_count_too_big.oradis', b'\xff\xff\x00\x00\x00\x00' + first)
    write('bad/final_count_too_small.final', b'\x00\x05' + b'\x00' * 6 + first)       # byte count < header
    write('bad/hdlc_bad_fcs.hdlc', hdlc_frame(first)[:-1] + b'\x00')                  # frame that never closes
    write('bad/hdlc_block_len_zero.hdlc', hdlc_frame(b'\x01\x00\x00' + first[3:9]))  # used to loop forever
    pcap = open(os.path.join(SAMPLES, 'cat_034_048.pcap'), 'rb').read()
    write('bad/pcap_truncated_header.pcap', pcap[:24 + 10])                           # packet header cut
    write('bad/pcap_truncated_packet.pcap', pcap[:24 + 16 + 30])                      # packet cut in IP/UDP
    write('bad/pcap_huge_incl_len.pcap', pcap[:24 + 8] + b'\xff\xff\xff\xff' + pcap[24 + 12:])  # used to allocate 4 GB
    gps = open(os.path.join(SAMPLES, 'parsegps.gps'), 'rb').read()
    write('bad/gps_truncated.gps', gps[:2200 + 40])                                   # GPS record cut


if __name__ == '__main__':
    main()
