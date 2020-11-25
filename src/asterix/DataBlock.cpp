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

#include "DataBlock.h"
#include "Tracer.h"
#include "Utils.h"
#include "asterixformat.hxx"

extern bool gFiltering;

DataBlock::DataBlock(Category *cat, unsigned long len, const unsigned char *data, unsigned long nTimestamp)
        : m_pCategory(cat), m_nLength(len), m_nTimestamp(nTimestamp), m_bFormatOK(false) {
    const unsigned char *m_pItemDataStart = data;
    long nUnparsed = len;
    int counter = 1;

    if (gFiltering && !m_pCategory->m_bFiltered) {
        m_bFormatOK = true;
        return;
    }

    while (nUnparsed > 0) {
        DataRecord *dr = new DataRecord(cat, counter++, nUnparsed, m_pItemDataStart, (unsigned long) nTimestamp);

        if (!dr) {
            Tracer::Error("Error DataBlock format.");
            break;
        }

        m_lDataRecords.push_back(dr);

        if (dr->m_nLength <= 0) {
            Tracer::Error("Wrong length in DataBlock format.");
            break;
        }

        m_pItemDataStart += dr->m_nLength;
        nUnparsed -= dr->m_nLength;
    }

    if (nUnparsed > 0) {
        m_nLength -= nUnparsed;
    }

    m_bFormatOK = true;
}

DataBlock::~DataBlock() {
    // go through all present data items in this block
    std::list<DataRecord *>::iterator it = m_lDataRecords.begin();
    while (it != m_lDataRecords.end()) {
        delete (DataRecord *) (*it);
        it = m_lDataRecords.erase(it);
    }
}

bool DataBlock::getText(std::string &strResult, const unsigned int formatType) {
    std::string strHeader;

    if (gFiltering && !m_pCategory->m_bFiltered) {
        return false;
    }

    switch (formatType) {
        case CAsterixFormat::ETxt:
            strResult += format("\nCategory: %d", m_pCategory->m_id);
            strResult += format("\nLen: %ld", m_nLength);
            strResult += format("\nHexData: %02X%02X%02X", m_pCategory->m_id, ((m_nLength + 3) >> 8) & 0xff, (m_nLength + 3) & 0xff);
            break;
        case CAsterixFormat::EOut:
            strHeader = format("Asterix.CAT%03d", m_pCategory->m_id);
            break;
    }

    if (!m_bFormatOK) {
        Tracer::Error("Block not parsed properly.");
        return true;
    }

    if (m_lDataRecords.size() > 0) {
        // go through all present data items in this block
        std::list<DataRecord *>::iterator it;
        for (it = m_lDataRecords.begin(); it != m_lDataRecords.end(); it++) {
            DataRecord *dr = (DataRecord *) (*it);
            if (dr != NULL) {
                dr->getText(strResult, strHeader, formatType);
            }
        }
    }
    return true;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_data* DataBlock::getData(int byteoffset)
{
  fulliautomatix_data *firstData = NULL, *lastData=NULL;
  int endOffset = byteoffset+m_nLength+3;
  char tmp[256];
  snprintf(tmp, 256, "Data Block Cat%03d - %.200s", m_pCategory->m_id, m_pCategory->m_strName.c_str());

  firstData = lastData = newDataTree(NULL, byteoffset, m_nLength+3, tmp);

  lastData = newDataUL(lastData, PID_CATEGORY, byteoffset, 1, m_pCategory->m_id);
  byteoffset+=1;
  lastData = newDataUL(lastData, PID_LENGTH, byteoffset, 2, m_nLength+3);
  byteoffset+=2;

  // go through all present data items in this block
  std::list<DataRecord*>::iterator it;
  for ( it=m_lDataRecords.begin() ; it != m_lDataRecords.end(); it++ )
  {
    DataRecord* dr = (DataRecord*)(*it);
    if (dr != NULL)
    {
      lastData->next = dr->getData(byteoffset);
      while(lastData->next)
        lastData = lastData->next;

      byteoffset = lastData->bytenr;
    }
  }

  lastData = newDataTreeEnd(lastData,endOffset);
  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
void DataBlock::getData(PyObject* plist, int verbose)
{
    // go through all present data items in this block and insert them to list
    std::list<DataRecord*>::iterator it;
    for ( it=m_lDataRecords.begin() ; it != m_lDataRecords.end(); it++ )
    {
        DataRecord* dr = (DataRecord*)(*it);
        if (dr != NULL)
        {
            PyObject* p = dr->getData(verbose);
            PyList_Append(plist, p);
            Py_DECREF(p);
        }
    }
}
#endif
