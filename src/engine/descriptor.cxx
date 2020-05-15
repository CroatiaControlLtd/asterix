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
 * AUTHORS: Jurica Baricevic, Croatia Control Ltd.
 *
 */
#include <string.h>

#include "descriptor.hxx"


CDescriptor::CDescriptor(const char *description, const char *delimiters, bool multiMode) {
    unsigned int size = 0;

    // 1. Allocate the memory for the description string and initialize it
    if (description) {
        size = strlen(description);
    }

    if (size != 0) {
        size++; // Null terminator
        _description = new char[size];
        strcpy(_description, description);
    } else {
        // Make an empty string to avoid special handling with NULL pointers.
        _description = new char[1];
        _description[0] = '\0';
        size = 1;
    }


    // 2. Initialize the rest of members    
    _iterator = _description;
    _end = _description + size;
    *(_end - 1) = '\0'; // safety precaution
    _bMultipleDelimiters = multiMode;


    // 3. Delimit parameters with '\0' to get a list of strings
    if (delimiters) {
        do {
            if (strchr(delimiters, *_iterator)) {
                *_iterator = '\0';
            }
        } while (++_iterator < _end);

        _iterator = _description; // reset iterator
    }
}


CDescriptor::~CDescriptor() {
    if (_description) {
        delete[] _description;
    }
}


const char *CDescriptor::GetFirst(const char *empty_chars) {
    // Reset iterator first
    _iterator = _description;


    // Advance the iterator over all NULL characters just before the next item
    // in case multiple delimiters are allowed
    if (_bMultipleDelimiters) {
        while ((_iterator < _end) && (*_iterator == '\0')) {
            _iterator++;
        }
    }

    // Check for reaching the end
    if (_iterator >= _end) {
        _iterator--; // Get back within boundary for the next call to GetNext()
        return NULL;
    } else {
        // Removing empty chars allowed only in multiple delimiter mode
        if (_bMultipleDelimiters)
            RemoveEmptyChars(empty_chars);
        return _iterator;
    }
}


const char *CDescriptor::GetNext(const char *empty_chars) {
    // Advance the iterator till the first NULL character
    while ((_iterator < _end) && (*_iterator != '\0')) {
        _iterator++;
    }

    // Advance the iterator over all NULL characters just before the next item
    // in multiple delimiters mode
    if (_bMultipleDelimiters) {
        while ((_iterator < _end) && (*_iterator == '\0')) {
            _iterator++;
        }
    } else {
        // Advance to a start of new parameter
        _iterator++;
    }


    // Check for reaching the end
    if (_iterator >= _end) {
        _iterator--; // Get back within boundary for the next call to GetNext()
        return NULL;
    } else {
        // Removing empty chars allowed only in multiple delimiter mode
        if (_bMultipleDelimiters)
            RemoveEmptyChars(empty_chars);
        return _iterator;
    }
}


void CDescriptor::RemoveEmptyChars(const char *empty_chars) {
    if (empty_chars) {
        // Ignore leading empty chars
        while (strchr(empty_chars, *_iterator) && (*_iterator != '\0')) {
            _iterator++;
        }

        char *word_start = _iterator;

        // Move to the end
        while (*_iterator++ != '\0');

        // Remove trailing empty chars (up to the first null char)
        while (strchr(empty_chars, *--_iterator)) {
            *_iterator = '\0';
        }

        // Reset iterator
        _iterator = word_start;
    }
}
