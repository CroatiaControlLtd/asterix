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

#ifndef DATAITEMFORMATCOMPOUND_H_
#define DATAITEMFORMATCOMPOUND_H_

#include "DataItemFormat.h"
#include "DataItemFormatVariable.h"

class DataItemFormatCompound : public DataItemFormat
{
public:
  DataItemFormatCompound(int id=0);
  DataItemFormatCompound(const DataItemFormatCompound& obj);
  virtual
  ~DataItemFormatCompound();

  DataItemFormatCompound* clone() const { return new DataItemFormatCompound(*this); } // Return clone of object
  long getLength(const unsigned char* pData);
  bool getText(std::string& strResult, std::string& strHeader, const unsigned int formatType, unsigned char* pData, long nLength); // appends value description to strResult
  std::string printDescriptors(std::string header); // print items format descriptors
  bool filterOutItem(const char* name); // mark item for filtering
  bool isFiltered(const char* name);
  bool isCompound() 	{ return true; }; // true if this is Compound format
  const char* getDescription(const char* field, const char* value ); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
  fulliautomatix_definitions* getWiresharkDefinitions();
  fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif

#if defined(PYTHON_WRAPPER)
  PyObject* getObject(unsigned char* pData, long nLength, int description);
  void insertToDict(PyObject* p, unsigned char* pData, long nLength, int description);
#endif

};

#endif /* DATAITEMFORMATCOMPOUND_H_ */
