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
: m_pFixed(NULL)
{
}

DataItemFormatExplicit::~DataItemFormatExplicit()
{
  if (m_pFixed)
    delete m_pFixed;
}

long DataItemFormatExplicit::getLength(const unsigned char* pData)
{
  return (long)(*pData);
}

void DataItemFormatExplicit::addBits(DataItemBits* pBits)
{
  if (m_pFixed == NULL)
  {
    Tracer::Error("Wrong data in Explicit");
    return;
  }
  m_pFixed->m_lBits.push_back(pBits);
}

bool DataItemFormatExplicit::get(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long nLength)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nFullLength = nLength - 1;
  pData++;


  switch(formatType)
  {
	  case CAsterixFormat::EJSON:
	  case CAsterixFormat::EJSONH:
		  strResult += format("[");

		  for (int i=0; i<nFullLength; i+=fixedLength)
		  {
			  m_pFixed->get(strResult, strHeader, formatType, pData, fixedLength);
			  pData += fixedLength;
			  strResult += format(",");
		  }
		  if (strResult[strResult.length()-1] == ',')
		  {
			  strResult[strResult.length()-1] = ']';
		  }
		  else
		  {
			  strResult += ']';
		  }
		  break;
	  default:
		  for (int i=0; i<nFullLength; i+=fixedLength)
		  {
			  m_pFixed->get(strResult, strHeader, formatType, pData, fixedLength);
			  pData += fixedLength;
		  }
		  break;
  	  }

  return true;
}

bool DataItemFormatExplicit::getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nFullLength = nLength - 1;
  pData++;

  for (int i=0; i<nFullLength; i+=fixedLength)
  {
	  if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
		  return true;
	  pData += fixedLength;
  }
  return false;
}

bool DataItemFormatExplicit::getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nFullLength = nLength - 1;
  pData++;

  for (int i=0; i<nFullLength; i+=fixedLength)
  {
	  if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
		  return true;
	  pData += fixedLength;
  }
  return false;
}

bool DataItemFormatExplicit::getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
	  int fixedLength = m_pFixed->getLength(pData);
	  unsigned char nFullLength = nLength - 1;
	  pData++;

	  for (int i=0; i<nFullLength; i+=fixedLength)
	  {
		  if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
			  return true;
		  pData += fixedLength;
	  }
	  return false;
}

std::string DataItemFormatExplicit::printDescriptors(std::string header)
{
	return m_pFixed->printDescriptors(header);
}

bool DataItemFormatExplicit::filterOutItem(const char* name)
{
	return m_pFixed->filterOutItem(name);
}

bool DataItemFormatExplicit::isFiltered(const char* name)
{
	return m_pFixed->isFiltered(name);
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatExplicit::getWiresharkDefinitions()
{
  if (!m_pFixed)
  {
    Tracer::Error("Wrong format of explicit item");
    return NULL;
  }
  return m_pFixed->getWiresharkDefinitions();
}

fulliautomatix_data* DataItemFormatExplicit::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;

  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nFullLength = *pData;

  firstData = lastData = newDataUL(NULL, PID_LEN, byteoffset, 1, nFullLength);
  byteoffset+=1;

  pData++;

  lastData->next = m_pFixed->getData(pData, fixedLength, byteoffset);
  while(lastData->next)
    lastData = lastData->next;

  byteoffset += nFullLength;

  return firstData;
}
#endif
