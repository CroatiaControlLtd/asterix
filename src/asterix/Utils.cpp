/*
 *  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
 *
 *  This file is part of Asterix.
 *
 *  Asterix is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation, either version 3 of the License, or
 *  (at your option) any later version.
 *
 *  Asterix is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

#include "Utils.h"

#if __cplusplus >= 201703L && defined(__has_include)
#if __has_include(<charconv>)
#include <charconv>
#endif
#endif
#if defined(__cpp_lib_to_chars) && __cpp_lib_to_chars >= 201611L
#define ASTERIX_HAVE_FLOAT_TO_CHARS 1
#else
#define ASTERIX_HAVE_FLOAT_TO_CHARS 0
#endif

std::string format_arg_list(const char *fmt, int size, va_list args) {
    char *buffer = new char[++size];;
    size = vsnprintf(buffer, size, fmt, args);
    if (size < 0) {
        delete[] buffer;
        return std::string();
    }
    std::string s(buffer);
    delete[] buffer;
    return s;
}

int get_format_len(const char *fmt, va_list args) {
    int size = 0;
    char* buffer = NULL;
    size = vsnprintf(buffer, size, fmt, args);
    if (size < 0) {
        // for glibc < 2.0.6 vsnprintf returns -1 when the output was truncated
        // so we just put some maximal size and if the string is longer it will be discarded in format_arg_list
        size = 4096;
    }
    return size;
}

std::string format(const char *fmt, ...) {
    // Format once into a stack buffer; only results that do not fit need a second pass.
    char buffer[512];
    va_list args;

    va_start(args, fmt);
    int size = vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);

    if (size < 0) {
        return std::string();
    }
    if ((size_t) size < sizeof(buffer)) {
        return std::string(buffer);
    }

    va_start(args, fmt);
    std::string s = format_arg_list(fmt, size, args);
    va_end(args);

    return s;
}

void appendDoubleFixed(std::string &s, double value, int precision) {
#if ASTERIX_HAVE_FLOAT_TO_CHARS
    // std::to_chars with a precision is specified to give the same result as printf("%.*f")
    char buffer[400];
    std::to_chars_result r = std::to_chars(buffer, buffer + sizeof(buffer), value, std::chars_format::fixed,
                                           precision);
    if (r.ec == std::errc()) {
        s.append(buffer, r.ptr - buffer);
        return;
    }
#endif
    char small[64];
    int size = snprintf(small, sizeof(small), "%.*f", precision, value);
    if (size >= 0 && (size_t) size < sizeof(small))
        s.append(small, size);
    else
        s += format("%.*f", precision, value);
}

void appendDouble7(std::string &s, double value) {
    appendDoubleFixed(s, value, 7);
}

uint32_t crc32(const void *pBuffer, size_t nLength, uint32_t nPreviousCrc32) {
    uint32_t nCrc = ~nPreviousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
    const uint8_t *pCurrent = (const uint8_t *) pBuffer;

    while (nLength-- > 0) {
        nCrc = (nCrc >> 8) ^ Crc32Lookup[(nCrc & 0xFF) ^ *pCurrent++];
    }

    return ~nCrc; // same as crc ^ 0xFFFFFFFF
}
