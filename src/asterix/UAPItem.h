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

#ifndef UAPITEM_H_
#define UAPITEM_H_

#include "Tracer.h"
#include "DataItemFormat.h"


class UAPItem : public DataItemFormat
{
public:
  UAPItem();
  virtual
  ~UAPItem();

  int m_nBit; // <!ATTLIST UAPItem bit CDATA #REQUIRED>
  int m_nFRN; // <!ATTLIST UAPItem frn CDATA #REQUIRED>
  bool m_bFX; // <!ATTLIST UAPItem fx CDATA "0">
  int m_nLen; // <!ATTLIST UAPItem len CDATA "-">
  int m_nItemID; // <!ELEMENT UAPItem (#PCDATA)>

  virtual long getLength(const unsigned char* pData) { Tracer::Error("Function should not be called!"); return 0; }
  virtual void addBits(DataItemBits* pBits) { Tracer::Error("Function should not be called!"); }
  virtual bool getDescription(std::string& strDescription, unsigned char* pData, long nLength)
  { Tracer::Error("Function should not be called!"); return false;} // appends description to strDescription
  virtual bool getText(std::string& strDescription, std::string& strHeader, unsigned char* pData, long nLength)
  { Tracer::Error("Function should not be called!"); return false;} // appends description to strDescription
  virtual bool getXIDEF(std::string& strXIDEF, unsigned char* pData, long nLength)
  { Tracer::Error("Function should not be called!"); return false;} // appends description to strDescription
  virtual bool getValue(unsigned char* pData, long nLength, long& value, const char* pstrBitsShortName, const char* pstrBitsName = NULL) { Tracer::Error("Function should not be called!"); return false; }
  virtual bool getValue(unsigned char* pData, long nLength, unsigned long& value, const char* pstrBitsShortName, const char* pstrBitsName = NULL) { Tracer::Error("Function should not be called!"); return false; }
  virtual bool getValue(unsigned char* pData, long nLength, std::string& value, const char* pstrBitsShortName, const char* pstrBitsName = NULL) { Tracer::Error("Function should not be called!"); return false; }
#ifdef WIRESHARK_WRAPPER
  fulliautomatix_definitions* getWiresharkDefinitions();
  fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
};

#endif /* UAPITEM_H_ */
