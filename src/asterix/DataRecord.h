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

#ifndef DATARECORD_H_
#define DATARECORD_H_

#include "DataItem.h"

class DataRecord
{
public:
  DataRecord(Category* cat, int id, unsigned long len, const unsigned char* data, unsigned long nTimestamp);
  virtual
  ~DataRecord();

  Category* m_pCategory;
  int           m_nID;
  unsigned long m_nLength;
  unsigned long m_nFSPECLength;
  unsigned char* m_pFSPECData;
  unsigned long m_nTimestamp; // Date and time when this packet was captured. This value is in seconds since January 1, 1970 00:00:00 GMT

  bool m_bFormatOK;
  std::list<DataItem*> m_lDataItems;

  int getCategory() { return (m_pCategory) ? m_pCategory->m_id : 0; }

  bool getDescription(std::string& strDescription); // appends description to strDescription
  bool getText(std::string& strDescription, std::string& strHeader);
  bool getXIDEF(std::string& strXIDEF); // appends XIDEF descriptin to strXIDEF

  DataItem* getItem(int itemid);
  bool getValue(int itemid, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName = NULL);
  bool getValue(int itemid, long& value, const char* pstrBitsShortName, const char* pstrBitsName = NULL);
  bool getValue(int itemid, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName = NULL);

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
  fulliautomatix_data* getData(int byteoffset);
#endif
};

#endif /* DATARECORD_H_ */
