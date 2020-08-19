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

DataItemFormatFixed::DataItemFormatFixed(int id)
        : DataItemFormat(id), m_nLength(0) {
}

DataItemFormatFixed::DataItemFormatFixed(const DataItemFormatFixed &obj)
        : DataItemFormat(obj.m_nID), m_nLength(obj.m_nLength) {
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

long DataItemFormatFixed::getLength(const unsigned char *) {
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
    if (m_nLength != nLength) {
        Tracer::Error("Length doesn't match!!!");
        return false;
    }

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            strResult += '{';
        }
            break;
    }

    bool ret = false;
    std::list<DataItemFormat *>::iterator it;
    DataItemBits *bv = NULL;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        bv = (DataItemBits *) (*it);
        ret |= bv->getText(strResult, strHeader, formatType, pData, m_nLength);
    }

    switch (formatType) {
        case CAsterixFormat::EJSON:
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
