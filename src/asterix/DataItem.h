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
#ifndef DATAITEM_H_
#define DATAITEM_H_

#include <string>

#include "DataItemDescription.h"

class DataItem
{
public:
  DataItem(DataItemDescription* pDesc);
  virtual
  ~DataItem();

  DataItemDescription* m_pDescription;

  bool getDescription(std::string& strDescription); // appends description to strDescription
  bool getText(std::string& strDescription, std::string& strHeader); // appends description to strDescription
  bool getXIDEF(std::string& strXIDEF); // appends XIDEF descriptin to strXIDEF

  long parse(const unsigned char* pData, long len);
  bool getValue(unsigned long& value, long nLength, const char* pstrBitsShortName, const char* pstrBitsName = NULL);
  bool getValue(long& value, long nLength, const char* pstrBitsShortName, const char* pstrBitsName = NULL);
  bool getValue(std::string& value, long nLength, const char* pstrBitsShortName, const char* pstrBitsName = NULL);

  long getLength() { return m_nLength; }
#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
  fulliautomatix_data* getData(int byteoffset);
#endif

private:
  unsigned char* m_pData;
  long m_nLength;

};

#endif /* DATAITEM_H_ */
