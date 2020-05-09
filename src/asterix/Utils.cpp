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

std::string format_arg_list(const char *fmt, va_list args)
{
    if (!fmt) return "";
    int   result = -1, length = 256;
    va_list args_t;
    // copy args so we can reuse it if vsnprintf overflows
    *args_t = *args;
    char *buffer = 0;
    while (result == -1)
    {
        if (buffer) delete [] buffer;
        buffer = new char [length + 1];
        memset(buffer, 0, length + 1);
        result = vsnprintf(buffer, length, fmt, args_t);
	// result wil be -1 if error, but if there is not enough space in
	// buffer, result will contain the amount of bytes needed
	if ( result == -1 ) {
	    // we don't know exact size, so guess
	    length *= 2;
	    result = -1;
	} else if ( result > length ) {
	    length = result + 1;
	    result = -1;
	}
	// args_t is undefined after vsnprintf, if we are going to loop
	// we will need pointer restored
        *args_t = *args;
    }
    std::string s(buffer);
    delete [] buffer;
    return s;
}

std::string format(const char *fmt, ...)
{
    va_list args;
    va_start(args, fmt);
    std::string s = format_arg_list(fmt, args);
    va_end(args);
    return s;
}

uint32_t crc32(const void* pBuffer, size_t nLength, uint32_t nPreviousCrc32 = 0)
{
    uint32_t nCrc = ~nPreviousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
    const uint8_t* pCurrent = (const uint8_t*) pBuffer;

    while (nLength-- > 0) {
	nCrc = (nCrc >> 8) ^ Crc32Lookup[(nCrc & 0xFF) ^ *pCurrent++];
    }

    return ~nCrc; // same as crc ^ 0xFFFFFFFF
}
