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

#include "DataItem.h"
#include "Tracer.h"
#include "Utils.h"
#include "asterixformat.hxx"

DataItem::DataItem(DataItemDescription *pDesc)
        : m_pDescription(pDesc), m_pData(NULL), m_nLength(0) {
}

DataItem::~DataItem() {
    if (m_pData)
        delete[] m_pData;
}

bool DataItem::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType) {
    std::string newHeader;
    std::string strNewResult;
    std::string indent("    ");  // 4 spaces make an indent.

    switch (formatType) {
        case CAsterixFormat::EJSON:
            strNewResult = format("\"I%s\":", m_pDescription->m_strID.c_str());
            break;
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE:
            strNewResult = format("\t\"I%s\":", m_pDescription->m_strID.c_str());
            break;
        case CAsterixFormat::EXML:
            strNewResult = format("<I%s>", m_pDescription->m_strID.c_str());
            break;
        case CAsterixFormat::EXMLH:
            strNewResult = format("\n%s", indent.c_str());  // New line and indent 1 level (4 spaces).
            strNewResult += format("<I%s>", m_pDescription->m_strID.c_str());
            break;
        case CAsterixFormat::ETxt:
            strNewResult = format("\n\nItem %s : %s", m_pDescription->m_strID.c_str(),
                                  m_pDescription->m_strName.c_str());
            strNewResult += format("\n[ ");
            for (int i = 0; i < m_nLength; i++) {
                strNewResult += format("%02X ", *(m_pData + i));
            }
            strNewResult += format("]");
            break;
        case CAsterixFormat::EOut:
            newHeader = format("%s.%s", strHeader.c_str(), m_pDescription->m_strID.c_str());
            break;
    }

    if (false == m_pDescription->getText(strNewResult, newHeader, formatType, m_pData, m_nLength)) {
        return false;
    }
    strResult += strNewResult;

    switch (formatType) {
        case CAsterixFormat::EXML:
            strResult += format("</I%s>", m_pDescription->m_strID.c_str());
            break;
        case CAsterixFormat::EXMLH:
            strResult += format("\n%s", indent.c_str());  // New line and indent 1 levels (4 spaces).
            strResult += format("</I%s>", m_pDescription->m_strID.c_str());
            break;
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE:
            // replace last ',' with '}'
            if (strResult[strResult.length() - 1] == ',') {
                strResult[strResult.length() - 1] = '}';
            }
            break;
    }

    return true;
}

long DataItem::parse(const unsigned char *pData, long len) {
    if (m_pDescription == NULL || m_pDescription->m_pFormat == NULL) {
        Tracer::Error("DataItem::parse NULL pointer");
        return 0;
    }

    m_nLength = m_pDescription->m_pFormat->getLength(pData);

    if (m_nLength > len) {
        // Print unparsed bytes
        std::string strNewResult;
        for (int i = 0; i < len; i++) {
            strNewResult += format("%02X ", *(pData + i));
        }
        Tracer::Error("DataItem::parse needed length=%d , and there is only %d : [ %s ]", m_nLength, len,
                      strNewResult.c_str());
    } else if (m_nLength > 0) {
        m_pData = new unsigned char[m_nLength];
        memcpy(m_pData, pData, m_nLength);
    } else {
        Tracer::Error("DataItem::parse length=0");
    }
    return m_nLength;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_data* DataItem::getData(int byteoffset)
{
  fulliautomatix_data *firstData, *lastData;
  std::string strDesc;
  char tmp[20];
  snprintf(tmp, 20, "Data item %s - ", m_pDescription->m_strID.c_str());
  strDesc = tmp;
  strDesc += m_pDescription->m_strName;
  lastData = firstData = newDataTree(NULL, byteoffset, m_nLength, (char*)strDesc.c_str());
  if (m_pDescription && m_pDescription->m_pFormat && m_pData)
  {
    lastData->next = m_pDescription->m_pFormat->getData(m_pData, m_nLength, byteoffset);
  }
  else
  {
    lastData->next = newDataMessage(NULL, byteoffset, m_nLength, 2, (char*)"Error: Unknown item format.");
  }
  while (lastData->next)
  {
    if (lastData->next->err)
      firstData->err = lastData->next->err;
    lastData = lastData->next;
  }
  byteoffset += m_nLength;
  lastData = newDataTreeEnd(lastData,byteoffset);
  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
PyObject* DataItem::getData(int verbose)
{
  if (m_pDescription && m_pDescription->m_pFormat && m_pData)
  {
    return m_pDescription->m_pFormat->getObject(m_pData, m_nLength, verbose);
  }
  return Py_BuildValue("s", "Error");
}
#endif
