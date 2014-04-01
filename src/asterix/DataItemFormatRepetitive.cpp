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

DataItemFormatRepetitive::DataItemFormatRepetitive()
: m_pFixed(NULL)
{
}

DataItemFormatRepetitive::~DataItemFormatRepetitive()
{
  if (m_pFixed)
    delete m_pFixed;
}

long DataItemFormatRepetitive::getLength(const unsigned char* pData)
{
  if (m_pFixed == NULL)
  {
    Tracer::Error("Wrong data in Repetitive");
    return 0;
  }
  unsigned char nRepetition = *pData;
  return (1+nRepetition*m_pFixed->getLength(pData+1));
}

void DataItemFormatRepetitive::addBits(DataItemBits* pBits)
{
  if (m_pFixed == NULL)
  {
    Tracer::Error("Wrong data in Repetitive");
    return;
  }
  m_pFixed->m_lBits.push_back(pBits);
}

bool DataItemFormatRepetitive::get(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long nLength)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nRepetition = *pData;

  if (1+nRepetition*fixedLength != nLength)
  {
    Tracer::Error("Wrong length in Repetitive");
    return true;
  }

  pData++;

  switch(formatType)
  {
	  case CAsterixFormat::EJSON:
	  case CAsterixFormat::EJSONH:
		  strResult += format("[");

		  while(nRepetition--)
		  {
			  m_pFixed->get(strResult, strHeader, formatType, pData, fixedLength);
			  pData += fixedLength;

			  if (nRepetition > 0)
				  strResult += format(",");
		  }
		  strResult += format("]");
		  break;
	  default:
		  while(nRepetition--)
		  {
			m_pFixed->get(strResult, strHeader, formatType, pData, fixedLength);
		    pData += fixedLength;
		  }
		  break;
  	  }
  return true;
}

bool DataItemFormatRepetitive::getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nRepetition = *pData;

  if (1+nRepetition*fixedLength != nLength)
  {
    Tracer::Error("Wrong length in Repetitive");
    return false;
  }

  pData++;

  while(nRepetition--)
  {
    if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
      return true;

    pData += fixedLength;
  }
  return false;
}

bool DataItemFormatRepetitive::getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nRepetition = *pData;

  if (1+nRepetition*fixedLength != nLength)
  {
    Tracer::Error("Wrong length in Repetitive");
    return false;
  }

  pData++;

  while(nRepetition--)
  {
    if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
      return true;

    pData += fixedLength;
  }
  return false;
}

bool DataItemFormatRepetitive::getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  unsigned char nRepetition = *pData;

  if (1+nRepetition*fixedLength != nLength)
  {
    Tracer::Error("Wrong length in Repetitive");
    return false;
  }

  pData++;

  while(nRepetition--)
  {
    if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
      return true;

    pData += fixedLength;
  }
  return false;
}

std::string DataItemFormatRepetitive::printDescriptors(std::string header)
{
	return m_pFixed->printDescriptors(header);
}

bool DataItemFormatRepetitive::filterOutItem(const char* name)
{
	return m_pFixed->filterOutItem(name);
}

bool DataItemFormatRepetitive::isFiltered(const char* name)
{
	return m_pFixed->isFiltered(name);
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatRepetitive::getWiresharkDefinitions()
{
  if (!m_pFixed)
  {
    Tracer::Error("Wring format of repetitive item");
    return NULL;
  }
  return m_pFixed->getWiresharkDefinitions();
}

fulliautomatix_data* DataItemFormatRepetitive::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;

  int fixedLength = m_pFixed->getLength(pData);
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
    lastData->next = m_pFixed->getData(pData, fixedLength, byteoffset);
    while(lastData->next)
      lastData = lastData->next;

    pData += fixedLength;
    byteoffset += fixedLength;
  }

  return firstData;
}
#endif
