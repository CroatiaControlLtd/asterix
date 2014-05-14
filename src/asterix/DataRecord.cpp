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

#include "../main/asterix.h"
#include "Category.h"
#include "DataRecord.h"
#include "Tracer.h"
#include "Utils.h"
#include "asterixformat.hxx"

DataRecord::DataRecord(Category* cat, int nID, unsigned long len, const unsigned char* data, unsigned long nTimestamp)
: m_pCategory(cat)
, m_nID(nID)
, m_nLength(len)
, m_nFSPECLength(0)
, m_pFSPECData(NULL)
, m_nTimestamp(nTimestamp)
, m_bFormatOK(false)
{
	const unsigned char* m_pItemDataStart = data;
	long nUnparsed = len;

	UAP* pUAP = m_pCategory->getUAP(data, len);
	if (!pUAP)
	{
		Tracer::Error("UAP not found for category %d", m_pCategory->m_id);
		return;
	}

	// parse FSPEC
	unsigned int nFSPEC = 0;
	int nFRN = 1;
	bool lastFSPEC = false;
	do
	{
		unsigned bitmask = 0x80;
		unsigned char FSPEC = *m_pItemDataStart;
		lastFSPEC = (FSPEC & 0x01) ? false : true;

		while(bitmask > 1)
		{
			if ( FSPEC & bitmask)
			{
				DataItemDescription *dataitemdesc = m_pCategory->getDataItemDescription(pUAP->getDataItemIDByUAPfrn(nFRN));
				if (dataitemdesc)
				{
					DataItem* di = new DataItem(dataitemdesc);
					m_lDataItems.push_back(di);
				}
				else
				{
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
	}
	while( !lastFSPEC && nUnparsed>0);

	m_pFSPECData = (unsigned char*)malloc(m_nFSPECLength);
	memcpy(m_pFSPECData, data, m_nFSPECLength);

	if (nUnparsed < 0)
	{
		Tracer::Error("Wrong FSPEC in data block");
		return;
	}

	bool errorReported = false;

	// parse DataItems
	std::list<DataItem*>::iterator it;
	for ( it=m_lDataItems.begin() ; it != m_lDataItems.end(); it++ )
	{
		DataItem* di = (DataItem*)(*it);

		if (di->m_pDescription == NULL || di->m_pDescription->m_pFormat == NULL)
		{
			Tracer::Error("DataItem format not defined for CAT%03d/I%03d", cat->m_id, di->m_pDescription->m_nID);
			errorReported = true;
			break;
		}

		long usedbytes = di->parse(m_pItemDataStart, nUnparsed);
		if (usedbytes <= 0 || usedbytes > nUnparsed)
		{
			Tracer::Error("Wrong length in DataItem format for CAT%03d/I%03d", cat->m_id, di->m_pDescription->m_nID);
			errorReported = true;
			break;
		}

		m_pItemDataStart += usedbytes;
		nUnparsed -= usedbytes;
	}

	if (nUnparsed > 0)
	{
		//	  Tracer::Error("Unparsed bytes: %ld", nUnparsed );
		m_nLength -= nUnparsed;
	}

	if (it != m_lDataItems.end())
	{
		if (errorReported == false)
		{
			Tracer::Error("Not enough data in record for CAT%03d", cat->m_id );
		}

		while(it != m_lDataItems.end())
		{
			DataItem* di = (DataItem*)(*it);
			delete di;
			it = m_lDataItems.erase(it);
		}
	}
	else
	{
		m_bFormatOK = true;
	}
}

DataRecord::~DataRecord()
{
	// go through all present data items in this block
	std::list<DataItem*>::iterator it = m_lDataItems.begin();
	while(it != m_lDataItems.end())
	{
		delete (DataItem*)(*it);
		it = m_lDataItems.erase(it);
	}

	if (m_pFSPECData)
		free(m_pFSPECData);
}

bool DataRecord::getText(std::string& strResult, std::string& strHeader, const unsigned int formatType)
{
	if (!m_bFormatOK)
	{
		Tracer::Error("Record not parsed properly. CAT%03d len=%ld", m_pCategory->m_id, m_nLength);
		return false;
	}

	std::string strNewResult;

	switch(formatType)
	{
	case CAsterixFormat::ETxt:
		strNewResult = format("\n-------------------------\nData Record %d", m_nID);
		strNewResult += format("\nLen: %ld", m_nLength);
		break;
	case CAsterixFormat::EJSON:
		strNewResult = format("{\"CAT%03d\":{", m_pCategory->m_id);
		break;
	case CAsterixFormat::EJSONH:
		strNewResult = format("{\"CAT%03d\":{\n", m_pCategory->m_id);
		break;
	case CAsterixFormat::EXIDEF:
		const int nXIDEFv = 1;
		strNewResult = format("\n<ASTERIX ver=\"%d\" cat=\"%d\">", nXIDEFv, m_pCategory->m_id);
		break;
	}

	// go through all present data items in this block
	bool ret = false;

	std::list<DataItem*>::iterator it;
	for ( it=m_lDataItems.begin() ; it != m_lDataItems.end(); it++ )
	{
		DataItem* di = (DataItem*)(*it);
		if (di != NULL)
		{
			if (di->getText(strNewResult, strHeader, formatType))
			{
				if (!ret)
				{
					ret = true;
				}
				else
				{
					switch(formatType)
					{
					case CAsterixFormat::EJSON:
						strResult += ",";
						break;
					case CAsterixFormat::EJSONH:
						strResult += ",\n";
						break;
					}
				}
				strResult += strNewResult;
				strNewResult = "";
			}
		}
	}

	if (ret)
	{
		switch(formatType)
		{
		case CAsterixFormat::EJSON:
		case CAsterixFormat::EJSONH:
			strResult += "}},\n";
			break;
		case CAsterixFormat::EXIDEF:
			strResult += "\n</ASTERIX>";
			break;
		}
	}

	return ret;
}

DataItem* DataRecord::getItem(int itemid)
{
	// go through all present data items in this block
	std::list<DataItem*>::iterator it;
	for ( it=m_lDataItems.begin() ; it != m_lDataItems.end(); it++ )
	{
		DataItem* di = (DataItem*)(*it);
		if (di && di->m_pDescription && di->m_pDescription->m_nID == itemid)
		{
			return di;
		}
	}
	return NULL;
}

bool DataRecord::getValue(int itemid, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
	DataItem* di = getItem(itemid);
	if (di)
	{
		return di->getValue(value, di->getLength(), pstrBitsShortName, pstrBitsName);
	}
	return false;
}

bool DataRecord::getValue(int itemid, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
	DataItem* di = getItem(itemid);
	if (di)
	{
		return di->getValue(value, di->getLength(), pstrBitsShortName, pstrBitsName);
	}
	return false;
}

bool DataRecord::getValue(int itemid, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
	DataItem* di = getItem(itemid);
	if (di)
	{
		return di->getValue(value, di->getLength(), pstrBitsShortName, pstrBitsName);
	}
	return false;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_data* DataRecord::getData(int byteoffset)
{
	fulliautomatix_data *firstData = NULL, *lastData=NULL;
	int endOffset = byteoffset+m_nLength;

	char tmp[64];
	sprintf(tmp, "Data Record %d - %ld bytes", m_nID, m_nLength);

	firstData = lastData = newDataTree(lastData, byteoffset, m_nLength, tmp);

	UAP* pUAP = m_pCategory->getUAP(m_pFSPECData, m_nFSPECLength);
	if (!pUAP)
	{
		Tracer::Error("UAP for CAT%03d not found!", m_pCategory->m_id);
		firstData->err = 2;
	}
	else
	{
		lastData = newDataTree(lastData, byteoffset, m_nFSPECLength, "FSPEC");

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
