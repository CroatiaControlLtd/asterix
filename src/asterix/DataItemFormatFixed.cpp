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

#include "DataItemFormatFixed.h"
#include "Tracer.h"

DataItemFormatFixed::DataItemFormatFixed()
: m_nLength(0)
{
}

DataItemFormatFixed::~DataItemFormatFixed()
{
  // destroy list items
  std::list<DataItemBits*>::iterator it = m_lBits.begin();
  while(it != m_lBits.end())
  {
    delete (DataItemBits*)(*it);
    it = m_lBits.erase(it);
  }
}

long DataItemFormatFixed::getLength()
{
  return m_nLength;
}

long DataItemFormatFixed::getLength(const unsigned char*)
{
  return m_nLength;
}

void DataItemFormatFixed::addBits(DataItemBits* pBits)
{
  m_lBits.push_back(pBits);
}

/*
 * Check FX bit to see if this is last part of variable item.
 */
bool DataItemFormatFixed::isLastPart(const unsigned char* pData)
{
  // go through all bits and find which is FX
  std::list<DataItemBits*>::iterator bitit;
  for ( bitit=m_lBits.begin() ; bitit != m_lBits.end(); bitit++ )
  {
    DataItemBits* bit = (DataItemBits*)(*bitit);
    if (bit == NULL)
    {
      Tracer::Error("Missing bits format!");
      return true;
    }
    if (bit->m_bExtension)
    { // this is extension bit
      int bitnr = bit->m_nFrom;

      if (bitnr < 1 || bitnr > m_nLength*8)
      {
        Tracer::Error("Error in bits format");
        return true;
      }

      // select byte
      int bytenr = m_nLength-1-(bitnr-1)/8;

      pData += bytenr;

      bitnr = bitnr%8;
      if (bitnr == 0)
        bitnr = 8;

      unsigned char mask = 0x01;
      mask <<= (bitnr-1);

      // check this bit in pData
      if (*pData & mask)
      {
        return false;
      }
      return true;
    }
  }
  return true;
}

bool DataItemFormatFixed::isSecondaryPartPresent(const unsigned char* pData, int part)
{
  // go through all bits and find which has BitsPresence set to part
  std::list<DataItemBits*>::iterator bitit;
  for ( bitit=m_lBits.begin() ; bitit != m_lBits.end(); bitit++ )
  {
    DataItemBits* bit = (DataItemBits*)(*bitit);
    if (bit == NULL)
    {
      Tracer::Error("Missing bits format!");
      return true;
    }
    if (bit->m_nPresenceOfField == part)
    { // found
      int bitnr = bit->m_nFrom;

      if (bitnr < 1 || bitnr > m_nLength*8)
      {
        Tracer::Error("Error in bits format");
        return true;
      }

      // select byte
      int bytenr = (bitnr-1)/8;
      pData += bytenr;

      bitnr -= bytenr*8;

      unsigned char mask = 0x01;
      mask <<= (bitnr-1);

      // check this bit in pData
      if (*pData & mask)
      {
        return true;
      }
      return false;
    }
  }
  Tracer::Error("BitsPresence bit not found");
  return false;
}

bool DataItemFormatFixed::getDescription(std::string& strDescription, unsigned char* pData, long nLength)
{
  if (m_nLength != nLength)
  {
    Tracer::Error("Length doesn't match!!!");
    return true;
  }

  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    bv->getDescription(strDescription, pData, m_nLength);
  }
  return true;
}

bool DataItemFormatFixed::getText(std::string& strDescription, std::string& strHeader, unsigned char* pData, long nLength)
{
  if (m_nLength != nLength)
  {
    Tracer::Error("Length doesn't match!!!");
    return true;
  }

  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    bv->getText(strDescription, strHeader, pData, m_nLength);
  }
  return true;
}

bool DataItemFormatFixed::getXIDEF(std::string& strXIDEF, unsigned char* pData, long nLength)
{
  if (m_nLength != nLength)
  {
    Tracer::Error("Length doesn't match!!!");
    return true;
  }

  if (!m_strXIDEF.empty())
  {
    strXIDEF += format("\n<%s>", m_strXIDEF.c_str());
  }

  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    bv->getXIDEF(strXIDEF, pData, m_nLength);
  }

  if (!m_strXIDEF.empty())
  {
    strXIDEF += format("\n</%s>", m_strXIDEF.c_str());
  }

  return true;
}

bool DataItemFormatFixed::getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (bv->getValue(pData, nLength, value, pstrBitsShortName, pstrBitsName))
      return true;
  }
  return false;
}

bool DataItemFormatFixed::getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (bv->getValue(pData, nLength, value, pstrBitsShortName, pstrBitsName))
      return true;
  }
  return false;
}

bool DataItemFormatFixed::getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (bv->getValue(pData, nLength, value, pstrBitsShortName, pstrBitsName))
      return true;
  }
  return false;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatFixed::getWiresharkDefinitions()
{
  fulliautomatix_definitions *def = NULL, *startDef = NULL;
  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (def)
    {
      def->next = bv->getWiresharkDefinitions();
      def = def->next;
    }
    else
    {
      startDef = def = bv->getWiresharkDefinitions();
    }
  }
  return startDef;
}

fulliautomatix_data* DataItemFormatFixed::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;
  std::list<DataItemBits*>::iterator it;
  DataItemBits* bv = NULL;
  for ( it=m_lBits.begin() ; it != m_lBits.end(); it++ )
  {
    bv = (DataItemBits*)(*it);
    if (lastData)
    {
      lastData->next = bv->getData(pData, len, byteoffset);
      lastData = lastData->next;
    }
    else
    {
      firstData = lastData = bv->getData(pData, len, byteoffset);
    }
  }
  return firstData;
}
#endif
