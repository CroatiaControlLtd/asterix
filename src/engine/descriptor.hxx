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
#ifndef DESCRIPTOR_HXX__
#define DESCRIPTOR_HXX__

/**
 * @class CDescriptor
 * 
 * @brief The descriptor class for simplified parameters handling from string.
 *
 */
class CDescriptor {
private:
    char *_description;
    char *_iterator;
    char *_end;
    int _bMultipleDelimiters;

    void RemoveEmptyChars(const char *empty_chars);

public:

    /**
     * Default class constructors
     * @param <multiMode> When <true> multiple consecutive delimiters are considered as 
     *        one. When <false> multiple consecutive delimiters are considered 
     *        each separately (single mode - default).
     */
    CDescriptor(const char *description, const char *delimiters, bool multiMode = false);

    /**
     * Class destructor.
     */
    ~CDescriptor();

    const char *GetFirst(const char *empty_chars = NULL);

    const char *GetNext(const char *empty_chars = NULL);
};

#endif
