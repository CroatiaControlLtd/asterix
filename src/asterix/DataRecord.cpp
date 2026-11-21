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

#include "Category.h"
#include "DataRecord.h"
#include "Tracer.h"
#include "Utils.h"
#include "asterixformat.hxx"
#include <vector>

extern bool gFiltering;

DataRecord::DataRecord(Category *cat, int nID, unsigned long len, const unsigned char *data, double nTimestamp, unsigned int nIP)
        : m_pCategory(cat), m_nID(nID), m_nLength(len), m_nFSPECLength(0), m_pFSPECData(NULL), m_nTimestamp(nTimestamp),
          m_nIP(nIP), m_nCrc(0), m_pHexData(NULL), m_bFormatOK(false) {
    const unsigned char *m_pItemDataStart = data;
    long nUnparsed = len;

    UAP *pUAP = m_pCategory->getUAP(data, len);
    if (!pUAP) {
        Tracer::Error("UAP not found for category %d", m_pCategory->m_id);
        return;
    }

    // parse FSPEC: descriptions of the items present, in record order
    std::vector<DataItemDescription *> itemDescs;
    itemDescs.reserve(16);
    unsigned int nFSPEC = 0;
    int nFRN = 1;
    bool lastFSPEC = false;
    do {
        unsigned bitmask = 0x80;
        unsigned char FSPEC = *m_pItemDataStart;
        lastFSPEC = (FSPEC & 0x01) ? false : true;

        while (bitmask > 1) {
            if (FSPEC & bitmask) {
                DataItemDescription *dataitemdesc = pUAP->getDataItemDescriptionByUAPfrn(m_pCategory, nFRN);
                if (dataitemdesc) {
                    itemDescs.push_back(dataitemdesc);
                } else {
                    Tracer::Error("Description of UAP FRN %d in category %03d not found", nFRN, m_pCategory->m_id);
                    return;
                }
            }
            bitmask >>= 1;
            bitmask &= 0x7F;
            nFRN++;
        }

        m_pItemDataStart++;
        nFSPEC++;
        m_nFSPECLength++;
        nUnparsed--;
    } while (!lastFSPEC && nUnparsed > 0);

    m_pFSPECData = (unsigned char *) malloc(m_nFSPECLength);
    memcpy(m_pFSPECData, data, m_nFSPECLength);

    if (nUnparsed < 0) {
        Tracer::Error("Wrong FSPEC in data block");
        return;
    }

    bool errorReported = false;

    // parse DataItems. When filtering (-LF), an item with no field in the filter prints nothing: only its
    // length is read, to step over it, and no DataItem (nor copy of its bytes) is made.
    size_t nItem;
    for (nItem = 0; nItem < itemDescs.size(); nItem++) {
        DataItemDescription *desc = itemDescs[nItem];

        if (desc->m_pFormat == NULL) {
            Tracer::Error("DataItem format not defined for CAT%03d/I%s", cat->m_id, desc->m_strID.c_str());
            errorReported = true;
            break;
        }

        long usedbytes;
        if (gFiltering && !desc->m_bFiltered) {
            usedbytes = DataItem::getLength(desc, m_pItemDataStart, nUnparsed);
        } else {
            DataItem *di = new DataItem(desc);
            di->m_nOffset = (long) (m_pItemDataStart - data) - (long) m_nFSPECLength;
            usedbytes = di->parse(m_pItemDataStart, nUnparsed);
            if (usedbytes <= 0 || usedbytes > nUnparsed) {
                delete di;
            } else {
                m_lDataItems.push_back(di);
            }
        }
        if (usedbytes <= 0 || usedbytes > nUnparsed) {
            Tracer::Error("Wrong length in DataItem format for CAT%03d/I%s", cat->m_id, desc->m_strID.c_str());
            errorReported = true;
            break;
        }

        m_pItemDataStart += usedbytes;
        nUnparsed -= usedbytes;
    }

    if (nUnparsed > 0) {
//	  Tracer::Error("Unparsed bytes: %ld", nUnparsed );
        m_nLength -= nUnparsed;
    }

    if (nItem < itemDescs.size()) {
        if (errorReported == false) {
            Tracer::Error("Not enough data in record for CAT%03d", cat->m_id);
        }
    } else {
        m_bFormatOK = true;
    }

    if (!m_bFormatOK) {
        // Print whole record in case of error
        std::string strNewResult = format("Data Record bytes: [ ");
        for (unsigned int i = 0; i < len; i++) {
            strNewResult += format("%02X ", *(data + i));
        }
        strNewResult += format("]");
        Tracer::Error("%s", strNewResult.c_str());
    } else {
        unsigned int i;
        m_nCrc = crc32(data, m_nLength, 1);

        // build hexdata string
        m_pHexData = (char *) calloc(m_nLength  * 2 + 1 /* null */, sizeof(char));
        for (i = 0; i < m_nLength; i++) {
            m_pHexData[i * 2] = HEXDIGITS_UPPER[data[i] >> 4];
            m_pHexData[i * 2 + 1] = HEXDIGITS_UPPER[data[i] & 0x0F];
        }
    }
}

DataRecord::~DataRecord() {
    // go through all present data items in this block
    std::list<DataItem *>::iterator it = m_lDataItems.begin();
    while (it != m_lDataItems.end()) {
        delete (DataItem *) (*it);
        it = m_lDataItems.erase(it);
    }

    if (m_pFSPECData)
        free(m_pFSPECData);

    if (m_pHexData)
        free(m_pHexData);
}

bool DataRecord::getText(std::string &strResult, std::string &strHeader, const unsigned int formatType) {
    if (!m_bFormatOK) {
        Tracer::Error("Record not parsed properly. CAT%03d len=%ld", m_pCategory->m_id, m_nLength);
        return false;
    }
    unsigned char ip[4];
    ip[0] = (unsigned char)(m_nIP >> 24);
    ip[1] = (unsigned char)(m_nIP >> 16);
    ip[2] = (unsigned char)(m_nIP >> 8);
    ip[3] = (unsigned char)(m_nIP);

    // The record is written straight into strResult; if no item has anything to print, it is cut off again.
    const std::string::size_type recordStart = strResult.length();
    std::string &strNewResult = strResult;

    // Text and JSON outputs print the FSPEC apart ("Fspec" / "fspec", from the FSPEC bytes stored when the
    // record was parsed, m_pFSPECData) and "HexData" / "hexdata" holds only the data items after it, so the
    // record is fspec + hexdata. In EJSONX, field "pos" values are byte offsets in this hexdata (its first
    // byte is pos 0). XML keeps the whole record in hexdata.
    std::string strFSPEC;
    const char *pItemsHex = m_pHexData;
    switch (formatType) {
        case CAsterixFormat::ETxt:
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONX:
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE: {
            appendHexBytes(strFSPEC, m_pFSPECData, m_nFSPECLength);
            size_t hexLen = strlen(m_pHexData);
            pItemsHex = m_pHexData + (strFSPEC.length() < hexLen ? strFSPEC.length() : hexLen);
            break;
        }
    }

    switch (formatType) {
        case CAsterixFormat::ETxt:
            strNewResult += format("\n-------------------------\nData Record %d", m_nID);
            strNewResult += format("\nLen: %ld", m_nLength);
            strNewResult += format("\nCRC: %08X", m_nCrc);
            strNewResult += format("\nSource IP: %u.%u.%u.%u", ip[0], ip[1], ip[2], ip[3]);
            strNewResult += "\nFspec: ";
            strNewResult += strFSPEC;
            strNewResult += "\nHexData: ";
            strNewResult += pItemsHex;
            break;
        case CAsterixFormat::EJSON:
        case CAsterixFormat::EJSONX: {
            // same text as format("{\"id\":%d,\"cat\":%d,\"length\":%ld,\"crc\":\"%08X\",\"timestamp\":%lf,
            // \"srcip\":\"%u.%u.%u.%u\",\"fspec\":\"...\",\"hexdata\":\"...\",\"CAT%03d\":{"), written without printf
            // because it is done for every record
            strNewResult.append("{\"id\":", 6);
            appendLong(strNewResult, m_nID);
            strNewResult.append(",\"cat\":", 7);
            appendLong(strNewResult, m_pCategory->m_id);
            strNewResult.append(",\"length\":", 10);
            appendLong(strNewResult, (long) m_nLength);
            strNewResult.append(",\"crc\":\"", 8);
            appendHex32(strNewResult, m_nCrc);
            strNewResult.append("\",\"timestamp\":", 14);
            appendDoubleFixed(strNewResult, m_nTimestamp, 6);
            strNewResult.append(",\"srcip\":\"", 10);
            appendIPv4(strNewResult, m_nIP);
            strNewResult.append("\",\"fspec\":\"", 11);
            strNewResult += strFSPEC;
            strNewResult.append("\",\"hexdata\":\"", 13);
            strNewResult += pItemsHex;
            strNewResult.append("\",\"CAT", 6);
            appendInt03(strNewResult, m_pCategory->m_id);
            strNewResult.append("\":{", 3);
            break;
        }
        case CAsterixFormat::EJSONH:
        case CAsterixFormat::EJSONE:
            strNewResult += format(
                    "{\"id\":%d,\n\"cat\":%d,\n\"length\":%ld,\n\"crc\":\"%08X\",\n\"timestamp\":%lf,\n\"srcip\":\"%u.%u.%u.%u\",\n\"fspec\":\"",
                    m_nID, m_pCategory->m_id, m_nLength, m_nCrc, m_nTimestamp, ip[0], ip[1], ip[2], ip[3]);
            strNewResult += strFSPEC;
            strNewResult += "\",\n\"hexdata\":\"";
            strNewResult += pItemsHex;
            strNewResult += format("\",\n\"CAT%03d\":{\n", m_pCategory->m_id);
            break;
        case CAsterixFormat::EXML:
        case CAsterixFormat::EXMLH: {
            const int nXIDEFv = 1;
            strNewResult += format(
                    "<ASTERIX ver=\"%d\" cat=\"%d\" length=\"%ld\" crc=\"%08X\" timestamp=\"%lf\" srcip=\"%u.%u.%u.%u\" hexdata=\"%s\">",
                    nXIDEFv, m_pCategory->m_id, m_nLength, m_nCrc, m_nTimestamp, ip[0], ip[1], ip[2], ip[3], m_pHexData);
            break;
        }
    }

    // go through all present data items in this block
    bool ret = false;

    std::list<DataItem *>::iterator it;
    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        DataItem *di = (DataItem *) (*it);
        if (di != NULL) {
            const std::string::size_type itemStart = strResult.length();
            if (ret) {
                // separator from the previous item, removed below if this item prints nothing
                switch (formatType) {
                    case CAsterixFormat::EJSON:
                    case CAsterixFormat::EJSONX:
                        strResult += ",";
                        break;
                    case CAsterixFormat::EJSONH:
                    case CAsterixFormat::EJSONE:
                        strResult += ",\n";
                        break;
                }
            }
            if (di->getText(strNewResult, strHeader, formatType)) {
                ret = true;
            } else {
                strResult.resize(itemStart);
            }
        }
    }

    if (!ret) {
        strResult.resize(recordStart);
    }

    if (ret) {
        switch (formatType) {
            case CAsterixFormat::EJSON:
            case CAsterixFormat::EJSONX:
                strResult += "}}\n";
                break;
            case CAsterixFormat::EJSONH:
            case CAsterixFormat::EJSONE:
                strResult += "}},\n";
                break;
            case CAsterixFormat::EXML:
                strResult += "</ASTERIX>\n";
                break;
            case CAsterixFormat::EXMLH:
                strResult += "\n</ASTERIX>\n";
                break;
        }
    }

    return ret;
}

DataItem *DataRecord::getItem(std::string itemid) {
    // go through all present data items in this block
    std::list<DataItem *>::iterator it;
    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        DataItem *di = (DataItem *) (*it);
        if (di && di->m_pDescription && di->m_pDescription->m_strID == itemid) {
            return di;
        }
    }
    return NULL;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_data* DataRecord::getData(int byteoffset)
{
  fulliautomatix_data *firstData = NULL, *lastData=NULL;
  int endOffset = byteoffset+m_nLength;

  char tmp[64];
  snprintf(tmp, 64, "Data Record %d - %ld bytes", m_nID, m_nLength);

  firstData = lastData = newDataTree(lastData, byteoffset, m_nLength, tmp);

  UAP* pUAP = m_pCategory->getUAP(m_pFSPECData, m_nFSPECLength);
  if (!pUAP)
  {
    Tracer::Error("UAP for CAT%03d not found!", m_pCategory->m_id);
    firstData->err = 2;
  }
  else
  {
    lastData = newDataTree(lastData, byteoffset, m_nFSPECLength, (char*)"FSPEC");

    // go through all UAPitems items in this record
    std::list<UAPItem*>::iterator uapit;
    for ( uapit=pUAP->m_lUAPItems.begin(); uapit != pUAP->m_lUAPItems.end(); uapit++ )
    {
      UAPItem* uap = (UAPItem*)(*uapit);
      lastData->next = uap->getData(m_pFSPECData, m_nFSPECLength, byteoffset);

      if(lastData->next)
      {
        lastData = lastData->next;
      }
      else
      {
        break;
      }
    }
    byteoffset += m_nFSPECLength;

    lastData = newDataTreeEnd(lastData, byteoffset);

    // go through all present data items in this record
    std::list<DataItem*>::iterator it;
    for ( it=m_lDataItems.begin() ; it != m_lDataItems.end(); it++ )
    {
      DataItem* di = (DataItem*)(*it);
      if (di)
      {
        lastData->next = di->getData(byteoffset);
        while(lastData->next)
        {
          if (lastData->next->err)
          {
            firstData->err = lastData->next->err;
          }
          lastData = lastData->next;
        }
        byteoffset = lastData->bytenr;
      }
    }
  }

  if (!m_bFormatOK)
  {
    firstData->err = 2;
  }

  lastData = newDataTreeEnd(lastData,endOffset);
  return firstData;
}
#endif

#if defined(PYTHON_WRAPPER)
PyObject* DataRecord::getData(int verbose)
{
    PyObject*p = PyDict_New();

    PyObject* k1 = Py_BuildValue("s", "category");
    PyObject* v1 = Py_BuildValue("H", m_pCategory->m_id);
    PyDict_SetItem(p, k1, v1);
    Py_DECREF(k1);
    Py_DECREF(v1);

    PyObject* k2 = Py_BuildValue("s", "len");
    PyObject* v2 = Py_BuildValue("l", m_nLength);
    PyDict_SetItem(p, k2, v2);
    Py_DECREF(k2);
    Py_DECREF(v2);

    char hexcrc[9];
    sprintf(hexcrc, "%08X", m_nCrc);
    PyObject* k3 = Py_BuildValue("s", "crc");
    PyObject* v3 = Py_BuildValue("s", hexcrc);
    PyDict_SetItem(p, k3, v3);
    Py_DECREF(k3);
    Py_DECREF(v3);

    PyObject* k4 = Py_BuildValue("s", "ts");
    PyObject* v4 = Py_BuildValue("d", m_nTimestamp);
    PyDict_SetItem(p, k4, v4);
    Py_DECREF(k4);
    Py_DECREF(v4);

    PyObject* k5 = Py_BuildValue("s", "hexdata");
    PyObject* v5 = Py_BuildValue("s", m_pHexData);
    PyDict_SetItem(p, k5, v5);
    Py_DECREF(k5);
    Py_DECREF(v5);

    if (m_bFormatOK)
    {
        // go through all present data items in this record
        std::list<DataItem*>::iterator it;
        for ( it=m_lDataItems.begin() ; it != m_lDataItems.end(); it++ )
        {
            DataItem* di = (DataItem*)(*it);
            if (di)
            {
                PyObject* v1 = di->getData(verbose);
                if (v1 == NULL) {
                    v1 = Py_BuildValue("s", "Error");
                }
                char tmp[20];
                snprintf(tmp, 20, "I%s", di->m_pDescription->m_strID.c_str());
                PyObject* k1 = Py_BuildValue("s", tmp);
                PyDict_SetItem(p, k1, v1);
                Py_DECREF(k1);
                Py_DECREF(v1);
            }
        }
    }

  return p;
}
#endif
