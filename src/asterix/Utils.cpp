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
    int size = 0;
    va_list args_t;
    char *buffer = NULL;
    *args_t = *args;
    size = vsnprintf(buffer, size, fmt, args_t);

    if ( size < 0 )
	return std::string();

    size++; // for '\0'
    buffer = (char *) calloc(1, size);
    if ( buffer == NULL )
	return std::string();

    *args_t = *args;
    size = vsnprintf(buffer, size, fmt, args_t);

    if ( size < 0 ) {
	free(buffer);
	return std::string();
    }

    std::string s(buffer);
    free(buffer);
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

uint32_t crc32(const void* pBuffer, size_t nLength, uint32_t nPreviousCrc32)
{
    uint32_t nCrc = ~nPreviousCrc32; // same as previousCrc32 ^ 0xFFFFFFFF
    const uint8_t* pCurrent = (const uint8_t*) pBuffer;

    while (nLength-- > 0) {
	nCrc = (nCrc >> 8) ^ Crc32Lookup[(nCrc & 0xFF) ^ *pCurrent++];
    }

    return ~nCrc; // same as crc ^ 0xFFFFFFFF
}
