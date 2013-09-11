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

DataItem::DataItem(DataItemDescription* pDesc)
: m_pDescription(pDesc)
, m_pData(NULL)
, m_nLength(0)
{
}

DataItem::~DataItem()
{
  if (m_pData)
    delete[] m_pData;
}

bool DataItem::getDescription(std::string& strDescription)
{
  strDescription += format("\n\nItem %d : %s", m_pDescription->m_nID, m_pDescription->m_strName.c_str());
  strDescription += format("\n[ ");
  for (int i=0; i<m_nLength; i++)
  {
    strDescription += format("%02X ", *(m_pData+i));
  }
  strDescription += format("]");

  m_pDescription->getDescription(strDescription, m_pData, m_nLength);
  return true;
}

bool DataItem::getText(std::string& strDescription, std::string& strHeader)
{
	std::string newHeader = format("%s.%d", strHeader.c_str(), m_pDescription->m_nID);
	m_pDescription->getText(strDescription, newHeader, m_pData, m_nLength);
	return true;
}

bool DataItem::getXIDEF(std::string& strXIDEF)
{
  m_pDescription->getXIDEF(strXIDEF, m_pData, m_nLength);
  return true;
}

long DataItem::parse(const unsigned char* pData, long len)
{
  if (m_pDescription == NULL || m_pDescription->m_pFormat == NULL)
  {
    Tracer::Error("DataItem::parse NULL pointer");
    return 0;
  }

  m_nLength = m_pDescription->m_pFormat->getLength(pData);

  if (m_nLength > len)
  {
    Tracer::Error("DataItem::parse needed length=%d , and there is only %d", m_nLength, len);
  }
  else if (m_nLength>0)
  {
    m_pData = new unsigned char[m_nLength];
    memcpy(m_pData, pData, m_nLength);
  }
  else
  {
    Tracer::Error("DataItem::parse length=0");
  }
  return m_nLength;
}

bool DataItem::getValue(unsigned long& value, long nLength, const char* pstrBitsShortName, const char* pstrBitsName)
{
  return m_pDescription->m_pFormat->getValue(m_pData, nLength, value, pstrBitsShortName, pstrBitsName);
}

bool DataItem::getValue(long& value, long nLength, const char* pstrBitsShortName, const char* pstrBitsName)
{
  return m_pDescription->m_pFormat->getValue(m_pData, nLength, value, pstrBitsShortName, pstrBitsName);
}

bool DataItem::getValue(std::string& value, long nLength, const char* pstrBitsShortName, const char* pstrBitsName)
{
  return m_pDescription->m_pFormat->getValue(m_pData, nLength, value, pstrBitsShortName, pstrBitsName);
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_data* DataItem::getData(int byteoffset)
{
  fulliautomatix_data *firstData, *lastData;
  std::string strDesc;
  char tmp[20];
  sprintf(tmp, "Data item %03d - ", m_pDescription->m_nID);
  strDesc = tmp;
  strDesc += m_pDescription->m_strName;
  lastData = firstData = newDataTree(NULL, byteoffset, m_nLength, (char*)strDesc.c_str());
  if (m_pDescription && m_pDescription->m_pFormat && m_pData)
  {
    lastData->next = m_pDescription->m_pFormat->getData(m_pData, m_nLength, byteoffset);
  }
  else
  {
    lastData->next = newDataMessage(NULL, byteoffset, m_nLength, 2, "Error: Unknown item format.");
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
