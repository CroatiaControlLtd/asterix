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

#ifndef DATAITEMFORMATFIXED_H_
#define DATAITEMFORMATFIXED_H_

#include <set>
#include <vector>
#include "DataItemFormat.h"
#include "DataItemBits.h"

class DataItemFormatFixed : public DataItemFormat {
public:
    DataItemFormatFixed(int id = 0);

    DataItemFormatFixed(const DataItemFormatFixed &obj);

    virtual
    ~DataItemFormatFixed();

    int m_nLength;

private:
    bool m_bJsonPrefixesSet;                   //!< m_vJsonPrefixes is built
    bool m_bPresenceBitsSet;                   //!< m_vPresenceBits is built
    std::vector<int> m_vPresenceBits;          //!< index part: bit number of its BitsPresence bit, 0 none, -1 use the list
    std::vector<std::string> m_vJsonPrefixes;  //!< EJSONX prefixes of this item on its own (empty object, extent 0)

public:
    DataItemFormatFixed *clone() const { return new DataItemFormatFixed(*this); } // Return clone of object
    long getLength();

    bool isLastPart(const unsigned char *pData);

    bool isSecondaryPartPresent(const unsigned char *pData, int part);

    std::string &getPartName(int part);

    long getLength(const unsigned char *pData, long nMaxLength);

    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength); // appends value description to strResult
    /**
     * Same as getText, but in EJSONX each bits field starts with jsonPrefixes[i] ("key":{ of the
     * i-th bits field), built by makeJsonPrefixes so that keys are unique within the JSON object.
     */
    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength, const std::vector<std::string> &jsonPrefixes);
    /**
     * EJSONX keys of the bits fields, made unique within the JSON object being written:
     * jsonKeys holds the keys already used in that object (shared by all parts of a Variable item)
     * and extent is the index of this part (0 for the first one), used to number FX bits: FX0, FX1...
     * The new keys are added to jsonKeys and their prefixes ("key":{) appended to jsonPrefixes.
     * The result depends only on the format, so callers compute it once and keep it.
     */
    void makeJsonPrefixes(std::set<std::string> &jsonKeys, int extent, std::vector<std::string> &jsonPrefixes);
    std::string printDescriptors(std::string header); // print items format descriptors
    bool filterOutItem(const char *name); // mark item for filtering
    bool isFiltered(const char *name);

    bool isFixed() { return true; }; // true if this is Fixed format
    const char *getDescription(const char *field, const char *value); // return description ef element

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
    fulliautomatix_data* getData(unsigned char* pData, long len, int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    PyObject* getObject(unsigned char* pData, long nLength, int description);
    void insertToDict(PyObject* p, unsigned char* pData, long nLength, int verbose);
#endif
};

#endif /* DATAITEMFORMATFIXED_H_ */
