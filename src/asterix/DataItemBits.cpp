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

#include "DataItemFormat.h"
#include "DataItemBits.h"
#include "Tracer.h"
#include "asterixformat.hxx"

extern bool gFiltering;

static const char SIXBITCODE[] = {' ', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O',
                                  'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', ' ', ' ', ' ', ' ', ' ',
                                  ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ', ' ',
                                  '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', ' ', ' ', ' ', ' ', ' ', ' '};

DataItemBits::DataItemBits(int id)
        : DataItemFormat(id), m_nFrom(0), m_nTo(0), m_eEncoding(DATAITEM_ENCODING_UNSIGNED), m_bIsConst(false),
          m_nConst(0), m_dScale(0.0), m_bMaxValueSet(false), m_dMaxValue(0.0), m_bMinValueSet(false), m_dMinValue(0.0),
          m_bExtension(false), m_nPresenceOfField(0), m_bFiltered(false) {

}

DataItemBits::DataItemBits(const DataItemBits &obj)
        : DataItemFormat(obj.m_nID) {
    std::list<DataItemFormat *>::iterator it = ((DataItemFormat &) obj).m_lSubItems.begin();
    while (it != obj.m_lSubItems.end()) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        m_lSubItems.push_back(di->clone());
        it++;
    }

    m_pParentFormat = obj.m_pParentFormat;

    m_strShortName = obj.m_strShortName;
    m_strName = obj.m_strName;
    m_nFrom = obj.m_nFrom;
    m_nTo = obj.m_nTo;
    m_eEncoding = obj.m_eEncoding;
    m_bIsConst = obj.m_bIsConst;
    m_nConst = obj.m_nConst;
    m_strUnit = obj.m_strUnit;
    m_dScale = obj.m_dScale;
    m_bMaxValueSet = obj.m_bMaxValueSet;
    m_dMaxValue = obj.m_dMaxValue;
    m_bMinValueSet = obj.m_bMinValueSet;
    m_dMinValue = obj.m_dMinValue;
    m_bExtension = obj.m_bExtension;
    m_nPresenceOfField = obj.m_nPresenceOfField;

    std::list<BitsValue *>::const_iterator bit = obj.m_lValue.begin();
    while (bit != obj.m_lValue.end()) {
        BitsValue *bv = (BitsValue *) (*bit);
        m_lValue.push_back(new BitsValue(bv->m_nVal, bv->m_strDescription));
        bit++;
    }
    m_bFiltered = obj.m_bFiltered;
}


DataItemBits::~DataItemBits() {
    // destroy bits value items
    std::list<BitsValue *>::iterator it = m_lValue.begin();
    while (it != m_lValue.end()) {
        delete (BitsValue *) (*it);
        it = m_lValue.erase(it);
    }
}

long DataItemBits::getLength(const unsigned char *) {
    Tracer::Error("DataItemBits::getLength Should not be called!");
    return 0;
}

unsigned char *DataItemBits::getBits(unsigned char *pData, int bytes, int frombit, int tobit) {
    /* example:
     * bytenr     2                               1
     * bitnr    [16][15][14][13][12][11][10][ 9][ 8][ 7][ 6][ 5][ 4][ 3][ 2][ 1]
     * value      1   1   0   0   1   0   0   1   1   1   1   0   1   0   0   1
     * getBits(pData, 2, 7, 10)
     *                                    |to         |from
     * returns                            0   1   1   1   0   0   0   0
     *
     */
    int numberOfBits = (tobit - frombit + 1);
    int numberOfBytes = (numberOfBits + 7) / 8;

    if (frombit > tobit || tobit < 1 || frombit < 1 || numberOfBytes > bytes) {
        Tracer::Error("Irregular request for getBits");
        return 0;
    }

    unsigned char *pVal = new unsigned char[numberOfBytes];
    unsigned char *pTmp = pVal;
    memset(pVal, 0, numberOfBytes);

    if (frombit == 1 && tobit == bytes * 8) {
        memcpy(pVal, pData, bytes);
        return pVal;
    }

    unsigned char bitmask = 0x80;
    unsigned char outbits = 0;
    for (int bit = bytes * 8; bit >= frombit; bit--) {
        if (bit <= tobit) {
            unsigned char bitval = *pData & bitmask;
            *pTmp <<= 1;
            if (bitval)
                *pTmp |= 0x01;

            if (++outbits >= 8) {
                outbits = 0;
                pTmp++;
            }
        }

        bitmask >>= 1;
        bitmask &= 0x7F;

        if (bitmask == 0) {
            bitmask = 0x80;
            pData++;
        }
    }

    if (pVal + numberOfBytes - 1 >= pTmp)
        *pTmp <<= 8 - outbits;

    return pVal;
}

unsigned long DataItemBits::getUnsigned(unsigned char *pData, int bytes, int frombit, int tobit) {
    unsigned long val = 0;
    int numberOfBits = (tobit - frombit + 1);

    if (numberOfBits < 1 || numberOfBits > 32) {
        Tracer::Error(
                "DataItemBits::getUnsigned : Wrong parameter.m Number of bits = %d, and must be between 1 and 32. Currently is from %d to %d",
                numberOfBits, tobit, frombit);
    } else {
        unsigned char *pTmp = getBits(pData, bytes, frombit, tobit);

        if (!pTmp) {
            Tracer::Error("DataItemBits::getUnsigned : Error.");
            return 0;
        }

        if (numberOfBits == 8) {
            val = *((unsigned char *) pTmp);
        } else {
            unsigned char *pTmp2 = pTmp;
            unsigned char bitmask = 0x80;
            while (numberOfBits--) {
                unsigned char bitval = *pTmp2 & bitmask;
                val <<= 1;
                if (bitval)
                    val |= 0x01;

                bitmask >>= 1;
                bitmask &= 0x7F;
                if (bitmask == 0) {
                    bitmask = 0x80;
                    pTmp2++;
                }
            }
        }
        delete[] pTmp;
    }
    return val;
}

signed long DataItemBits::getSigned(unsigned char *pData, int bytes, int frombit, int tobit) {
    unsigned long ul = getUnsigned(pData, bytes, frombit, tobit);
    int numberOfBits = (tobit - frombit + 1);
    unsigned long maxval = 0x01;
    maxval <<= (numberOfBits - 1);
    if (ul >= maxval) {
        // calculate 2's complement to get negative value
        ul = ~ul;
        ul &= (maxval - 1);
        signed long sl = ul + 1;
        return (-sl);
    }
    return (signed long) ul;
}

unsigned char *DataItemBits::getSixBitString(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits || numberOfBits % 6) {
        Tracer::Error("Six-bit char representation not valid");
        return (unsigned char *) strdup("???");
    }

    unsigned char *pB = getBits(pData, bytes, frombit, tobit);

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_SIX_BIT_CHAR : Error.");
        return (unsigned char *) strdup("???");
    }

    int numberOfCharacters = numberOfBits / 6;
    unsigned char *str = new unsigned char[numberOfCharacters + 1];
    unsigned char *pStr = str;
    memset(str, 0, numberOfCharacters + 1);

    unsigned char *pTmp = pB;
    unsigned char bitmask = 0x80;
    int outbits = 0;
    unsigned char val = 0;
    while (numberOfBits--) {
        unsigned char bitval = *pTmp & bitmask;
        val <<= 1;
        if (bitval)
            val |= 0x01;

        bitmask >>= 1;
        bitmask &= 0x7F;
        if (bitmask == 0) {
            bitmask = 0x80;
            pTmp++;
        }

        if (outbits++ == 5) {
            outbits = 0;
            *pStr = SIXBITCODE[val];
            val = 0;
            pStr++;
        }
    }
    delete[] pB;
    return str;
}

unsigned char *DataItemBits::getHexBitString(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits || numberOfBits % 4) {
        Tracer::Error("Hex representation not valid");
        return (unsigned char *) strdup("???");
    }

    unsigned char *pB = getBits(pData, bytes, frombit, tobit);

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_HEX_BIT_CHAR : Error.");
        return (unsigned char *) strdup("???");
    }

    int numberOfCharacters = numberOfBits / 4;
    unsigned char *str = new unsigned char[numberOfCharacters + 1];

    memset(str, 0, numberOfCharacters + 1);

    int numberOfBytes = (numberOfCharacters + 1) / 2;
    int i;
    for (i = 0; i < numberOfBytes; i++) {
        sprintf((char *) &str[i * 2], "%02X", pB[i]);
    }

    delete[] pB;
    return str;
}

unsigned char *DataItemBits::getHexBitStringFullByte(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits) {
        Tracer::Error("Hex representation not valid");
        return (unsigned char *) strdup("???");
    }

    if (tobit%8) {
        tobit = (tobit/8 + 1)*8;
    }
    if ((frombit-1)%8) {
        frombit = ((frombit-1)/8)*8+1;
    }
    numberOfBits = (tobit - frombit + 1);

    unsigned char *pB = getBits(pData, bytes, frombit, tobit);

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_HEX_BIT_CHAR : Error.");
        return (unsigned char *) strdup("???");
    }

    int numberOfCharacters = numberOfBits / 4;
    unsigned char *str = new unsigned char[numberOfCharacters + 1];

    memset(str, 0, numberOfCharacters + 1);

    int numberOfBytes = (numberOfCharacters + 1) / 2;
    int i;
    for (i = 0; i < numberOfBytes; i++) {
        sprintf((char *) &str[i * 2], "%02X", pB[i]);
    }

    delete[] pB;
    return str;
}

unsigned char *DataItemBits::getHexBitStringMask(int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits) {
        Tracer::Error("Hex representation not valid");
        return (unsigned char *) strdup("???");
    }

    int tobitStart = tobit;
    if (tobitStart%8) {
        tobitStart = (tobitStart/8 + 1)*8;
    }
    int frombitStart = frombit;
    if ((frombitStart-1)%8) {
        frombitStart = ((frombitStart-1)/8)*8+1;
    }
    numberOfBits = (tobitStart - frombitStart + 1);

    unsigned char *str = new unsigned char[numberOfBits + 1];
    unsigned char *p = str;
    int indx = 0;
    for (int i = tobitStart; i >= frombitStart; --i) {
        if (i >= frombit && i <= tobit)
            p[indx++] = '1';
        else
            p[indx++] = '0';
    }
    p[indx] = 0;
    return str;
}


unsigned char *DataItemBits::getOctal(unsigned char *pData, int bytes, int frombit, int tobit) {
    int numberOfBits = (tobit - frombit + 1);
    if (!numberOfBits || numberOfBits % 3) {
        Tracer::Error("Octal representation not valid");
        return (unsigned char *) strdup("???");
    }

    unsigned char *pB = getBits(pData, bytes, frombit, tobit);

    if (!pB) {
        Tracer::Error("DATAITEM_ENCODING_OCTAL : Error.");
        return (unsigned char *) strdup("???");
    }

    int numberOfCharacters = numberOfBits / 3;
    unsigned char *str = new unsigned char[numberOfCharacters + 1];
    unsigned char *pStr = str;
    memset(str, 0, numberOfCharacters + 1);

    unsigned char *pTmp = pB;
    unsigned char bitmask = 0x80;
    int outbits = 0;
    unsigned char val = 0;

    while (numberOfBits--) {
        unsigned char bitval = *pTmp & bitmask;
        val <<= 1;
        if (bitval)
            val |= 0x01;

        bitmask >>= 1;
        bitmask &= 0x7F;
        if (bitmask == 0) {
            bitmask = 0x80;
            pTmp++;
        }

        if (outbits++ == 2) {
            outbits = 0;
            *pStr = val + '0';
            val = 0;
            pStr++;
        }
    }

    delete[] pB;
    return str;
}

char *DataItemBits::getASCII(unsigned char *pData, int bytes, int frombit, int tobit) {

    int numberOfBits = (tobit - frombit + 1);
    if (bytes < numberOfBits / 8 || !numberOfBits || numberOfBits % 8) {
        Tracer::Error("ASCII representation not valid");
        return strdup("???");
    }

    unsigned char *pTmp = getBits(pData, bytes, frombit, tobit);
    int numberOfBytes = numberOfBits/8;

    char *pStr = new char[numberOfBytes + 1];
    char *ppStr = pStr;

    // replace non alphabetic ASCII characters with empty string
    for (int i = 0; i < numberOfBytes; i++) {
        if (*pData >= 32 && *pData <= 126)
            *ppStr++ = pTmp[i];
        else
            *ppStr++ = ' ';
    }

    delete[] pTmp;
    *ppStr = 0;
    return pStr;
}


bool DataItemBits::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                           unsigned char *pData, long nLength) {
    if (gFiltering && !m_bFiltered)
        return false;

    if (m_nFrom > m_nTo) { // just in case
        int tmp = m_nFrom;
        m_nFrom = m_nTo;
        m_nTo = tmp;
    }

    if (m_nFrom < 1 || m_nTo > nLength * 8) {
        Tracer::Error("Wrong bit format!");
        return true;
    }

    if (m_strName.empty())
        m_strName = m_strShortName;
    else if (m_strShortName.empty())
        m_strShortName = m_strName;

    std::string indent("    ");  // 4 spaces make an indent.

    switch (formatType) {
        case CAsterixFormat::EJSON:
            strResult += format("\"%s\":", m_strShortName.c_str());
            break;
        case CAsterixFormat::EJSONH:
            strResult += format("\n\t\t\"%s\":", m_strShortName.c_str());
            break;
        case CAsterixFormat::EJSONE:
            strResult += format("\n\t\t\"%s\":{", m_strShortName.c_str());
            break;
        case CAsterixFormat::EXML:
            strResult += format("<%s>", m_strShortName.c_str());
            break;
        case CAsterixFormat::EXMLH:
            strResult += format("\n%s%s", indent.c_str(), indent.c_str());  // New line and indent 2 levels (8 spaces).
            strResult += format("<%s>", m_strShortName.c_str());
            break;
    }

    switch (m_eEncoding) {
        case DATAITEM_ENCODING_UNSIGNED: {
            unsigned long value = getUnsigned(pData, nLength, m_nFrom, m_nTo);

            switch (formatType) {
                case CAsterixFormat::ETxt: {
                    strResult += format("\n\t%s: %ld", m_strName.c_str(), value);

                    if (m_dScale != 0) {
                        double scaled = value * m_dScale;
                        strResult += format(" (%.7lf %s)", scaled, m_strUnit.c_str());

                        if (m_bMaxValueSet && scaled > m_dMaxValue) {
                            strResult += format("\n\tWarning: Value larger than max (%.7lf)", m_dMaxValue);
                        }
                        if (m_bMinValueSet && scaled < m_dMinValue) {
                            strResult += format("\n\tWarning: Value smaller than min (%.7lf)", m_dMinValue);
                        }
                    } else if (m_bIsConst) {
                        if ((int) value != m_nConst) {
                            strResult += format("\n\tWarning: Value should be set to %d", m_nConst);
                        }
                    } else if (!m_lValue.empty()) { // check values
                        std::list<BitsValue *>::iterator it;
                        for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                            BitsValue *bv = (BitsValue *) (*it);
                            if (bv->m_nVal == (int) value) {
                                strResult += format(" (%s)", bv->m_strDescription.c_str());
                                break;
                            }
                        }
                        if (it == m_lValue.end()) {
                            strResult += format(" ( ?????? )");
                        }
                    }
                }
                    break;
                case CAsterixFormat::EOut: {
                    strResult += format("\n%s.%s %ld", strHeader.c_str(), m_strShortName.c_str(), value);

                    if (m_dScale != 0) {
                        double scaled = value * m_dScale;
                        strResult += format(" (%.7lf %s)", scaled, m_strUnit.c_str());

                        if (m_bMaxValueSet && scaled > m_dMaxValue) {
                            strResult += format(" tWarning: Value larger than max (%.7lf)", m_dMaxValue);
                        }
                        if (m_bMinValueSet && scaled < m_dMinValue) {
                            strResult += format(" Warning: Value smaller than min (%.7lf)", m_dMinValue);
                        }
                    } else if (m_bIsConst) {
                        if ((int) value != m_nConst) {
                            strResult += format(" Warning: Value should be set to %d", m_nConst);
                        }
                    } else if (!m_lValue.empty()) { // check values
                        std::list<BitsValue *>::iterator it;
                        for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                            BitsValue *bv = (BitsValue *) (*it);
                            if (bv->m_nVal == (int) value) {
                                strResult += format(" (%s)", bv->m_strDescription.c_str());
                                break;
                            }
                        }
                        if (it == m_lValue.end()) {
                            strResult += format(" ( ?????? )");
                        }
                    }
                }
                    break;
                case CAsterixFormat::EJSONE: {
                    if (m_dScale != 0) {
                        double scaled = value * m_dScale;
                        strResult += format("\"val\"=%.7lf", scaled);
                    } else {
                        strResult += format("\"val\"=%ld", value);
                    }

                    unsigned char *hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
                    strResult += format(", \"hex\"=\"%s\"", hexstr);
                    delete[] hexstr;

                    if ((m_nTo - m_nFrom + 1) % 8) {
                        unsigned char *maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                        strResult += format(", \"mask\"=\"%s\"", maskstr);
                        delete[] maskstr;
                    }

                    strResult += format(", \"name\"=\"%s\"", m_strName.c_str());

                    if (!m_lValue.empty()) { // check values
                        std::list<BitsValue *>::iterator it;
                        for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                            BitsValue *bv = (BitsValue *) (*it);
                            if (bv->m_nVal == (int) value) {
                                strResult += format(", \"meaning\"=\"%s\"", bv->m_strDescription.c_str());
                                break;
                            }
                        }
                        if (it == m_lValue.end()) {
                            strResult += format(" ( ?????? )");
                        }
                    }
                }
                    break;
                default: {
                    if (m_dScale != 0) {
                        double scaled = value * m_dScale;
                        strResult += format("%.7lf", scaled);
                    } else {
                        strResult += format("%ld", value);
                    }
                }
            }
        }
            break;

        case DATAITEM_ENCODING_SIGNED: {
            signed long value = getSigned(pData, nLength, m_nFrom, m_nTo);

            switch (formatType) {
                case CAsterixFormat::ETxt: {
                    strResult += format("\n\t%s: %ld", m_strName.c_str(), value);

                    if (m_dScale != 0) {
                        double scaled = (double) value * m_dScale;
                        strResult += format(" (%.7lf %s)", scaled, m_strUnit.c_str());

                        if (m_bMaxValueSet && scaled > m_dMaxValue) {
                            strResult += format("\n\tWarning: Value larger than max (%.7lf)", m_dMaxValue);
                        }
                        if (m_bMinValueSet && scaled < m_dMinValue) {
                            strResult += format("\n\tWarning: Value smaller than min (%.7lf)", m_dMinValue);
                        }
                    }
                }
                    break;
                case CAsterixFormat::EOut: {
                    strResult += format("\n%s.%s %ld", strHeader.c_str(), m_strShortName.c_str(), value);

                    if (m_dScale != 0) {
                        double scaled = (double) value * m_dScale;
                        strResult += format(" (%.7lf %s)", scaled, m_strUnit.c_str());

                        if (m_bMaxValueSet && scaled > m_dMaxValue) {
                            strResult += format(" Warning: Value larger than max (%.7lf)", m_dMaxValue);
                        }
                        if (m_bMinValueSet && scaled < m_dMinValue) {
                            strResult += format(" Warning: Value smaller than min (%.7lf)", m_dMinValue);
                        }
                    }
                }
                    break;
                case CAsterixFormat::EJSONE: {
                    if (m_dScale != 0) {
                        double scaled = value * m_dScale;
                        strResult += format("\"val\"=%.7lf", scaled);
                    } else {
                        strResult += format("\"val\"=%ld", value);
                    }

                    unsigned char *hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
                    strResult += format(", \"hex\"=\"%s\"", hexstr);
                    delete[] hexstr;

                    if ((m_nTo - m_nFrom + 1) % 8) {
                        unsigned char *maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                        strResult += format(", \"mask\"=\"%s\"", maskstr);
                        delete[] maskstr;
                    }

                    strResult += format(", \"name\"=\"%s\"", m_strName.c_str());

                    if (!m_lValue.empty()) { // check values
                        std::list<BitsValue *>::iterator it;
                        for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                            BitsValue *bv = (BitsValue *) (*it);
                            if (bv->m_nVal == (int) value) {
                                strResult += format(", \"meaning\"=\"%s\"", bv->m_strDescription.c_str());
                                break;
                            }
                        }
                        if (it == m_lValue.end()) {
                            strResult += format(" ( ?????? )");
                        }
                    }
                }
                    break;
                default: {
                    if (m_dScale != 0) {
                        double scaled = (double) value * m_dScale;
                        strResult += format("%.7lf", scaled);
                    } else {
                        strResult += format("%ld", value);
                    }
                }
            }
        }
            break;

        case DATAITEM_ENCODING_SIX_BIT_CHAR: {
            unsigned char *str = getSixBitString(pData, nLength, m_nFrom, m_nTo);

            switch (formatType) {
                case CAsterixFormat::ETxt:
                    strResult += format("\n\t%s: %s", m_strName.c_str(), str);
                    break;
                case CAsterixFormat::EOut:
                    strResult += format("\n%s.%s %s", strHeader.c_str(), m_strShortName.c_str(), str);
                    break;
                case CAsterixFormat::EJSON:
                case CAsterixFormat::EJSONH:
                    strResult += format("\"%s\"", str);
                    break;
                case CAsterixFormat::EJSONE: {
                    strResult += format("\"val\"=\"%s\"", str);

                    unsigned char *hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
                    strResult += format(", \"hex\"=\"%s\"", hexstr);
                    delete[] hexstr;

                    if ((m_nTo - m_nFrom + 1) % 8) {
                        unsigned char *maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                        strResult += format(", \"mask\"=\"%s\"", maskstr);
                        delete[] maskstr;
                    }

                    strResult += format(", \"name\"=\"%s\"", m_strName.c_str());
                    break;
                }
                default:
                    strResult += format("%s", str);
                    break;
            }
            delete[] str;
        }
            break;

        case DATAITEM_ENCODING_HEX_BIT_CHAR: {
            unsigned char *str = getHexBitString(pData, nLength, m_nFrom, m_nTo);

            switch (formatType) {
                case CAsterixFormat::ETxt:
                    strResult += format("\n\t%s: %s", m_strName.c_str(), str);
                    break;
                case CAsterixFormat::EOut:
                    strResult += format("\n%s.%s %s", strHeader.c_str(), m_strShortName.c_str(), str);
                    break;
                case CAsterixFormat::EJSON:
                case CAsterixFormat::EJSONH:
                    strResult += format("\"%s\"", str);
                    break;
                case CAsterixFormat::EJSONE: {
                    strResult += format("\"val\"=\"%s\"", str);
                    unsigned char *hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
                    strResult += format(", \"hex\"=\"%s\"", hexstr);
                    delete[] hexstr;

                    if ((m_nTo - m_nFrom + 1) % 8) {
                        unsigned char *maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                        strResult += format(", \"mask\"=\"%s\"", maskstr);
                        delete[] maskstr;
                    }
                    strResult += format(", \"name\"=\"%s\"", m_strName.c_str());
                }
                    break;
                default:
                    strResult += format("%s", str);
                    break;
            }
            delete[] str;
        }
            break;

        case DATAITEM_ENCODING_OCTAL: {
            unsigned char *str = getOctal(pData, nLength, m_nFrom, m_nTo);
            switch (formatType) {
                case CAsterixFormat::ETxt:
                    strResult += format("\n\t%s: %s", m_strName.c_str(), str);
                    break;
                case CAsterixFormat::EOut:
                    strResult += format("\n%s.%s %s", strHeader.c_str(), m_strShortName.c_str(), str);
                    break;
                case CAsterixFormat::EJSON:
                case CAsterixFormat::EJSONH:
                    strResult += format("\"%s\"", str);
                    break;
                case CAsterixFormat::EJSONE: {
                    strResult += format("\"val\"=\"%s\"", str);
                    unsigned char *hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
                    strResult += format(", \"hex\"=\"%s\"", hexstr);
                    delete[] hexstr;

                    if ((m_nTo - m_nFrom + 1) % 8) {
                        unsigned char *maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                        strResult += format(", \"mask\"=\"%s\"", maskstr);
                        delete[] maskstr;
                    }
                    strResult += format(", \"name\"=\"%s\"", m_strName.c_str());
                }
                    break;
                default:
                    strResult += format("%s", str);
                    break;
            }
            delete[] str;
        }
            break;
        case DATAITEM_ENCODING_ASCII: {
            char *pStr = getASCII(pData, nLength, m_nFrom, m_nTo);
            switch (formatType) {
                case CAsterixFormat::ETxt:
                    strResult += format("\n\t%s: %s", m_strName.c_str(), pStr);
                    break;
                case CAsterixFormat::EOut:
                    strResult += format("\n%s.%s %s", strHeader.c_str(), m_strShortName.c_str(), pStr);
                    break;
                case CAsterixFormat::EJSON:
                case CAsterixFormat::EJSONH:
                    strResult += format("\"%s\"", pStr);
                    break;
                case CAsterixFormat::EJSONE: {
                    strResult += format("\"val\"=\"%s\"", pStr);
                    unsigned char *hexstr = getHexBitStringFullByte(pData, nLength, m_nFrom, m_nTo);
                    strResult += format(", \"hex\"=\"%s\"", hexstr);
                    delete[] hexstr;

                    if ((m_nTo - m_nFrom + 1) % 8) {
                        unsigned char *maskstr = getHexBitStringMask(nLength, m_nFrom, m_nTo);
                        strResult += format(", \"mask\"=\"%s\"", maskstr);
                        delete[] maskstr;
                    }
                    strResult += format(", \"name\"=\"%s\"", m_strName.c_str());
                }
                    break;
                default:
                    break;
            }
            delete[] pStr;
        }
            break;
        default:
            Tracer::Error("Unknown encoding");
            break;
    }

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
            strResult += format(",");
            break;
        case CAsterixFormat::EJSONE:
            strResult += format("},");
            break;
        case CAsterixFormat::EXML:
        case CAsterixFormat::EXMLH:
            strResult += format("</%s>", m_strShortName.c_str());
            break;
    }

    return true;
}

std::string DataItemBits::printDescriptors(std::string header) {
    std::string strDes;

    if (gFiltering && !m_bFiltered) {
        strDes = "#";
    }

    strDes += header + m_strShortName;

    int fill = 60 - strDes.length();
    if (fill > 0) {
        std::string strFill(fill, ' ');
        strDes += strFill;
    }

    strDes += " " + m_strName + "\n";

    return strDes;
}

bool DataItemBits::filterOutItem(const char *name) {
    if (0 == strncmp(name, m_strShortName.c_str(), m_strShortName.length())) {
        m_bFiltered = true;
        return true;
    }
    return false;
}

const char *DataItemBits::getDescription(const char *field, const char *value = NULL) {
    if (m_strName.empty() && !m_strShortName.empty())
        m_strName = m_strShortName;
    else if (!m_strName.empty() && m_strShortName.empty())
        m_strShortName = m_strName;


    if (m_strShortName.compare(field) == 0) {
        if (value == NULL) {
            return m_strName.c_str();
        } else {
            int val = atoi(value);
            if (m_lValue.size() > 0) {
                std::list<BitsValue *>::iterator it;
                for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                    BitsValue *bv = (BitsValue *) (*it);
                    if (bv->m_nVal == val)
                        return bv->m_strDescription.c_str();
                }
            }
        }
    }
    return NULL;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemBits::getWiresharkDefinitions()
{
if (m_strName.empty() && !m_strShortName.empty())
m_strName = m_strShortName;
else if (!m_strName.empty() && m_strShortName.empty())
m_strShortName = m_strName;

fulliautomatix_definitions* def = new fulliautomatix_definitions;
memset(def, 0, sizeof(fulliautomatix_definitions));
def->pid = getPID();
def->name = strdup(m_strName.c_str());

// abbrev can only contain -._ and chars
std::string strAbbrev = m_strShortName;
std::size_t index;
while((index=strAbbrev.find_first_not_of ("abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVXYZ_.-0123456789")) != std::string::npos)
strAbbrev.replace(index, 1, "_", 1);
strAbbrev = "asterix." + strAbbrev;

def->abbrev = strdup(strAbbrev.c_str());

def->strings = NULL;

if (m_lValue.size() > 0)
{
def->strings = (fulliautomatix_value_string*)malloc((1+m_lValue.size()) * sizeof(fulliautomatix_value_string));

std::list<BitsValue*>::iterator it;
BitsValue* bv = NULL;
int i=0;
for ( it=m_lValue.begin() ; it != m_lValue.end(); it++ )
{
    bv = (BitsValue*)(*it);
    def->strings[i].value = bv->m_nVal;
    def->strings[i].strptr = strdup(bv->m_strDescription.c_str());
    i++;
}
def->strings[i].value = 0;
def->strings[i].strptr = NULL;
}

if (m_nFrom > m_nTo)
{ // just in case
int tmp = m_nFrom;
m_nFrom = m_nTo;
m_nTo = tmp;
}

switch(m_eEncoding)
{
case DATAITEM_ENCODING_UNSIGNED:
{
int numberOfBits = (m_nTo-m_nFrom+1);

if (numberOfBits < 8)
{
    def->type = FA_FT_UINT8;
    def->display = FA_BASE_DEC;

    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1)%8;

}
else if (numberOfBits == 8)
{
    def->type = FA_FT_UINT8;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits < 16)
{
    def->type = FA_FT_UINT16;
    def->display = FA_BASE_DEC;
    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1)%16;
}
else if (numberOfBits == 16)
{
    def->type = FA_FT_UINT16;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits < 24)
{
    def->type = FA_FT_UINT24;
    def->display = FA_BASE_DEC;
    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1);
}
else if (numberOfBits == 24)
{
    def->type = FA_FT_UINT24;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits == 32)
{
    def->type = FA_FT_UINT32;
    def->display = FA_BASE_DEC;
}
else
{
    def->type = FA_FT_UINT32;
    def->display = FA_BASE_DEC;
    def->bitmask = 0x01;
    def->bitmask <<= numberOfBits;
    def->bitmask--;
    def->bitmask <<= (m_nFrom-1);
}
}
break;

case DATAITEM_ENCODING_SIGNED:
{
int numberOfBits = (m_nTo-m_nFrom+1);

if (numberOfBits <= 8)
{
    def->type = FA_FT_INT8;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits <= 16)
{
    def->type = FA_FT_INT16;
    def->display = FA_BASE_DEC;
}
else if (numberOfBits <= 24)
{
    def->type = FA_FT_INT24;
    def->display = FA_BASE_DEC;
}
else
{
    def->type = FA_FT_INT32;
    def->display = FA_BASE_DEC;
}
}
break;

case DATAITEM_ENCODING_SIX_BIT_CHAR:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
case DATAITEM_ENCODING_HEX_BIT_CHAR:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
case DATAITEM_ENCODING_OCTAL:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
case DATAITEM_ENCODING_ASCII:
{
def->type = FA_FT_STRING;
def->display = FA_BASE_NONE;
}
break;
default:
Tracer::Error("Unknown encoding");
break;
}

return def;
}

fulliautomatix_data* DataItemBits::getData(unsigned char* pData, long nLength, int byteoffset)
{
    if (m_nFrom > m_nTo)
    { // just in case
        int tmp = m_nFrom;
        m_nFrom = m_nTo;
        m_nTo = tmp;
    }

    int firstByte = nLength - (m_nTo-1)/8 - 1;
    int numberOfBits = (m_nTo-m_nFrom+1);
    int numberOfBytes = (numberOfBits+7)/8;

    if (m_strName.empty())
        m_strName = m_strShortName;

    switch(m_eEncoding)
    {
    case DATAITEM_ENCODING_UNSIGNED:
    {
        fulliautomatix_data* pOutData;
        unsigned long value = getUnsigned(pData, nLength, m_nFrom, m_nTo);
        if (value && m_nFrom>1 && numberOfBits%8)
        { // this is because of bitmask presentation in Wireshark
            value <<= ((m_nFrom-1)%(numberOfBytes*8));
        }
        pOutData = newDataUL(NULL, getPID(), byteoffset+firstByte, numberOfBytes, value);

        if (m_dScale != 0 || m_bIsConst)
        {
            double scaled = value*m_dScale;
            char tmp[128];
            if ((m_bMaxValueSet && scaled > m_dMaxValue) || (m_bMinValueSet && scaled < m_dMinValue))
            {
                snprintf(tmp, 128, " (%.7lf %s) Warning! Value out of range (%.7lf to %.7lf)", scaled, m_strUnit.c_str(), m_dMinValue, m_dMaxValue);
                pOutData->err = 1;
            }
            else if (m_bIsConst && (int)value != m_nConst)
            {
                snprintf(tmp, 128, " (%.7lf %s) Warning! Value should be %d", scaled, m_strUnit.c_str(), m_nConst);
                pOutData->err = 1;
            }
            else
            {
                snprintf(tmp, 128, " (%.7lf %s)", scaled, m_strUnit.c_str());
            }
            pOutData->value_description = strdup(tmp);
        }
        return pOutData;
    }
    break;

    case DATAITEM_ENCODING_SIGNED:
    {
        fulliautomatix_data* pOutData;
        signed long value = getSigned(pData, nLength, m_nFrom, m_nTo);
        pOutData = newDataSL(NULL, getPID(), byteoffset+firstByte, numberOfBytes, value);

        if (m_dScale != 0)
        {
            double scaled = value*m_dScale;
            char tmp[128];
            if ((m_bMaxValueSet && scaled > m_dMaxValue) || (m_bMinValueSet && scaled < m_dMinValue))
            {
                snprintf(tmp, 128, " (%.7lf %s) Warning! Value out of range (%.7lf to %.7lf)", scaled, m_strUnit.c_str(), m_dMinValue, m_dMaxValue);
                pOutData->err = 1;
            }
            else if (m_bIsConst && (int)value != m_nConst)
            {
                snprintf(tmp, 128, " (%.7lf %s) Warning! Value should be %d", scaled, m_strUnit.c_str(), m_nConst);
                pOutData->err = 1;
            }
            else
            {
                snprintf(tmp, 128, " (%.7lf %s)", scaled, m_strUnit.c_str());
            }
            pOutData->value_description = strdup(tmp);
        }
        return pOutData;
    }
    break;

    case DATAITEM_ENCODING_SIX_BIT_CHAR:
    {
        unsigned char* str = getSixBitString(pData, nLength, m_nFrom, m_nTo);
        fulliautomatix_data* data = newDataString(NULL, getPID(), byteoffset+firstByte, numberOfBytes, (char*)str);
        delete[] str;
        return data;
    }
    break;
    case DATAITEM_ENCODING_HEX_BIT_CHAR:
    {
        unsigned char* str = getHexBitString(pData, nLength, m_nFrom, m_nTo);
        fulliautomatix_data* data = newDataString(NULL, getPID(), byteoffset+firstByte, numberOfBytes, (char*)str);
        delete[] str;
        return data;
    }
    break;
    case DATAITEM_ENCODING_OCTAL:
    {
        unsigned char* str = getOctal(pData, nLength, m_nFrom, m_nTo);
        fulliautomatix_data* data = newDataString(NULL, getPID(), byteoffset+firstByte, numberOfBytes, (char*)str);
        delete[] str;
        return data;
    }
    break;
    case DATAITEM_ENCODING_ASCII:
    {
        char* pStr = getASCII(pData, nLength, m_nFrom, m_nTo);
        fulliautomatix_data* data = newDataString(NULL, getPID(), byteoffset+firstByte, numberOfBytes, pStr);
        delete pStr;
        return data;
    }
    break;
    default:
        Tracer::Error("Unknown encoding");
        break;
    }

    fulliautomatix_data* data = newDataMessage(NULL, byteoffset+firstByte, numberOfBytes, 2, (char*)"Error: Unknown encoding.");
    return data;
}
#endif

#if defined(PYTHON_WRAPPER)

PyObject* DataItemBits::getObject(unsigned char* pData, long nLength, int verbose)
{
    PyObject* p = PyDict_New();
    insertToDict(p, pData, nLength, verbose);
    return p;
}

void DataItemBits::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
    // Add item name
    PyObject* pValue = PyDict_New();
    PyObject* k2 = Py_BuildValue("s", m_strShortName.c_str());
    PyDict_SetItem(p, k2, pValue);
    Py_DECREF(k2);
    Py_DECREF(pValue);

    if (verbose)
    {
        // Add item description
        PyObject* k1 = Py_BuildValue("s", "desc");
        PyObject* v1 = Py_BuildValue("s", m_strName.c_str());
        PyDict_SetItem(pValue, k1, v1);
        Py_DECREF(k1);
        Py_DECREF(v1);
    }

    if (m_nFrom > m_nTo)
    { // just in case
        int tmp = m_nFrom;
        m_nFrom = m_nTo;
        m_nTo = tmp;
    }

    //int firstByte = nLength - (m_nTo-1)/8 - 1;
    //int numberOfBits = (m_nTo-m_nFrom+1);
    //int numberOfBytes = (numberOfBits+7)/8;

    switch(m_eEncoding)
    {
    case DATAITEM_ENCODING_UNSIGNED:
    {
        unsigned long value = getUnsigned(pData, nLength, m_nFrom, m_nTo);

        if (m_dScale != 0) {
            double scaled = value * m_dScale;
            PyObject* k1 = Py_BuildValue("s", "val");
            PyObject* v1 = Py_BuildValue("d", scaled);
            PyDict_SetItem(pValue, k1, v1);
            Py_DECREF(k1);
            Py_DECREF(v1);

            if (verbose && m_bMaxValueSet) {
                PyObject* k1 = Py_BuildValue("s", "max");
                PyObject* v1 = Py_BuildValue("d", m_dMaxValue);
                PyDict_SetItem(pValue, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
            }
            if (verbose && m_bMinValueSet) {
                PyObject* k1 = Py_BuildValue("s", "min");
                PyObject* v1 = Py_BuildValue("d", m_dMinValue);
                PyDict_SetItem(pValue, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
            }
        } else if (m_bIsConst) {
            PyObject* k1 = Py_BuildValue("s", "val");
            PyObject* v1 = Py_BuildValue("k", value);
            PyDict_SetItem(pValue, k1, v1);
            Py_DECREF(k1);
            Py_DECREF(v1);
            if (verbose) {
                PyObject* k2 = Py_BuildValue("s", "const");
                PyObject* v2 = Py_BuildValue("k", m_nConst);
                PyDict_SetItem(pValue, k2, v2);
                Py_DECREF(k2);
                Py_DECREF(v2);
            }
        } else if (!m_lValue.empty()) { // check values
            std::list<BitsValue*>::iterator it;
            for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                BitsValue* bv = (BitsValue*) (*it);
                if (bv->m_nVal == (int) value) {
                    PyObject* k1 = Py_BuildValue("s", "val");
                    PyObject* v1 = Py_BuildValue("k", value);
                    PyDict_SetItem(pValue, k1, v1);
                    Py_DECREF(k1);
                    Py_DECREF(v1);
                    if (verbose && !bv->m_strDescription.empty()) {
                        PyObject* k2 = Py_BuildValue("s", "meaning");
                        PyObject* v2 = Py_BuildValue("s", bv->m_strDescription.c_str());
                        PyDict_SetItem(pValue, k2, v2);
                        Py_DECREF(k2);
                        Py_DECREF(v2);
                    }
                    break;
                }
            }
            if (it == m_lValue.end()) {
                PyObject* k1 = Py_BuildValue("s", "val");
                PyObject* v1 = Py_BuildValue("k", value);
                PyDict_SetItem(pValue, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
                if (verbose) {
                    PyObject* k2 = Py_BuildValue("s", "meaning");
                    PyObject* v2 = Py_BuildValue("s", "???");
                    PyDict_SetItem(pValue, k2, v2);
                    Py_DECREF(k2);
                    Py_DECREF(v2);
                }
            }
        }
        else
        {
            PyObject* k1 = Py_BuildValue("s", "val");
            PyObject* v1 = Py_BuildValue("k", value);
            PyDict_SetItem(pValue, k1, v1);
            Py_DECREF(k1);
            Py_DECREF(v1);
        }
    }
    break;

    case DATAITEM_ENCODING_SIGNED:
    {
        signed long value = getSigned(pData, nLength, m_nFrom, m_nTo);

        if (m_dScale != 0) {
            double scaled = value * m_dScale;
            PyObject* k1 = Py_BuildValue("s", "val");
            PyObject* v1 = Py_BuildValue("d", scaled);
            PyDict_SetItem(pValue, k1, v1);
            Py_DECREF(k1);
            Py_DECREF(v1);

            if (verbose && m_bMaxValueSet) {
                PyObject* k1 = Py_BuildValue("s", "max");
                PyObject* v1 = Py_BuildValue("d", m_dMaxValue);
                PyDict_SetItem(pValue, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
            }
            if (verbose && m_bMinValueSet) {
                PyObject* k1 = Py_BuildValue("s", "min");
                PyObject* v1 = Py_BuildValue("d", m_dMinValue);
                PyDict_SetItem(pValue, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
            }
        } else if (m_bIsConst) {
            PyObject* k1 = Py_BuildValue("s", "val");
            PyObject* v1 = Py_BuildValue("d", value);
            PyDict_SetItem(pValue, k1, v1);
            Py_DECREF(k1);
            Py_DECREF(v1);
            if (verbose) {
                PyObject* k2 = Py_BuildValue("s", "const");
                PyObject* v2 = Py_BuildValue("k", m_nConst);
                PyDict_SetItem(pValue, k2, v2);
                Py_DECREF(k2);
                Py_DECREF(v2);
            }
        } else if (!m_lValue.empty()) { // check values
            std::list<BitsValue*>::iterator it;
            for (it = m_lValue.begin(); it != m_lValue.end(); it++) {
                BitsValue* bv = (BitsValue*) (*it);
                if (bv->m_nVal == (int) value) {
                    PyObject* k1 = Py_BuildValue("s", "val");
                    PyObject* v1 = Py_BuildValue("d", value);
                    PyDict_SetItem(pValue, k1, v1);
                    Py_DECREF(k1);
                    Py_DECREF(v1);
                    if (verbose && !bv->m_strDescription.empty()) {
                        PyObject* k2 = Py_BuildValue("s", "meaning");
                        PyObject* v2 = Py_BuildValue("s", bv->m_strDescription.c_str());
                        PyDict_SetItem(pValue, k2, v2);
                        Py_DECREF(k2);
                        Py_DECREF(v2);
                    }
                    break;
                }
            }
            if (it == m_lValue.end()) {
                PyObject* k1 = Py_BuildValue("s", "val");
                PyObject* v1 = Py_BuildValue("d", value);
                PyDict_SetItem(pValue, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
                if (verbose) {
                    PyObject* k2 = Py_BuildValue("s", "meaning");
                    PyObject* v2 = Py_BuildValue("s", "???");
                    PyDict_SetItem(pValue, k2, v2);
                    Py_DECREF(k2);
                    Py_DECREF(v2);
                }
            }
        }
        else
        {
            PyObject* k1 = Py_BuildValue("s", "val");
            PyObject* v1 = Py_BuildValue("l", value);
            PyDict_SetItem(pValue, k1, v1);
            Py_DECREF(k1);
            Py_DECREF(v1);
        }
    }
    break;

    case DATAITEM_ENCODING_SIX_BIT_CHAR:
    {
        unsigned char* str = getSixBitString(pData, nLength, m_nFrom, m_nTo);
        PyObject* k1 = Py_BuildValue("s", "val");
        PyObject* v1 = Py_BuildValue("s", str);
        PyDict_SetItem(pValue, k1, v1);
        Py_DECREF(k1);
        Py_DECREF(v1);
        delete[] str;
    }
    break;

    case DATAITEM_ENCODING_HEX_BIT_CHAR:
    {
        unsigned char* str = getHexBitString(pData, nLength, m_nFrom, m_nTo);
        PyObject* k1 = Py_BuildValue("s", "val");
        PyObject* v1 = Py_BuildValue("s", str);
        PyDict_SetItem(pValue, k1, v1);
        Py_DECREF(k1);
        Py_DECREF(v1);
        delete[] str;
    }
    break;

    case DATAITEM_ENCODING_OCTAL:
    {
        unsigned char* str = getOctal(pData, nLength, m_nFrom, m_nTo);
        PyObject* k1 = Py_BuildValue("s", "val");
        PyObject* v1 = Py_BuildValue("s", str);
        PyDict_SetItem(pValue, k1, v1);
        Py_DECREF(k1);
        Py_DECREF(v1);
        delete[] str;
    }
    break;

    case DATAITEM_ENCODING_ASCII:
    {
        char* pStr = getASCII(pData, nLength, m_nFrom, m_nTo);
        PyObject* k1 = Py_BuildValue("s", "val");
        PyObject* v1 = Py_BuildValue("s", pStr);
        PyDict_SetItem(pValue, k1, v1);
        Py_DECREF(k1);
        Py_DECREF(v1);
        delete[] pStr;
    }
    break;
    default:
        PyObject* k1 = Py_BuildValue("s", "val");
        PyObject* v1 = Py_BuildValue("s", "???");
        PyDict_SetItem(pValue, k1, v1);
        Py_DECREF(k1);
        Py_DECREF(v1);
        break;
    }
}
#endif
