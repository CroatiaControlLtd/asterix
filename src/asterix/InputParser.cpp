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

#include "InputParser.h"

InputParser::InputParser(AsterixDefinition* pDefinition)
: m_pDefinition(pDefinition)
{
}

/*
 * Parse data
 */
AsterixData* InputParser::parsePacket(const unsigned char* m_pBuffer, unsigned int m_nBufferSize, unsigned long nTimestamp)
{
  AsterixData* pAsterixData = new AsterixData();
  unsigned int m_nPos = 0;
  unsigned int m_nDataLength = 0;
  const unsigned char* m_pData = m_pBuffer; // internally used pointer to parsed data

  while(m_nPos < m_nBufferSize)
  {
    bool bOK = true;
    m_nDataLength = m_nBufferSize;

    while(bOK && m_nDataLength > 0)
    {
      unsigned char nCategory = *m_pData;
      m_pData ++; m_nPos ++;
      unsigned short dataLen = *m_pData; // length
      m_pData ++; m_nPos ++;
      dataLen <<= 8;
      dataLen |= *m_pData;
      m_pData ++; m_nPos ++;

      // parse Asterix data
      if (dataLen <= 3 || dataLen > m_nDataLength)
      {
        Tracer::Error("Wrong Asterix data length (%d)", dataLen);

        if (dataLen <= 3)
        {
          // otherwise finish
          return pAsterixData;
        }

        // there is not enough data, but parse what is there
        dataLen = (unsigned short)m_nDataLength;
      }

      m_nDataLength -= 3;
      dataLen -= 3;

      DataBlock* db = new DataBlock(m_pDefinition->getCategory(nCategory), dataLen, m_pData, nTimestamp);
      m_pData += dataLen;
      m_nPos += dataLen;
      pAsterixData->m_lDataBlocks.push_back(db);

      if (db->m_bFormatOK == false)
      {
        break;
      }
      m_nDataLength -= dataLen;
    }
  }
  return pAsterixData;
}

std::string InputParser::printDefinition()
{
	return m_pDefinition->printDescriptors();
}

bool InputParser::filterOutItem(int cat, std::string item, const char* name)
{
	return m_pDefinition->filterOutItem(cat,item,name);
}

bool InputParser::isFiltered(int cat, std::string item, const char* name)
{
	return m_pDefinition->isFiltered(cat,item,name);
}
