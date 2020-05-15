__author__ = 'dsalanti'

import asterix
import struct
import datetime

# Read example file from packet resources
sample_filename = '/home/dsalanti/PycharmProjects/asterix/install/my_samples/REC_RECSDPS_MAIN_ACC_SYS_TK.ff'

final_header_format = ">hBBBBBB"
final_header_size = struct.calcsize(final_header_format)

final_padding_format = "BBBB"
final_padding_size = struct.calcsize(final_padding_format)
final_padding_value = (0xA5, 0xA5, 0xA5, 0xA5)

with open(sample_filename, "rb") as f:
    packetnr = 1
    while True:
        header_bytes = f.read(final_header_size)
        if len(header_bytes) != final_header_size:
            break

        (data_size, board_nr, line_nr, rec_day, time1, time2, time3) = struct.unpack(final_header_format, header_bytes)
        timems = (time1 * 65536 + time2 * 256 + time3) * 10
        packet_size = data_size - final_header_size - final_padding_size

        timestamp = datetime.timedelta(milliseconds=timems)

        print("%d.\ttime=%s\tSize=%d\tboard=%d\tline=%d\tday=%d)" % (
        packetnr, timestamp, packet_size, board_nr, line_nr, rec_day))
        packetnr += 1

        packet_bytes = f.read(packet_size)

        # Parse packet
        packets = asterix.parse(packet_bytes)
        # print(packets)
        for packet in packets:
            print(packet)

        padding_bytes = f.read(4)
        padding = struct.unpack("BBBB", padding_bytes)
        if padding != final_padding_value:
            print("Wrong padding!!")
            exit(1)

print("Finished")
