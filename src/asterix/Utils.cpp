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
    va_list args;
    int size = 0;

    va_start(args, fmt);
    size = get_format_len(fmt, args);
    va_end(args);

    va_start(args, fmt);
    std::string s = format_arg_list(fmt, size, args);
    va_end(args);

    return s;
}

uint32_t crc32(const void *pBuffer, size_t nLength, uint32_t nPreviousCrc32) {
    uint32_t nCrc = ~nPreviousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
    const uint8_t *pCurrent = (const uint8_t *) pBuffer;

    while (nLength-- > 0) {
        nCrc = (nCrc >> 8) ^ Crc32Lookup[(nCrc & 0xFF) ^ *pCurrent++];
    }

    return ~nCrc; // same as crc ^ 0xFFFFFFFF
}
