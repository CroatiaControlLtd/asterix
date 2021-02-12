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

#include "DataItemFormatExplicit.h"
#include "Tracer.h"
#include "asterixformat.hxx"

DataItemFormatExplicit::DataItemFormatExplicit(int id)
        : DataItemFormat(id) {
}

DataItemFormatExplicit::DataItemFormatExplicit(const DataItemFormatExplicit &obj)
        : DataItemFormat(obj.m_nID) {
    std::list<DataItemFormat *>::iterator it = ((DataItemFormat &) obj).m_lSubItems.begin();

    while (it != obj.m_lSubItems.end()) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        m_lSubItems.push_back(di->clone());
        it++;
    }

    m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatExplicit::~DataItemFormatExplicit() {
}

long DataItemFormatExplicit::getLength(const unsigned char *pData) {
    return (long) (*pData);
}

bool DataItemFormatExplicit::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                     unsigned char *pData, long nLength) {
    std::list<DataItemFormat *>::iterator it;
    int bodyLength = 0;
    bool ret = false;

    pData++; // skip explicit length byte (it is already in nLength)

    // calculate the size of all sub items
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        bodyLength += di->getLength(pData + bodyLength); // calculate length of body
    }

    int nFullLength = nLength - 1; // calculate full length

    // full length must be multiple of body length
    if (bodyLength == 0 || nFullLength % bodyLength != 0) {
        Tracer::Error("Wrong data length in Explicit. Needed=%d and there is %d bytes.", bodyLength, nFullLength);
    }

    std::string tmpStr = "";

    if (nFullLength != bodyLength) {
        switch (formatType) {
            case CAsterixFormat::EJSON:
            case CAsterixFormat::EJSONH:
            case CAsterixFormat::EJSONE: {
                tmpStr += format("[");
            }
        }
    }

    for (int i = 0; i < nFullLength; i += bodyLength) {
        for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
            DataItemFormat *di = (DataItemFormat *) (*it);
            ret |= di->getText(tmpStr, strHeader, formatType, pData, bodyLength);
            pData += bodyLength;

            if (nFullLength != bodyLength) {
                switch (formatType) {
                    case CAsterixFormat::EJSON:
                    case CAsterixFormat::EJSONH:
                    case CAsterixFormat::EJSONE: {
                        tmpStr += format(",");
                    }
                }
            }
        }
    }

    if (nFullLength != bodyLength) {
        switch (formatType) {
            case CAsterixFormat::EJSON:
            case CAsterixFormat::EJSONH:
            case CAsterixFormat::EJSONE: {
                if (tmpStr[tmpStr.length() - 1] == ',') {
                    tmpStr[tmpStr.length() - 1] = ']';
                } else {
                    tmpStr += ']';
                }
            }
        }
    }

    strResult += tmpStr;

    return ret;
}

std::string DataItemFormatExplicit::printDescriptors(std::string header) {
    std::string strDef = "";

    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormat *dip = (DataItemFormat *) (*it);
        strDef += dip->printDescriptors(header);
    }
    return strDef;
}

bool DataItemFormatExplicit::filterOutItem(const char *name) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormat *dip = (DataItemFormat *) (*it);
        if (true == dip->filterOutItem(name))
            return true;
    }
    return false;
}

bool DataItemFormatExplicit::isFiltered(const char *name) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormat *dip = (DataItemFormat *) (*it);
        if (true == dip->isFiltered(name))
            return true;
    }
    return false;
}

const char *DataItemFormatExplicit::getDescription(const char *field, const char *value = NULL) {
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
fulliautomatix_definitions* DataItemFormatExplicit::getWiresharkDefinitions()
{
    DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
    if (pFixed == NULL)
    {
        Tracer::Error("Wrong format of explicit item");
        return NULL;
    }
    return pFixed->getWiresharkDefinitions();
}

fulliautomatix_data* DataItemFormatExplicit::getData(unsigned char* pData, long, int byteoffset)
{
    fulliautomatix_data *lastData = NULL, *firstData = NULL;
    DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
    if (pFixed == NULL)
    {
        Tracer::Error("Wrong format of explicit item");
        return NULL;
    }

    // TODO : fix this as it is in DataItemFormatExplicit::getText
    int fixedLength = pFixed->getLength(pData);
    unsigned char nFullLength = *pData;

    firstData = lastData = newDataUL(NULL, PID_LEN, byteoffset, 1, nFullLength);
    byteoffset+=1;

    pData++;

    lastData->next = pFixed->getData(pData, fixedLength, byteoffset);
    while(lastData->next)
        lastData = lastData->next;

    byteoffset += nFullLength;

    return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)

PyObject* DataItemFormatExplicit::getObject(unsigned char* pData, long nLength, int verbose)
{
    std::list<DataItemFormat *>::iterator it;
    int bodyLength = 0;

    pData++; // skip explicit length byte (it is already in nLength)

    // calculate the size of all sub items
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        bodyLength += di->getLength(pData + bodyLength); // calculate length of body
    }

    int nFullLength = nLength - 1; // calculate full length

    // full length must be multiple of body length
    if (bodyLength == 0 || nFullLength % bodyLength != 0) {
        //TODO Tracer::Error("Wrong data length in Explicit. Needed=%d and there is %d bytes.", bodyLength, nFullLength);
        return NULL;
    }

    if (nFullLength == bodyLength) {
        PyObject* p = PyDict_New();
        DataItemFormat *di = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
        if (di != NULL) {
            di->insertToDict(p, pData, bodyLength, verbose);
        }
        return p;
    }
    else {
        PyObject* p = PyList_New(0);
        for (int i = 0; i < nFullLength; i += bodyLength) {
            for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
                DataItemFormat *di = (DataItemFormat *) (*it);
                PyObject* p1 = di->getObject(pData, bodyLength, verbose);
                PyList_Append(p, p1);
                Py_DECREF(p1);
                pData += bodyLength;
            }
        }
        return p;
    }
}

void DataItemFormatExplicit::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
 // Not supported
}
#endif
