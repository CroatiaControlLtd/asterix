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

#ifndef DATAITEMBITS_H_
#define DATAITEMBITS_H_

#include <string>
#include <list>

class BitsValue {
public:
    BitsValue(int val) { m_nVal = val; }

    BitsValue(int val, std::string strDescription) {
        m_nVal = val;
        m_strDescription = strDescription;
    }

    int m_nVal;
    std::string m_strDescription;
};

class DataItemBits : public DataItemFormat {
public:
    DataItemBits(int id = 0);

    DataItemBits(const DataItemBits &obj);

    virtual
    ~DataItemBits();

    std::string m_strShortName;
    std::string m_strName;
    int m_nFrom;
    int m_nTo;

    typedef enum {
        DATAITEM_ENCODING_UNSIGNED = 0,
        DATAITEM_ENCODING_SIGNED,
        DATAITEM_ENCODING_SIX_BIT_CHAR,
        DATAITEM_ENCODING_HEX_BIT_CHAR,
        DATAITEM_ENCODING_OCTAL,
        DATAITEM_ENCODING_ASCII
    } _eEncoding;

    _eEncoding m_eEncoding;

    bool m_bIsConst; //!< if true this is fixed value (from m_nConst)
    int m_nConst;

    std::string m_strUnit;
    double m_dScale;
    bool m_bMaxValueSet;
    double m_dMaxValue;
    bool m_bMinValueSet;
    double m_dMinValue;

    bool m_bExtension; //!< item extension (used only in Variable length items)
    int m_nPresenceOfField; //!< presence of compound data item (used only in Compound items)
    std::list<BitsValue *> m_lValue;

    bool m_bFiltered; //! item shall be printed when filter is applied

private:
    std::string m_strJsonMask;            //!< EJSONX ,"mask":"..." text (empty: no mask) for bits below
    int m_nJsonMaskFrom, m_nJsonMaskTo;   //!< m_nFrom, m_nTo m_strJsonMask was built for (0: not built)

public:

    DataItemBits *clone() const { return new DataItemBits(*this); } // Return clone of object
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value to strResult
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength, const std::string *pJsonPrefix); // same, in EJSONX starts with *pJsonPrefix ("key":{), NULL: short name
    std::string getJsonBaseName() const { return m_strShortName.empty() ? m_strName : m_strShortName; }

    /**
     * While an object of this class exists, EJSONX fields add "pos": the byte offset of their hex in
     * the record hexdata. pItemData is the item's data buffer (every format passes pointers inside it)
     * and nItemOffset the item's position in hexdata. NULL disables "pos". Output is single threaded.
     */
    class JsonPosition {
    public:
        JsonPosition(const unsigned char *pItemData, long nItemOffset)
                : m_pPrevData(s_pItemData), m_nPrevOffset(s_nItemOffset) {
            s_pItemData = pItemData;
            s_nItemOffset = nItemOffset;
        }
        ~JsonPosition() {
            s_pItemData = m_pPrevData;
            s_nItemOffset = m_nPrevOffset;
        }
        static const unsigned char *s_pItemData;
        static long s_nItemOffset;
    private:
        const unsigned char *m_pPrevData;
        long m_nPrevOffset;
    };
    std::string printDescriptors(std::string header); // print items format descriptors
    bool filterOutItem(const char *name); // mark item for filtering
    bool isFiltered(const char *) { return m_bFiltered; } // true if item is in filter (shall be printed)
    bool isBits() { return true; }; // true if this is Bits description format
    const char *getDescription(const char *field, const char *value); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif

#if defined(PYTHON_WRAPPER)
    PyObject* getObject(unsigned char* pData, long nLength, int description);
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif

    long getLength(const unsigned char *pData, long nMaxLength);

private:
    unsigned char *getBits(unsigned char *pData, int bytes, int frombit, int tobit);

    unsigned long getUnsigned(unsigned char *pData, int bytes, int frombit, int tobit);

    signed long getSigned(unsigned char *pData, int bytes, int frombit, int tobit);

    unsigned char *getSixBitString(unsigned char *pData, int bytes, int frombit, int tobit);

    unsigned char *getHexBitString(unsigned char *pData, int bytes, int frombit, int tobit);

    unsigned char *getOctal(unsigned char *pData, int bytes, int frombit, int tobit);

    unsigned char *getHexBitStringFullByte(unsigned char *pData, int bytes, int frombit, int tobit);

    unsigned char *getHexBitStringMask(int bytes, int frombit, int tobit);

    void appendJsonHexMask(std::string &strResult, unsigned char *pData, long nLength); // ,"hex":"..."[,"mask":"..."]

    char *getASCII(unsigned char *pData, int bytes, int frombit, int tobit);


};

#endif /* DATAITEMBITS_H_ */
