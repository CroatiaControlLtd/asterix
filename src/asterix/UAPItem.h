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

#ifndef UAPITEM_H_
#define UAPITEM_H_

#include "DataItemFormat.h"
#include "Tracer.h"


class UAPItem : public DataItemFormat {
public:
    UAPItem();

    UAPItem(const UAPItem &obj);

    virtual
    ~UAPItem();

    int m_nBit; // <!ATTLIST UAPItem bit CDATA #REQUIRED>
    int m_nFRN; // <!ATTLIST UAPItem frn CDATA #REQUIRED>
    bool m_bFX; // <!ATTLIST UAPItem fx CDATA "0">
    int m_nLen; // <!ATTLIST UAPItem len CDATA "-">
    std::string m_strItemID; // <!ELEMENT UAPItem (#PCDATA)>

    UAPItem *clone() const { return new UAPItem(*this); } // Return clone of object
    long getLength(const unsigned char *) {
        Tracer::Error("Function should not be called!");
        return 0;
    }

    bool getText(std::string &, std::string &, const unsigned int, unsigned char *, long) {
        Tracer::Error("Function should not be called!");
        return false;
    } // appends description to strDescription
    std::string printDescriptors(std::string) {
        Tracer::Error("Function should not be called!");
        return "";
    }; // print items format descriptors
    bool filterOutItem(const char *) {
        Tracer::Error("Function should not be called!");
        return false;
    }; // mark item for filtering
    bool isFiltered(const char *) {
        Tracer::Error("Function should not be called!");
        return false;
    }; // mark item for filtering
    const char *getDescription(const char *field, const char *value) {
        Tracer::Error("Function should not be called!");
        return NULL;
    }; // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    PyObject* getObject(unsigned char* pData, long nLength, int description) { return NULL; };
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description) {};
#endif
};

#endif /* UAPITEM_H_ */
