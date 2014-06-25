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

DataItemFormatExplicit::DataItemFormatExplicit()
{
}

DataItemFormatExplicit::~DataItemFormatExplicit()
{
}

long DataItemFormatExplicit::getLength(const unsigned char* pData)
{
	return (long)(*pData);
}

void DataItemFormatExplicit::addBits(DataItemBits* pBits)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Explicit");
		return;
	}
	pFixed->addBits(pBits);
}

bool DataItemFormatExplicit::getText(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long nLength)
{
	bool ret = false;

	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Explicit");
		return false;
	}

	int fixedLength = pFixed->getLength(pData);
	unsigned char nFullLength = nLength - 1;
	pData++;

	switch(formatType)
	{
	case CAsterixFormat::EJSON:
	case CAsterixFormat::EJSONH:
	{
		std::string tmpStr = format("[");

		for (int i=0; i<nFullLength; i+=fixedLength)
		{
			pFixed->getText(tmpStr, strHeader, formatType, pData, fixedLength);
			pData += fixedLength;
			tmpStr += format(",");
		}
		if (tmpStr[tmpStr.length()-1] == ',')
		{
			tmpStr[tmpStr.length()-1] = ']';
		}
		else
		{
			tmpStr += ']';
		}

		if (ret)
			strResult += tmpStr;
		break;
	}
	default:
	{
		for (int i=0; i<nFullLength; i+=fixedLength)
		{
			pFixed->getText(strResult, strHeader, formatType, pData, fixedLength);
			pData += fixedLength;
		}
		break;
	}
	}

	return ret;
}

std::string DataItemFormatExplicit::printDescriptors(std::string header)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Explicit");
		return "Wrong data in Explicit";
	}
	return pFixed->printDescriptors(header);
}

bool DataItemFormatExplicit::filterOutItem(const char* name)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Explicit");
		return false;
	}
	return pFixed->filterOutItem(name);
}

bool DataItemFormatExplicit::isFiltered(const char* name)
{
	DataItemFormatFixed* pFixed = m_lSubItems.size() ? (DataItemFormatFixed*)m_lSubItems.front() : NULL;
	if (pFixed == NULL)
	{
		Tracer::Error("Wrong data in Explicit");
		return false;
	}
	return pFixed->isFiltered(name);
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
