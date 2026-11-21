/*
 * DataItemBits: bit extraction, encodings and the -jx hex/mask/pos text, through getText()
 * (the getters are private: getText is what every output uses).
 * EJSON writes "name":value, (with a trailing comma, removed by the enclosing item).
 * Bit numbering: bit 1 is the LSB of the last byte, bit 8*n the MSB of the first byte.
 */
#include <memory>
#include <string>

#include "minitest.h"
#include "test_support.h"

typedef std::unique_ptr<DataItemBits> BitsPtr;

TEST_CASE("bits: unsigned values at byte boundaries and across them") {
    parser();  // Tracer capture
    CHECK_EQ(render(BitsPtr(makeBits("A", 1, 8)).get(), "A5", CAsterixFormat::EJSON), std::string("\"A\":165,"));
    CHECK_EQ(render(BitsPtr(makeBits("A", 5, 8)).get(), "A5", CAsterixFormat::EJSON), std::string("\"A\":10,"));
    CHECK_EQ(render(BitsPtr(makeBits("A", 1, 4)).get(), "A5", CAsterixFormat::EJSON), std::string("\"A\":5,"));
    // bits 5..12 of 0x1234: 0x23
    CHECK_EQ(render(BitsPtr(makeBits("A", 5, 12)).get(), "1234", CAsterixFormat::EJSON), std::string("\"A\":35,"));
    // 32-bit field, the widest allowed
    CHECK_EQ(render(BitsPtr(makeBits("A", 1, 32)).get(), "FFFFFFFF", CAsterixFormat::EJSON),
             std::string("\"A\":4294967295,"));
    // 32 bits not aligned: spread over 5 bytes
    CHECK_EQ(render(BitsPtr(makeBits("A", 5, 36)).get(), "0FFFFFFFF0", CAsterixFormat::EJSON),
             std::string("\"A\":4294967295,"));
    // a single bit
    CHECK_EQ(render(BitsPtr(makeBits("A", 16, 16)).get(), "8000", CAsterixFormat::EJSON), std::string("\"A\":1,"));
}

TEST_CASE("bits: signed two's complement and scaled values") {
    BitsPtr s(makeBits("S", 1, 8, DataItemBits::DATAITEM_ENCODING_SIGNED));
    CHECK_EQ(render(s.get(), "FF", CAsterixFormat::EJSON), std::string("\"S\":-1,"));
    CHECK_EQ(render(s.get(), "80", CAsterixFormat::EJSON), std::string("\"S\":-128,"));
    CHECK_EQ(render(s.get(), "7F", CAsterixFormat::EJSON), std::string("\"S\":127,"));
    BitsPtr s12(makeBits("S", 1, 12, DataItemBits::DATAITEM_ENCODING_SIGNED));
    CHECK_EQ(render(s12.get(), "F800", CAsterixFormat::EJSON), std::string("\"S\":-2048,"));
    BitsPtr scaled(makeBits("R", 1, 16, DataItemBits::DATAITEM_ENCODING_UNSIGNED, 1.0 / 256));
    CHECK_EQ(render(scaled.get(), "0180", CAsterixFormat::EJSON), std::string("\"R\":1.5000000,"));
    BitsPtr sneg(makeBits("R", 1, 16, DataItemBits::DATAITEM_ENCODING_SIGNED, 0.25));
    CHECK_EQ(render(sneg.get(), "FFFE", CAsterixFormat::EJSON), std::string("\"R\":-0.5000000,"));
}

TEST_CASE("bits: six-bit, hex, octal and ASCII encodings") {
    // "AB" in six-bit code: A=1, B=2 -> 000001 000010
    BitsPtr six(makeBits("ID", 1, 12, DataItemBits::DATAITEM_ENCODING_SIX_BIT_CHAR));
    CHECK_EQ(render(six.get(), "0042", CAsterixFormat::EJSON), std::string("\"ID\":\"AB\","));
    BitsPtr hex(makeBits("H", 1, 24, DataItemBits::DATAITEM_ENCODING_HEX_BIT_CHAR));
    CHECK_EQ(render(hex.get(), "3C660C", CAsterixFormat::EJSON), std::string("\"H\":\"3C660C\","));
    // Mode 3/A 7700 in octal: 111 111 000 000
    BitsPtr oct(makeBits("M3A", 1, 12, DataItemBits::DATAITEM_ENCODING_OCTAL));
    CHECK_EQ(render(oct.get(), "0FC0", CAsterixFormat::EJSON), std::string("\"M3A\":\"7700\","));
    // only A-Z, 0-9, space and '.' are kept, anything else becomes '@' (no output format has to escape it)
    BitsPtr ascii(makeBits("T", 1, 32, DataItemBits::DATAITEM_ENCODING_ASCII));
    CHECK_EQ(render(ascii.get(), "41014243", CAsterixFormat::EJSON), std::string("\"T\":\"A@BC\","));
    BitsPtr cs(makeBits("CS", 1, 56, DataItemBits::DATAITEM_ENCODING_ASCII));
    // quote, backslash, '<', '&', lower case and control characters
    CHECK_EQ(render(cs.get(), "225C3C26610009", CAsterixFormat::EJSON), std::string("\"CS\":\"@@@@@@@\","));
    CHECK_EQ(render(cs.get(), "45474744204720", CAsterixFormat::EJSONX).find("\"val\":\"EGGD G \""), (size_t) 6);
    // radio channel names (I205/090) keep their decimal point
    CHECK_EQ(render(cs.get(), "3132312E313030", CAsterixFormat::EJSON), std::string("\"CS\":\"121.100\","));
}

TEST_CASE("bits -jx: hex is the bytes holding the field, mask only when it does not fill them, pos") {
    BitsPtr full(makeBits("SAC", 9, 16));
    CHECK_EQ(render(full.get(), "19C9", CAsterixFormat::EJSONX),
             std::string("\"SAC\":{\"val\":25,\"hex\":\"19\",\"pos\":0},"));
    BitsPtr low(makeBits("SIC", 1, 8));
    CHECK_EQ(render(low.get(), "19C9", CAsterixFormat::EJSONX),
             std::string("\"SIC\":{\"val\":201,\"hex\":\"C9\",\"pos\":1},"));
    BitsPtr part(makeBits("TYP", 8, 8));
    CHECK_EQ(render(part.get(), "A0", CAsterixFormat::EJSONX),
             std::string("\"TYP\":{\"val\":1,\"hex\":\"A0\",\"mask\":\"10000000\",\"pos\":0},"));
    // 8 bits that do not start on a byte boundary: two bytes and a mask (BDS res1 case)
    BitsPtr shifted(makeBits("X", 5, 12));
    CHECK_EQ(render(shifted.get(), "1234", CAsterixFormat::EJSONX),
             std::string("\"X\":{\"val\":35,\"hex\":\"1234\",\"mask\":\"0000111111110000\",\"pos\":0},"));
    // without a JsonPosition (item data unknown) there is no pos
    std::vector<unsigned char> d = fromHex("19C9");
    std::string out, header;
    full->getText(out, header, CAsterixFormat::EJSONX, &d[0], 2);
    CHECK_EQ(out, std::string("\"SAC\":{\"val\":25,\"hex\":\"19\"},"));
}

TEST_CASE("bits: a field outside the data prints nothing") {
    tracerMessages().clear();
    BitsPtr b(makeBits("A", 1, 24));
    CHECK_EQ(render(b.get(), "1234", CAsterixFormat::EJSON), std::string(""));
    CHECK(!tracerMessages().empty());
}

TEST_CASE("bits: from/to given in reverse order are swapped") {
    BitsPtr b(makeBits("A", 8, 5));
    CHECK_EQ(render(b.get(), "A5", CAsterixFormat::EJSON), std::string("\"A\":10,"));
}

TEST_CASE("bits: more than 32 bits in an unsigned field is reported, not read") {
    tracerMessages().clear();
    BitsPtr b(makeBits("A", 1, 40));
    std::string out = render(b.get(), "FFFFFFFFFF", CAsterixFormat::EJSON);
    CHECK_EQ(out, std::string("\"A\":0,"));
    CHECK(!tracerMessages().empty());
}
