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

#include "DataItemFormatFixed.h"
#include "Tracer.h"
#include "asterixformat.hxx"

extern bool gFiltering;

DataItemFormatFixed::DataItemFormatFixed(int id)
        : DataItemFormat(id), m_nLength(0), m_bJsonPrefixesSet(false), m_bPresenceBitsSet(false) {
}

DataItemFormatFixed::DataItemFormatFixed(const DataItemFormatFixed &obj)
        : DataItemFormat(obj.m_nID), m_nLength(obj.m_nLength), m_bJsonPrefixesSet(false), m_bPresenceBitsSet(false) {
    std::list<DataItemFormat *>::iterator it = ((DataItemFormat &) obj).m_lSubItems.begin();

    while (it != obj.m_lSubItems.end()) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        m_lSubItems.push_back(di->clone());
        it++;
    }

    m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatFixed::~DataItemFormatFixed() {
    // destroy list items
    std::list<DataItemFormat *>::iterator it = m_lSubItems.begin();
    while (it != m_lSubItems.end()) {
        delete (DataItemBits *) (*it);
        it = m_lSubItems.erase(it);
    }
}

long DataItemFormatFixed::getLength() {
    return m_nLength;
}

long DataItemFormatFixed::getLength(const unsigned char *, long) { // no data is read
    return m_nLength;
}

/*
 * Check FX bit to see if this is last part of variable item.
 */
bool DataItemFormatFixed::isLastPart(const unsigned char *pData) {
    // go through all bits and find which is FX
    std::list<DataItemFormat *>::iterator bitit;
    for (bitit = m_lSubItems.begin(); bitit != m_lSubItems.end(); bitit++) {
        DataItemBits *bit = (DataItemBits *) (*bitit);
        if (bit == NULL) {
            Tracer::Error("Missing bits format!");
            return true;
        }
        if (bit->m_bExtension) { // this is extension bit
            int bitnr = bit->m_nFrom;

            if (bitnr < 1 || bitnr > m_nLength * 8) {
                Tracer::Error("Error in bits format");
                return true;
            }

            // select byte
            int bytenr = m_nLength - 1 - (bitnr - 1) / 8;

            pData += bytenr;

            bitnr = bitnr % 8;
            if (bitnr == 0)
                bitnr = 8;

            unsigned char mask = 0x01;
            mask <<= (bitnr - 1);

            // check this bit in pData
            if (*pData & mask) {
                return false;
            }
            return true;
        }
    }
    // If the fx bit is not found it means that this is last part.
    // Tracer::Error("Missing fx bit in variable item!");
    return true;
}

std::string &DataItemFormatFixed::getPartName(int part) {
    static std::string unknown = "unknown";

    // go through all bits and find which has BitsPresence set to part
    std::list<DataItemFormat *>::iterator bitit;
    for (bitit = m_lSubItems.begin(); bitit != m_lSubItems.end(); bitit++) {
        DataItemBits *bit = (DataItemBits *) (*bitit);
        if (bit != NULL && bit->m_nPresenceOfField == part) { // found
            if (bit->m_strShortName.empty())
                return bit->m_strName;
            return bit->m_strShortName;
        }
    }
    Tracer::Error("Compound part not found!");
    return unknown;
}

bool DataItemFormatFixed::isSecondaryPartPresent(const unsigned char *pData, int part) {
    // Called for every secondary part of every Compound item: the bit of each part is looked up once.
    if (!m_bPresenceBitsSet) {
        m_bPresenceBitsSet = true;
        std::list<DataItemFormat *>::iterator bitit;
        for (bitit = m_lSubItems.begin(); bitit != m_lSubItems.end(); bitit++) {
            DataItemBits *bit = (DataItemBits *) (*bitit);
            if (bit == NULL) {
                m_vPresenceBits.clear(); // let the loop below report it
                break;
            }
            int p = bit->m_nPresenceOfField;
            if (p > 0) {
                if ((size_t) p >= m_vPresenceBits.size())
                    m_vPresenceBits.resize(p + 1, 0);
                if (m_vPresenceBits[p] == 0) // the first bit with this part is the one used
                    m_vPresenceBits[p] = bit->m_nFrom >= 1 ? bit->m_nFrom : -1;
            }
        }
        if (bitit == m_lSubItems.end() && m_vPresenceBits.empty())
            m_vPresenceBits.push_back(0); // no parts at all, cache is still valid
    }
    if (part > 0 && !m_vPresenceBits.empty()) {
        int bitnr = (size_t) part < m_vPresenceBits.size() ? m_vPresenceBits[part] : 0;
        if (bitnr == 0)
            return false;
        if (bitnr > 0 && bitnr <= m_nLength * 8) {
            int shift = (bitnr - 1) % 8;
            return (pData[m_nLength - 1 - (bitnr - 1) / 8] & (0x01 << shift)) != 0;
        }
        // out of range: the loop below reports it
    }

    // go through all bits and find which has BitsPresence set to part
    std::list<DataItemFormat *>::iterator bitit;
    for (bitit = m_lSubItems.begin(); bitit != m_lSubItems.end(); bitit++) {
        DataItemBits *bit = (DataItemBits *) (*bitit);
        if (bit == NULL) {
            Tracer::Error("Missing bits format!");
            return true;
        }
        if (bit->m_nPresenceOfField == part) { // found
            int bitnr = bit->m_nFrom;

            if (bitnr < 1 || bitnr > m_nLength * 8) {
                Tracer::Error("Error in bits format");
                return true;
            }

            // select byte
            int bytenr = m_nLength - 1 - (bitnr - 1) / 8;
            pData += bytenr;

            int shift = (bitnr - 1) % 8;
            unsigned char mask = 0x01;
            mask <<= shift;

            // check this bit in pData
            if (*pData & mask) {
                return true;
            }
            return false;
        }
    }
    // Tracer::Error("BitsPresence bit %d not found", part);
    return false;
}

bool DataItemFormatFixed::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                  unsigned char *pData, long nLength) {
    if (formatType == CAsterixFormat::EJSONX) {
        if (!m_bJsonPrefixesSet) {
            std::set<std::string> jsonKeys; // a Fixed item on its own is one JSON object
            makeJsonPrefixes(jsonKeys, 0, m_vJsonPrefixes);
            m_bJsonPrefixesSet = true;
        }
        return getText(strResult, strHeader, formatType, pData, nLength, m_vJsonPrefixes);
    }
    static const std::vector<std::string> noPrefixes;
    return getText(strResult, strHeader, formatType, pData, nLength, noPrefixes);
}

/*
 * Key of a bits field in EJSONX, unique within the JSON object being written:
 *  - extension (FX) bits are numbered by extent: FX0, FX1, ... so FXn is always the FX of extent n
 *  - any other repeated name gets the first free number: spare, spare1, spare2, ...
 */
static std::string jsonUniqueKey(DataItemBits *bv, std::set<std::string> &jsonKeys, int extent) {
    std::string base = bv->getJsonBaseName();
    std::string key = bv->m_bExtension ? base + std::to_string(extent) : base;
    for (int n = 1; jsonKeys.count(key); n++) {
        key = base + std::to_string(n);
    }
    jsonKeys.insert(key);
    return key;
}

void DataItemFormatFixed::makeJsonPrefixes(std::set<std::string> &jsonKeys, int extent,
                                           std::vector<std::string> &jsonPrefixes) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemBits *bv = (DataItemBits *) (*it);
        jsonPrefixes.push_back("\"" + jsonUniqueKey(bv, jsonKeys, extent) + "\":{");
    }
}

bool DataItemFormatFixed::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                  unsigned char *pData, long nLength, const std::vector<std::string> &jsonPrefixes) {
    if (m_nLength != nLength) {
        Tracer::Error("Length doesn't match!!!");
        return false;
    }

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONX:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            strResult += '{';
        }
            break;
    }

    bool ret = false;
    std::list<DataItemFormat *>::iterator it;
    DataItemBits *bv = NULL;
    size_t nField = 0;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++, nField++) {
        bv = (DataItemBits *) (*it);
        if (gFiltering && !bv->m_bFiltered) {
            continue; // not in the filter (checked here too, it is most of the calls with -LF)
        }
        if (formatType == CAsterixFormat::EJSONX) {
            const std::string *pPrefix = nField < jsonPrefixes.size() ? &jsonPrefixes[nField] : NULL;
            ret |= bv->getText(strResult, strHeader, formatType, pData, m_nLength, pPrefix);
        } else {
            ret |= bv->getText(strResult, strHeader, formatType, pData, m_nLength);
        }
    }

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONX:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            if (strResult[strResult.length() - 1] == ',')
                strResult[strResult.length() - 1] = '}';
            else
                strResult += '}';
        }
            break;
    }

    return ret;
}

std::string DataItemFormatFixed::printDescriptors(std::string header) {
    std::string strDef = "";

    std::list<DataItemFormat *>::iterator it;
    DataItemBits *bv = NULL;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        bv = (DataItemBits *) (*it);
        strDef += bv->printDescriptors(header);
    }
    return strDef;
}

bool DataItemFormatFixed::filterOutItem(const char *name) {
    std::list<DataItemFormat *>::iterator it;
    DataItemBits *bv = NULL;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        bv = (DataItemBits *) (*it);
        if (true == bv->filterOutItem(name))
            return true;
    }
    return false;
}

bool DataItemFormatFixed::isFiltered(const char *name) {
    std::list<DataItemFormat *>::iterator it;
    DataItemBits *bv = NULL;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        bv = (DataItemBits *) (*it);
        if (true == bv->isFiltered(name))
            return true;
    }
    return false;
}

const char *DataItemFormatFixed::getDescription(const char *field, const char *value = NULL) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemBits *bv = (DataItemBits *) (*it);
        const char *desc = bv->getDescription(field, value);
        if (desc != NULL)
            return desc;
    }
    return NULL;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatFixed::getWiresharkDefinitions()
{
  fulliautomatix_definitions *def = NULL, *startDef = NULL;
  std::list<DataItemFormat*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lSubItems.begin() ; it != m_lSubItems.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (def)
    {
      def->next = bv->getWiresharkDefinitions();
      def = def->next;
    }
    else
    {
      startDef = def = bv->getWiresharkDefinitions();
    }
  }
  return startDef;
}

fulliautomatix_data* DataItemFormatFixed::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;
  std::list<DataItemFormat*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lSubItems.begin() ; it != m_lSubItems.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (lastData)
    {
      lastData->next = bv->getData(pData, len, byteoffset);
      lastData = lastData->next;
    }
    else
    {
      firstData = lastData = bv->getData(pData, len, byteoffset);
    }
  }
  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
PyObject* DataItemFormatFixed::getObject(unsigned char* pData, long nLength, int description)
{
    PyObject* p = PyDict_New();
    insertToDict(p, pData, nLength, description);
    return p;
}

void DataItemFormatFixed::insertToDict(PyObject* p, unsigned char* pData, long nLength, int description)
{
    std::list<DataItemFormat*>::iterator it;
    DataItemBits* bv = NULL;
    for ( it=m_lSubItems.begin() ; it != m_lSubItems.end(); it++ )
    {
        bv = (DataItemBits*)(*it);
        bv->insertToDict(p, pData, nLength, description);
    }
}

#endif
