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
#include "asterixformat.hxx"

DataItemFormatCompound::DataItemFormatCompound(int id)
: DataItemFormat(id)
{
}

DataItemFormatCompound::DataItemFormatCompound(const DataItemFormatCompound& obj)
: DataItemFormat(obj.m_nID)
{
	std::list<DataItemFormat*>::iterator it = ((DataItemFormat&)obj).m_lSubItems.begin();

	while(it != obj.m_lSubItems.end())
	{
		DataItemFormat* di = (DataItemFormat*)(*it);
		m_lSubItems.push_back(di->clone());
		it++;
	}

	m_pParentFormat = obj.m_pParentFormat;
}

DataItemFormatCompound::~DataItemFormatCompound()
{
}

long DataItemFormatCompound::getLength(const unsigned char* pData)
{
	long totalLength = 0;
	std::list<DataItemFormat*>::iterator it;
	std::list<DataItemFormat*>::iterator it2;
	it2 = m_lSubItems.begin();
	DataItemFormatVariable* pCompoundPrimary = (DataItemFormatVariable*)(*it2);
	it2++;
	if (pCompoundPrimary == NULL)
	{
		Tracer::Error("Missing primary subfield of Compound");
		return 0;
	}

	int primaryPartLength = pCompoundPrimary->getLength(pData);
	const unsigned char* pSecData = pData + primaryPartLength;
	totalLength += primaryPartLength;

	int secondaryPart = 1;

	for ( it=pCompoundPrimary->m_lSubItems.begin() ; it != pCompoundPrimary->m_lSubItems.end(); it++ )
	{
		if (it2 == m_lSubItems.end())
		{
			Tracer::Error("Error in compound format");
			return 0;
		}

		DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
		bool lastPart = dip->isLastPart(pData);

		for (int i=0;i<7 && it2 != m_lSubItems.end(); i++)
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

bool DataItemFormatCompound::getText(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long)
{
	bool ret = false;
	std::list<DataItemFormat*>::iterator it;
	std::list<DataItemFormat*>::iterator it2;
	it2 = m_lSubItems.begin();
	DataItemFormatVariable* pCompoundPrimary = (DataItemFormatVariable*)(*it2);
	it2++;
	if (pCompoundPrimary == NULL)
	{
		Tracer::Error("Missing primary subfield of Compound");
		return false;
	}

	switch(formatType)
	{
	case CAsterixFormat::EJSON:
	case CAsterixFormat::EJSONH:
	{
		strResult += '{';
	}
	break;
	}

	int primaryPartLength = pCompoundPrimary->getLength(pData);
	unsigned char* pSecData = pData + primaryPartLength;

	int secondaryPart = 1;

	for ( it=pCompoundPrimary->m_lSubItems.begin() ; it != pCompoundPrimary->m_lSubItems.end(); it++ )
	{
		if (it2 == m_lSubItems.end())
		{
			Tracer::Error("Error in compound format");
			return false;
		}

		DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
		bool lastPart = dip->isLastPart(pData);

		for (int i=0;i<7 && it2 != m_lSubItems.end(); i++)
		{ // parse up to 8 secondary parts
			if (dip->isSecondaryPartPresent(pData, secondaryPart))
			{
				DataItemFormat* dip2 = (DataItemFormat*)(*it2);
				int skip = 0;
				std::string tmpStr = "";

				switch(formatType)
				{
				case CAsterixFormat::EJSONH:
				{
					tmpStr += "\n\t\t";
				}
				/* no break */
				case CAsterixFormat::EJSON:
				{
					tmpStr += "\"" + dip->getPartName(secondaryPart) + "\":";

					skip = dip2->getLength(pSecData);
					bool r = dip2->getText(tmpStr, strHeader, formatType, pSecData, skip);
					ret |= r;
					pSecData += skip;

					if (r)
					{
						strResult += tmpStr;
						// replace last ',' with '}'
						if (strResult[strResult.length()-1] == ',')
						{
							strResult[strResult.length()-1] = '}';
						}
						strResult += ",";
					}
				}
				break;
				default:
				{
					skip = dip2->getLength(pSecData);
					ret |= dip2->getText(strResult, strHeader, formatType, pSecData, skip);
					pSecData += skip;
				}
				break;
				}
			}
			it2++;
			secondaryPart++;
		}

		pData += dip->getLength();

		if (lastPart)
			break;
	}

	switch(formatType)
	{
	case CAsterixFormat::EJSON:
	case CAsterixFormat::EJSONH:
	{
		if (strResult[strResult.length()-1] == ',')
		{
			strResult[strResult.length()-1] = '}';
		}
		else
			strResult += '}';
	}
	break;
	}


	return ret;
}

std::string DataItemFormatCompound::printDescriptors(std::string header)
{
	std::string strDef = "";

	std::list<DataItemFormat*>::iterator it;
	for ( it=m_lSubItems.begin(), it++ ; it != m_lSubItems.end(); it++ )
	{
		DataItemFormat* dip = (DataItemFormat*)(*it);
		strDef += dip->printDescriptors(header);
	}
	return strDef;
}

bool DataItemFormatCompound::filterOutItem(const char* name)
{
	std::list<DataItemFormat*>::iterator it;
	for ( it=m_lSubItems.begin(), it++ ; it != m_lSubItems.end(); it++ )
	{
		DataItemFormat* dip = (DataItemFormat*)(*it);
		if (true == dip->filterOutItem(name))
			return true;
	}
	return false;
}

bool DataItemFormatCompound::isFiltered(const char* name)
{
	std::list<DataItemFormat*>::iterator it;
	for ( it=m_lSubItems.begin(), it++ ; it != m_lSubItems.end(); it++ )
	{
		DataItemFormat* dip = (DataItemFormat*)(*it);
		if (true == dip->isFiltered(name))
			return true;
	}
	return false;
}

const char* DataItemFormatCompound::getDescription(const char* field, const char* value = NULL )
{
  std::list<DataItemFormat*>::iterator it;
  for ( it=m_lSubItems.begin() ; it != m_lSubItems.end(); it++ )
  {
    DataItemBits* bv = (DataItemBits*)(*it);
    const char* desc = bv->getDescription(field, value);
    if (desc != NULL)
        return desc;
  }
  return NULL;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* DataItemFormatCompound::getWiresharkDefinitions()
{
	fulliautomatix_definitions *def = NULL, *startDef = NULL;

	std::list<DataItemFormat*>::iterator it;
	it = m_lSubItems.begin();
	DataItemFormatVariable* pCompoundPrimary = (DataItemFormatVariable*)(*it);
	it++;
	if (pCompoundPrimary == NULL)
	{
		Tracer::Error("Missing primary subfield of Compound");
		return NULL;
	}

	startDef = def = pCompoundPrimary->getWiresharkDefinitions();
	while(def->next)
		def = def->next;

	for ( ; it != m_lSubItems.end(); it++ )
	{
		DataItemFormat* dip = (DataItemFormat*)(*it);
		def->next = dip->getWiresharkDefinitions();
		while(def->next)
			def = def->next;
	}
	return startDef;
}

fulliautomatix_data* DataItemFormatCompound::getData(unsigned char* pData, long, int byteoffset)
{
	fulliautomatix_data *lastData = NULL, *firstData = NULL;

	std::list<DataItemFormat*>::iterator it;
	std::list<DataItemFormat*>::iterator it2;
	it2 = m_lSubItems.begin();
	DataItemFormatVariable* pCompoundPrimary = (DataItemFormatVariable*)(*it2);
	it2++;
	if (pCompoundPrimary == NULL)
	{
		Tracer::Error("Missing primary subfield of Compound");
		return false;
	}

	int primaryPartLength = pCompoundPrimary->getLength(pData);
	unsigned char* pSecData = pData + primaryPartLength;

	lastData = firstData = newDataTree(NULL, byteoffset, primaryPartLength, (char*)"Compound item header");

	lastData->next = pCompoundPrimary->getData(pData, primaryPartLength, byteoffset);
	while(lastData->next)
		lastData = lastData->next;

	lastData = newDataTreeEnd(lastData,byteoffset);

	byteoffset += primaryPartLength;

	int secondaryPart = 1;

	for ( it=pCompoundPrimary->m_lSubItems.begin() ; it != pCompoundPrimary->m_lSubItems.end(); it++ )
	{
		if (it2 == m_lSubItems.end())
		{
			Tracer::Error("Error in compound format");
			return firstData;
		}

		DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
		bool lastPart = dip->isLastPart(pData);

		for (int i=0;i<7 && it2 != m_lSubItems.end(); i++)
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

#if defined(PYTHON_WRAPPER)
PyObject* DataItemFormatCompound::getObject(unsigned char* pData, long nLength)
{
  PyObject* p = PyDict_New();
  insertToDict(p, pData, nLength);
  return p;
}

void DataItemFormatCompound::insertToDict(PyObject* p, unsigned char* pData, long nLength)
{
  std::list<DataItemFormat*>::iterator it;
  std::list<DataItemFormat*>::iterator it2;
  it2 = m_lSubItems.begin();
  DataItemFormatVariable* pCompoundPrimary = (DataItemFormatVariable*)(*it2);
  it2++;
  if (pCompoundPrimary == NULL)
  {
    Tracer::Error("Missing primary subfield of Compound");
  }

  int primaryPartLength = pCompoundPrimary->getLength(pData);
  unsigned char* pSecData = pData + primaryPartLength;

  pCompoundPrimary->insertToDict(p, pData, primaryPartLength);

  int secondaryPart = 1;

  for ( it=pCompoundPrimary->m_lSubItems.begin() ; it != pCompoundPrimary->m_lSubItems.end(); it++ )
  {
    if (it2 == m_lSubItems.end())
    {
      Tracer::Error("Error in compound format");
      return;
    }

    DataItemFormatFixed* dip = (DataItemFormatFixed*)(*it);
    bool lastPart = dip->isLastPart(pData);

    for (int i=0;i<7 && it2 != m_lSubItems.end(); i++)
    { // parse up to 8 secondary parts
      if (dip->isSecondaryPartPresent(pData, secondaryPart))
      {
        DataItemFormat* dip2 = (DataItemFormat*)(*it2);
        int skip = dip2->getLength(pSecData);
        dip2->insertToDict(p, pSecData, skip);

        pSecData += skip;
      }
      it2++;
      secondaryPart++;
    }

    pData += dip->getLength();

    if (lastPart)
      break;
  }
}


#endif
