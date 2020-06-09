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

#include "DataItemFormatRepetitive.h"
#include "Tracer.h"
#include "asterixformat.hxx"

DataItemFormatRepetitive::DataItemFormatRepetitive(int id)
        : DataItemFormat(id) {
}

DataItemFormatRepetitive::DataItemFormatRepetitive(const DataItemFormatRepetitive &obj)
        : DataItemFormat(obj.m_nID) {
    std::list<DataItemFormat *>::iterator it = ((DataItemFormat &) obj).m_lSubItems.begin();

    while (it != obj.m_lSubItems.end()) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        m_lSubItems.push_back(di->clone());
        it++;
    }

    m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatRepetitive::~DataItemFormatRepetitive() {
}

long DataItemFormatRepetitive::getLength(const unsigned char *pData) {
    DataItemFormat *pF = m_lSubItems.size() ? (DataItemFormatFixed *) m_lSubItems.front() : NULL;
    if (pF == NULL) {
        Tracer::Error("Wrong data in Repetitive");
        return 0;
    }
    unsigned char nRepetition = *pData;
    return (1 + nRepetition * pF->getLength(pData + 1));
}

bool DataItemFormatRepetitive::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                       unsigned char *pData, long nLength) {
    bool ret = false;
    DataItemFormatFixed *pF = m_lSubItems.size() ? (DataItemFormatFixed *) m_lSubItems.front() : NULL;
    if (pF == NULL) {
        Tracer::Error("Wrong data in Repetitive");
        return true;
    }

    int fixedLength = pF->getLength(pData);
    unsigned char nRepetition = *pData;

    if (1 + nRepetition * fixedLength != nLength) {
        Tracer::Error("Wrong length in Repetitive");
        return true;
    }

    pData++;

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            std::string tmpStr = format("[");

            while (nRepetition--) {
                ret |= pF->getText(tmpStr, strHeader, formatType, pData, fixedLength);
                pData += fixedLength;

                if (nRepetition > 0)
                    tmpStr += format(",");
            }
            tmpStr += format("]");

            if (ret)
                strResult += tmpStr;

            break;
        }
        default: {
            while (nRepetition--) {
                ret |= pF->getText(strResult, strHeader, formatType, pData, fixedLength);
                pData += fixedLength;
            }
            break;
        }
    }
    return ret;
}

std::string DataItemFormatRepetitive::printDescriptors(std::string header) {
    DataItemFormatFixed *pFixed = m_lSubItems.size() ? (DataItemFormatFixed *) m_lSubItems.front() : NULL;
    if (pFixed == NULL) {
        Tracer::Error("Wrong data in Repetitive");
        return "Wrong data in Repetitive";
    }

    return pFixed->printDescriptors(header);
}

bool DataItemFormatRepetitive::filterOutItem(const char *name) {
    DataItemFormatFixed *pFixed = m_lSubItems.size() ? (DataItemFormatFixed *) m_lSubItems.front() : NULL;
    if (pFixed == NULL) {
        Tracer::Error("Wrong data in Repetitive");
        return false;
    }

    return pFixed->filterOutItem(name);
}

bool DataItemFormatRepetitive::isFiltered(const char *name) {
    DataItemFormatFixed *pFixed = m_lSubItems.size() ? (DataItemFormatFixed *) m_lSubItems.front() : NULL;
    if (pFixed == NULL) {
        Tracer::Error("Wrong data in Repetitive");
        return false;
    }

    return pFixed->isFiltered(name);
}

const char *DataItemFormatRepetitive::getDescription(const char *field, const char *value = NULL) {
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
fulliautomatix_definitions* DataItemFormatRepetitive::getWiresharkDefinitions()
{
    DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
    if (!pFixed)
    {
        Tracer::Error("Wring format of repetitive item");
        return NULL;
    }
    return pFixed->getWiresharkDefinitions();
}

fulliautomatix_data* DataItemFormatRepetitive::getData(unsigned char* pData, long len, int byteoffset)
{
    fulliautomatix_data *lastData = NULL, *firstData = NULL;
    DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
    if (!pFixed)
    {
        Tracer::Error("Wrong format of repetitive item");
        return NULL;
    }

    int fixedLength = pFixed->getLength(pData);
    unsigned char nRepetition = *pData;

    firstData = lastData = newDataUL(NULL, PID_REP, byteoffset, 1, nRepetition);
    byteoffset+=1;

    if (1+nRepetition*fixedLength != len)
    {
        Tracer::Error("Wrong length in Repetitive");
        return firstData;
    }

    pData++;

    while(nRepetition--)
    {
        lastData->next = pFixed->getData(pData, fixedLength, byteoffset);
        while(lastData->next)
            lastData = lastData->next;

        pData += fixedLength;
        byteoffset += fixedLength;
    }

    return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)

PyObject* DataItemFormatRepetitive::getObject(unsigned char* pData, long nLength, int verbose)
{
    PyObject* p = PyList_New(0);

     DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
      if (!pFixed)
      {
        // TODO Tracer::Error("Wrong format of repetitive item");
        return NULL;
      }

      int fixedLength = pFixed->getLength(pData);
      unsigned char nRepetition = *pData;

      if (1+nRepetition*fixedLength != nLength)
      {
        //TODO Tracer::Error("Wrong length in Repetitive");
        return NULL;
      }

      pData++;

      while(nRepetition--)
      {
        PyObject* p1 = pFixed->getObject(pData, fixedLength, verbose);
        PyList_Append(p, p1);
        Py_DECREF(p1);
        pData += fixedLength;
      }

    return p;
}


void DataItemFormatRepetitive::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
 // Not supported
}
#endif
