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

#include "DataItemFormatCompound.h"
#include "Tracer.h"

DataItemFormatCompound::DataItemFormatCompound()
: m_pCompoundPrimary(NULL)
{
}

DataItemFormatCompound::~DataItemFormatCompound()
{
  // destroy list items
  std::list<DataItemFormat*>::iterator it = m_lParts.begin();
  while(it != m_lParts.end())
  {
    delete (DataItemFormat*)(*it);
    it = m_lParts.erase(it);
  }
}

long DataItemFormatCompound::getLength(const unsigned char* pData)
{
  long totalLength = 0;

  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return 0;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  const unsigned char* pSecData = pData + primaryPartLength;
  totalLength += primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return 0;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);
        pSecData += skip;
        totalLength += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }

  return totalLength;
}

void DataItemFormatCompound::addBits(DataItemBits* pBits)
{
  std::list<DataItemFormat*>::reverse_iterator rit;
  rit = m_lParts.rbegin();
  if (rit != m_lParts.rend())
  {
    DataItemFormat* dip = (DataItemFormat*)(*rit);
    if (dip)
    {
      dip->addBits(pBits);
      return;
    }
  }
  else if (m_pCompoundPrimary != NULL)
  {
    m_pCompoundPrimary->addBits(pBits);
    return;
  }
  Tracer::Error("Adding bits to Variable failed");
}

bool DataItemFormatCompound::getDescription(std::string& strDescription, unsigned char* pData, long nLength)
{
  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return true;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return true;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);
        dip2->getDescription(strDescription, pSecData, skip);
        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return true;
}

bool DataItemFormatCompound::getText(std::string& strDescription, std::string& strHeader, unsigned char* pData, long nLength)
{
  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return true;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return true;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);
        dip2->getText(strDescription, strHeader, pSecData, skip);
        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return true;
}

bool DataItemFormatCompound::getXIDEF(std::string& strXIDEF, unsigned char* pData, long nLength)
{
  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return true;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return true;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);
        dip2->getXIDEF(strXIDEF, pSecData, skip);
        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return true;
}


bool DataItemFormatCompound::getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return false;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return false;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);

        if (dip2->getValue(pSecData, skip, value, pstrBitsShortName, pstrBitsName))
          return true;

        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return false;
}

bool DataItemFormatCompound::getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return false;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return false;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);

        if (dip2->getValue(pSecData, skip, value, pstrBitsShortName, pstrBitsName))
          return true;

        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return false;
}

bool DataItemFormatCompound::getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName)
{
  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return false;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return false;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);

        if (dip2->getValue(pSecData, skip, value, pstrBitsShortName, pstrBitsName))
          return true;

        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return false;
}

#ifdef WIRESHARK_WRAPPER
fulliautomatix_definitions* DataItemFormatCompound::getWiresharkDefinitions()
{
  fulliautomatix_definitions *def = NULL, *startDef = NULL;

  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return NULL;
  }

  startDef = def = m_pCompoundPrimary->getWiresharkDefinitions();
  while(def->next)
    def = def->next;

  std::list<DataItemFormat*>::iterator it;
  it = m_lParts.begin();
  for ( it=m_lParts.begin() ; it != m_lParts.end(); it++ )
  {
    DataItemFormat* dip = (DataItemFormat*)(*it);
    def->next = dip->getWiresharkDefinitions();
    while(def->next)
      def = def->next;
  }
  return startDef;
}

fulliautomatix_data* DataItemFormatCompound::getData(unsigned char* pData, long len, int byteoffset)
{
  fulliautomatix_data *lastData = NULL, *firstData = NULL;

  if (m_pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
    return NULL;
  }

  int primaryPartLength = m_pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  lastData = firstData = newDataTree(NULL, byteoffset, primaryPartLength, "Compound item header");

  lastData->next = m_pCompoundPrimary->getData(pData, primaryPartLength, byteoffset);
  while(lastData->next)
    lastData = lastData->next;

  lastData = newDataTreeEnd(lastData,byteoffset);

  byteoffset += primaryPartLength;

  int secondaryPart = 1;

  std::list<DataItemFormatFixed*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lParts.begin();

  for ( it=m_pCompoundPrimary->m_lParts.begin() ; it != m_pCompoundPrimary->m_lParts.end(); it++ )
  {
    if (it2 == m_lParts.end())
    {
      Tracer::Error("Error in compound format");
      return firstData;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lParts.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);
        lastData->next = dip2->getData(pSecData, skip, byteoffset);
        while(lastData->next)
          lastData = lastData->next;

        pSecData += skip;
        byteoffset += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
  return firstData;
}
#endif
