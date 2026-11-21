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

#include "DataItemFormatVariable.h"
#include "Tracer.h"
#include "asterixformat.hxx"

DataItemFormatVariable::DataItemFormatVariable(int id)
        : DataItemFormat(id) {
}

DataItemFormatVariable::DataItemFormatVariable(const DataItemFormatVariable &obj)
        : DataItemFormat(obj.m_nID) {
    std::list<DataItemFormat *>::iterator it = ((DataItemFormat &) obj).m_lSubItems.begin();

    while (it != obj.m_lSubItems.end()) {
        DataItemFormat *di = (DataItemFormat *) (*it);
        m_lSubItems.push_back(di->clone());
        it++;
    }

    m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatVariable::~DataItemFormatVariable() {
    // destroy list items
    std::list<DataItemFormat *>::iterator it = m_lSubItems.begin();
    while (it != m_lSubItems.end()) {
        delete (DataItemFormatFixed *) (*it);
        it = m_lSubItems.erase(it);
    }
}

const std::vector<std::string> &DataItemFormatVariable::getJsonPrefixes(int extent) {
    // part at index e uses sub item e, or the last sub item when there are fewer (as getText does)
    while ((int) m_vJsonPrefixes.size() <= extent) {
        int e = (int) m_vJsonPrefixes.size();
        std::list<DataItemFormat *>::iterator it = m_lSubItems.begin();
        for (int i = 0; i < e && it != m_lSubItems.end(); i++) {
            std::list<DataItemFormat *>::iterator next = it;
            if (++next == m_lSubItems.end())
                break;
            it = next;
        }
        if (m_lSubItems.size() == 1)
            m_sJsonKeys.clear(); // each extent is its own object in the list
        m_vJsonPrefixes.push_back(std::vector<std::string>());
        ((DataItemFormatFixed *) (*it))->makeJsonPrefixes(m_sJsonKeys, e, m_vJsonPrefixes.back());
    }
    return m_vJsonPrefixes[extent];
}

long DataItemFormatVariable::getLength(const unsigned char *pData, long nMaxLength) {
    long length = 0;
    std::list<DataItemFormat *>::iterator it;
    bool lastPart = false;
    it = m_lSubItems.begin();

    DataItemFormatFixed *dip = (DataItemFormatFixed *) (*it);

    do {
        long partlen = dip->getLength();
        if (partlen <= 0 || length + partlen > nMaxLength) {
            // the FX chain runs past the available data (isLastPart would read beyond it)
            return nMaxLength + 1;
        }
        lastPart = dip->isLastPart(pData);

        length += partlen;
        pData += partlen;

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = (DataItemFormatFixed *) (*it);
            }
        }
    } while (!lastPart);

    return length;
}

bool DataItemFormatVariable::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType,
                                     unsigned char *pData, long nLength) {
    bool ret = false;

    std::list<DataItemFormat *>::iterator it;
    bool lastPart = false;

    // If Variable item definition contains 1 Fixed subitem, show items in list
    bool listOfSubItems = false;
    if (m_lSubItems.size() == 1)
        listOfSubItems = true;

    it = m_lSubItems.begin();
    int extent = 0;                 // index of the current part, numbers the FX bits in EJSONX

    DataItemFormatFixed *dip = (DataItemFormatFixed *) (*it);

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONX:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            if (listOfSubItems)
                strResult += '[';
            else
                strResult += '{';
        }
            break;
    }

    do {
        lastPart = dip->isLastPart(pData);

        switch (formatType) {
            case CAsterixFormat::EJSON:
            case CAsterixFormat::EJSONX:
            case CAsterixFormat::EJSONH:
            case CAsterixFormat::EJSONE: {
                // the part is written as {...} straight into strResult, then the braces are trimmed in place
                const std::string::size_type partStart = strResult.length();
                if (formatType == CAsterixFormat::EJSONX) {
                    ret |= dip->getText(strResult, strHeader, formatType, pData, dip->getLength(),
                                        getJsonPrefixes(extent));
                } else {
                    ret |= dip->getText(strResult, strHeader, formatType, pData, dip->getLength());
                }
                if (strResult.length() - partStart > 2) { // if result != {}
                    if (!listOfSubItems) { // a list keeps {} around each part
                        strResult.erase(partStart, 1);
                        strResult.erase(strResult.length() - 1);
                    }
                    if (!lastPart) {
                        strResult += ',';
                    }
                } else {
                    strResult.resize(partStart);
                }
            }
                break;
            default:
                ret |= dip->getText(strResult, strHeader, formatType, pData, dip->getLength());
                break;
        }

        pData += dip->getLength();
        nLength -= dip->getLength();
        extent++;

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = (DataItemFormatFixed *) (*it);
            }
        }
    } while (!lastPart && nLength > 0);

    switch (formatType) {
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONX:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            if (strResult[strResult.length() - 1] == ',') {
                if (listOfSubItems)
                    strResult[strResult.length() - 1] = ']';
                else
                    strResult[strResult.length() - 1] = '}';
            } else {
                if (listOfSubItems)
                    strResult += ']';
                else
                    strResult += '}';
            }
        }
            break;
    }

    return ret;
}

std::string DataItemFormatVariable::printDescriptors(std::string header) {
    std::string strDef = "";

    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormatFixed *dip = (DataItemFormatFixed *) (*it);
        strDef += dip->printDescriptors(header);
    }
    return strDef;
}

bool DataItemFormatVariable::filterOutItem(const char *name) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormatFixed *dip = (DataItemFormatFixed *) (*it);
        if (true == dip->filterOutItem(name))
            return true;
    }
    return false;
}

bool DataItemFormatVariable::isFiltered(const char *name) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormatFixed *dip = (DataItemFormatFixed *) (*it);
        if (true == dip->isFiltered(name))
            return true;
    }
    return false;
}

const char *DataItemFormatVariable::getDescription(const char *field, const char *value = NULL) {
    std::list<DataItemFormat *>::iterator it;
    for (it = m_lSubItems.begin(); it != m_lSubItems.end(); it++) {
        DataItemFormatFixed *dip = (DataItemFormatFixed *) (*it);
        const char *desc = dip->getDescription(field, value);
        if (desc != NULL)
            return desc;
    }
    return NULL;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatVariable::getWiresharkDefinitions()
{
  fulliautomatix_definitions *def = NULL, *startDef = NULL;

  std::list<DataItemFormat*>::iterator it;
  for ( it=m_lSubItems.begin() ; it != m_lSubItems.end(); it++ )
  {
    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    if (def)
    {
      def->next = dip->getWiresharkDefinitions();
    }
    else
    {
      startDef = def = dip->getWiresharkDefinitions();
    }
    while(def->next)
      def = def->next;
  }
  return startDef;
}

fulliautomatix_data* DataItemFormatVariable::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;
  std::list<DataItemFormat*>::iterator it;
  bool lastPart = false;

  it=m_lSubItems.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    if (lastData)
    {
      lastData->next = dip->getData(pData, dip->getLength(), byteoffset);
    }
    else
    {
      firstData = lastData = dip->getData(pData, dip->getLength(), byteoffset);
    }
    while(lastData->next)
      lastData = lastData->next;

    byteoffset += dip->getLength();
    pData += dip->getLength();
    len -= dip->getLength();

    if (it != m_lSubItems.end())
    {
      it++;
      if (it != m_lSubItems.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart && len > 0);

  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
PyObject* DataItemFormatVariable::getObject(unsigned char* pData, long nLength, int verbose)
{
    PyObject* p = NULL;
    std::list<DataItemFormat*>::iterator it;
    bool lastPart = false;

    // If Variable item definition contains 1 Fixed subitem, show items in list
    bool listOfSubItems = false;
    if (m_lSubItems.size() == 1) {
        listOfSubItems = true;
        p = PyList_New(0);
    } else {
        // otherwise put directly to dictionary
        p = PyDict_New();
    }

    it=m_lSubItems.begin();
    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    do
    {
        lastPart = dip->isLastPart(pData);

        if (listOfSubItems) {
            PyObject* p1 = PyDict_New();
            dip->insertToDict(p1, pData, dip->getLength(), verbose);
            PyList_Append(p, p1);
            Py_DECREF(p1);
        }
        else {
            dip->insertToDict(p, pData, dip->getLength(), verbose);
        }

        pData += dip->getLength();
        nLength -= dip->getLength();

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = (DataItemFormatFixed*)(*it);
            }
        }
    }
    while(!lastPart && nLength > 0);

    return p;
}

void DataItemFormatVariable::insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose)
{
    return;
    /*
    std::list<DataItemFormat*>::iterator it;
    bool lastPart = false;

    it=m_lSubItems.begin();

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

    do
    {
        lastPart = dip->isLastPart(pData);

        dip->insertToDict(p, pData, dip->getLength(), verbose);

        pData += dip->getLength();
        nLength -= dip->getLength();

        if (it != m_lSubItems.end()) {
            it++;
            if (it != m_lSubItems.end()) {
                dip = (DataItemFormatFixed*)(*it);
            }
        }
    }
    while(!lastPart && nLength > 0);
     */
}
#endif
