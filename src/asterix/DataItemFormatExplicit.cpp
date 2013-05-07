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
    Tracer::Error("Wrong data in Repetitive");
    return;
  }
  m_pFixed->m_lBits.push_back(pBits);
}

bool DataItemFormatExplicit::getDescription(std::string& strDescription, unsigned char* pData, long nLength)
{
//TODO  int fixedLength = m_pFixed->getLength(pData);
  //TODO  unsigned char nFullLength = *pData;

  pData++;

  m_pFixed->getDescription(strDescription, pData, nLength);
  return true;
}

bool DataItemFormatExplicit::getText(std::string& strDescription, std::string& strHeader, unsigned char* pData, long nLength)
{
//TODO  int fixedLength = m_pFixed->getLength(pData);
  //TODO  unsigned char nFullLength = *pData;

  pData++;

  m_pFixed->getText(strDescription, strHeader, pData, nLength);
  return true;
}

bool DataItemFormatExplicit::getXIDEF(std::string& strXIDEF, unsigned char* pData, long nLength)
{
//TODO  int fixedLength = m_pFixed->getLength(pData);
  //TODO  unsigned char nFullLength = *pData;

  pData++;

  m_pFixed->getDescription(strXIDEF, pData, nLength);
  return true;
}


bool DataItemFormatExplicit::getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  //TODO  unsigned char nFullLength = *pData;

  pData++;

  if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
    return true;

  return false;
}

bool DataItemFormatExplicit::getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  //TODO unsigned char nFullLength = *pData;

  pData++;

  if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
    return true;

  return false;
}

bool DataItemFormatExplicit::getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  int fixedLength = m_pFixed->getLength(pData);
  //TODO unsigned char nFullLength = *pData;

  pData++;

  if (m_pFixed->getValue(pData, fixedLength, value, pstrBitsShortName, pstrBitsName))
    return true;

  return false;
}

#ifdef WIRESHARK_WRAPPER
fulliautomatix_definitions* DataItemFormatExplicit::getWiresharkDefinitions()
{
  if (!m_pFixed)
  {
    Tracer::Error("Wring format of repetitive item");
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
