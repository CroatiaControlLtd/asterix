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

#include "DataItemFormatVariable.h"
#include "Tracer.h"

DataItemFormatVariable::DataItemFormatVariable()
{
}

DataItemFormatVariable::~DataItemFormatVariable()
{
  // destroy list items
  std::list<DataItemFormatFixed*>::iterator it = m_lParts.begin();
  while(it != m_lParts.end())
  {
    delete (DataItemFormatFixed*)(*it);
    it = m_lParts.erase(it);
  }
}

long DataItemFormatVariable::getLength(const unsigned char* pData)
{
  long length = 0;
  std::list<DataItemFormatFixed*>::iterator it;
  bool lastPart = false;
  it=m_lParts.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);
    long partlen = dip->getLength();

    length += partlen;
    pData += partlen;

    if (it != m_lParts.end())
    {
      it++;
      if (it != m_lParts.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart);

  return length;
}

void DataItemFormatVariable::addBits(DataItemBits* pBits)
{
  std::list<DataItemFormatFixed*>::reverse_iterator rit;
  rit = m_lParts.rbegin();
  if (rit != m_lParts.rend())
  {
    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*rit);
    if (dip)
    {
      dip->addBits(pBits);
      return;
    }
  }
  Tracer::Error("Adding bits to Variable failed");
}

bool DataItemFormatVariable::get(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long nLength)
{
  std::list<DataItemFormatFixed*>::iterator it;
  bool lastPart = false;
  it=m_lParts.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    dip->get(strResult, strHeader, formatType, pData, dip->getLength());
    pData += dip->getLength();

    if (it != m_lParts.end())
    {
      it++;
      if (it != m_lParts.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart);
  return true;
}

bool DataItemFormatVariable::getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  std::list<DataItemFormatFixed*>::iterator it;
  bool lastPart = false;
  it=m_lParts.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    if (dip->getValue(pData, dip->getLength(), value, pstrBitsShortName, pstrBitsName))
      return true;

    pData += dip->getLength();

    if (it != m_lParts.end())
    {
      it++;
      if (it != m_lParts.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart);
  return false;
}

bool DataItemFormatVariable::getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  std::list<DataItemFormatFixed*>::iterator it;
  bool lastPart = false;
  it=m_lParts.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    if (dip->getValue(pData, dip->getLength(), value, pstrBitsShortName, pstrBitsName))
      return true;

    pData += dip->getLength();

    if (it != m_lParts.end())
    {
      it++;
      if (it != m_lParts.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart);
  return false;
}

bool DataItemFormatVariable::getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  std::list<DataItemFormatFixed*>::iterator it;
  bool lastPart = false;
  it=m_lParts.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    if (dip->getValue(pData, dip->getLength(), value, pstrBitsShortName, pstrBitsName))
      return true;

    pData += dip->getLength();

    if (it != m_lParts.end())
    {
      it++;
      if (it != m_lParts.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart);

  return false;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatVariable::getWiresharkDefinitions()
{
  fulliautomatix_definitions *def = NULL, *startDef = NULL;

  std::list<DataItemFormatFixed*>::iterator it;
  for ( it=m_lParts.begin() ; it != m_lParts.end(); it++ )
  {
    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    if (def)
    {
      def->next = dip->getWiresharkDefinitions();
    }
    else
    {
      startDef = def = dip->getWiresharkDefinitions();
    }
    while(def->next)
      def = def->next;
  }
  return startDef;
}

fulliautomatix_data* DataItemFormatVariable::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;
  std::list<DataItemFormatFixed*>::iterator it;
  bool lastPart = false;

  it=m_lParts.begin();

  DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);

  do
  {
    lastPart = dip->isLastPart(pData);

    if (lastData)
    {
      lastData->next = dip->getData(pData, dip->getLength(), byteoffset);
    }
    else
    {
      firstData = lastData = dip->getData(pData, dip->getLength(), byteoffset);
    }
    while(lastData->next)
      lastData = lastData->next;

    byteoffset += dip->getLength();
    pData += dip->getLength();
    len -= dip->getLength();

    if (it != m_lParts.end())
    {
      it++;
      if (it != m_lParts.end())
      {
        dip = (DataItemFormatFixed*)(*it);
      }
    }
  }
  while(!lastPart && len > 0);

  return firstData;
}
#endif
