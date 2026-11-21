/*
 * Utils: format() and the append helpers must write exactly what printf would.
 */
#include <climits>
#include <cstdarg>
#include <cmath>
#include <cstdio>
#include <string>

#include "minitest.h"
#include "Utils.h"

static std::string printfString(const char *fmt, double v) {
    char buf[512];
    snprintf(buf, sizeof(buf), fmt, v);
    return buf;
}

static std::string printfAny(const char *fmt, ...) {
    char buf[512];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buf, sizeof(buf), fmt, args);
    va_end(args);
    return buf;
}

TEST_CASE("format: short result, same as snprintf") {
    CHECK_EQ(format("%d-%s-%08X", 42, "abc", 0xBEEFu), std::string("42-abc-0000BEEF"));
    CHECK_EQ(format("%s", ""), std::string(""));
}

TEST_CASE("format: results longer than the 512-byte stack buffer take the second pass") {
    std::string big(3000, 'x');
    std::string out = format("<%s>", big.c_str());
    CHECK_EQ(out.size(), (size_t) 3002);
    CHECK(out == "<" + big + ">");
    // exactly at the buffer boundary
    for (size_t n = 508; n <= 515; n++) {
        std::string s(n, 'y');
        CHECK(format("%s", s.c_str()) == s);
    }
}

TEST_CASE("appendHexByte / appendHexBytes: %02X") {
    std::string s;
    appendHexByte(s, 0x00);
    appendHexByte(s, 0x0A);
    appendHexByte(s, 0xFF);
    CHECK_EQ(s, std::string("000AFF"));
    unsigned char data[] = {0xDE, 0xAD, 0xBE, 0xEF, 0x01};
    s = "x";
    appendHexBytes(s, data, sizeof(data));
    CHECK_EQ(s, std::string("xDEADBEEF01"));
    appendHexBytes(s, data, 0);
    CHECK_EQ(s, std::string("xDEADBEEF01"));
}

TEST_CASE("appendLong / writeLong: %ld for edge values") {
    long values[] = {0, 1, -1, 9, 10, -10, 123456789, -987654321, LONG_MAX, LONG_MIN, 4294967295L};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        char buf[32];
        snprintf(buf, sizeof(buf), "%ld", values[i]);
        std::string s;
        appendLong(s, values[i]);
        CHECK_EQ(s, std::string(buf));
    }
}

TEST_CASE("appendDouble7: same text as %.7lf") {
    double edge[] = {0.0, -0.0, 1.0, -1.0, 0.5, 1e-8, 5e-8, 4.9999999e-8, 0.00000005, 0.12345675, 0.12345665,
                     123456.78901234, -123456.78901234, 1e15, 1e20, 1e300, -1e300, 2.5e-7, 1.0 / 3.0,
                     360.0 * 65535 / 65536, 0.0078125 * 12345};
    for (size_t i = 0; i < sizeof(edge) / sizeof(edge[0]); i++) {
        std::string s;
        appendDouble7(s, edge[i]);
        CHECK_EQ(s, printfString("%.7lf", edge[i]));
    }
    // the values the decoder actually produces: integer * scale, for typical ASTERIX scales
    double scales[] = {1.0 / 128, 1.0 / 256, 0.25, 360.0 / 65536, 360.0 / 16384, 180.0 / 33554432, 0.1, 0.01,
                       6.25, 1.0 / 1024, 0.2, 2.5};
    unsigned long x = 12345;
    for (int k = 0; k < 20000; k++) {
        x = x * 6364136223846793005UL + 1442695040888963407UL;  // deterministic pseudo-random
        long raw = (long) ((x >> 33) & 0xFFFFFF) - 0x800000;
        double v = raw * scales[k % (sizeof(scales) / sizeof(scales[0]))];
        std::string s;
        appendDouble7(s, v);
        std::string expected = printfString("%.7lf", v);
        if (s != expected) {
            CHECK_EQ(s, expected);
            break;
        }
    }
}

TEST_CASE("appendDoubleFixed, appendHex32, appendInt03, appendIPv4: same text as printf (record header)") {
    double values[] = {0.0, -0.0, 12.5, 28801.429688, 28801.4296875, 46074965.0, 1795228755.539296, -1.0000005,
                       1e-7, 0.0000005, 1e20, -1e300};
    for (size_t i = 0; i < sizeof(values) / sizeof(values[0]); i++) {
        for (int precision = 0; precision <= 7; precision++) {
            std::string s;
            appendDoubleFixed(s, values[i], precision);
            CHECK_EQ(s, printfAny("%.*lf", precision, values[i]));
        }
        std::string s;
        appendDoubleFixed(s, values[i], 6);
        CHECK_EQ(s, printfAny("%lf", values[i]));
    }
    uint32_t hex[] = {0u, 1u, 0xABCDEFu, 0x166B4DBFu, 0xFFFFFFFFu, 0x80000000u};
    for (size_t i = 0; i < sizeof(hex) / sizeof(hex[0]); i++) {
        std::string s;
        appendHex32(s, hex[i]);
        CHECK_EQ(s, printfAny("%08X", hex[i]));
    }
    int cats[] = {0, 1, 21, 48, 255, 999, 1000, -5};
    for (size_t i = 0; i < sizeof(cats) / sizeof(cats[0]); i++) {
        std::string s;
        appendInt03(s, cats[i]);
        CHECK_EQ(s, printfAny("%03d", cats[i]));
    }
    unsigned int ips[] = {0u, 0x0AE7D111u, 0xFFFFFFFFu, 0x7F000001u};
    for (size_t i = 0; i < sizeof(ips) / sizeof(ips[0]); i++) {
        std::string s;
        appendIPv4(s, ips[i]);
        CHECK_EQ(s, printfAny("%u.%u.%u.%u", ips[i] >> 24, (ips[i] >> 16) & 0xFF, (ips[i] >> 8) & 0xFF,
                                 ips[i] & 0xFF));
    }
}

TEST_CASE("crc32: zlib polynomial, as used for the record crc") {
    const char *text = "123456789";
    CHECK_EQ(crc32(text, 9, 0), 0xCBF43926u);  // standard CRC-32 check value
    CHECK(crc32(text, 9, 1) != crc32(text, 9, 0));
    CHECK_EQ(crc32(text, 0, 7), 7u);
}
