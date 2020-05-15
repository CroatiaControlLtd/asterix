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

#ifndef DATAITEMFORMAT_H_
#define DATAITEMFORMAT_H_

#if defined(PYTHON_WRAPPER)
#include <Python.h>
#endif

#include <string>
#include <list>
#include "Utils.h"

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
#include "WiresharkWrapper.h"
#endif

class DataItemBits;

class DataItemFormat {
public:
    DataItemFormat(int id = 0);

    virtual
    ~DataItemFormat();

    std::list<DataItemFormat *> m_lSubItems; //!< List of subitem formats in this item

    DataItemFormat *m_pParentFormat; //! Pointer to parent format (used only in XML parsing)
    int m_nID; //! ID of the item

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    static int m_nLastPID; //!< static used for allocation of m_nPID
#endif

    virtual DataItemFormat *clone() const = 0; // Copy constructor
    virtual long getLength(const unsigned char *pData) = 0;

    virtual bool
    getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
            long nLength) = 0; // appends value to strResult
    virtual std::string printDescriptors(std::string header) = 0; // print items format descriptors
    virtual bool filterOutItem(const char *name) = 0; // mark item for filtering
    virtual bool isFiltered(const char *name) = 0; // is item filtered
    virtual const char *getDescription(const char *field, const char *value) = 0; // return descrtption ef element

    virtual bool isFixed() { return false; }; // true if this is Fixed format
    virtual bool isRepetitive() { return false; }; // true if this is Repetitive format
    virtual bool isBDS() { return false; }; // true if this is BDS format
    virtual bool isVariable() { return false; }; // true if this is Variable format
    virtual bool isExplicit() { return false; }; // true if this is Explicit format
    virtual bool isCompound() { return false; }; // true if this is Compound format
    virtual bool isBits() { return false; }; // true if this is Bits description format


#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    virtual fulliautomatix_definitions* getWiresharkDefinitions() = 0;
    virtual fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset) = 0;
    int getPID() { return m_nPID; }

  private:
    int m_nPID; //!< Unique identifier of this format description
#endif

#if defined(PYTHON_WRAPPER)
    virtual PyObject* getObject(unsigned char* pData, long nLength, int description) = 0;
    virtual void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description) = 0;
#endif

};

#endif /* DATAITEMFORMAT_H_ */
