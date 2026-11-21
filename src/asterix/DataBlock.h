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
#ifndef DATABLOCK_H_
#define DATABLOCK_H_

#include "Category.h"
#include "DataRecord.h"

class DataBlock {
public:
    DataBlock(Category *cat, unsigned long len, const unsigned char *data, double nTimestamp = 0.0, unsigned int nIP = 0);

    virtual
    ~DataBlock();

    Category *m_pCategory;
    unsigned long m_nLength;
    double m_nTimestamp; // Date and time when this packet was captured. This value is in seconds since January 1, 1970 00:00:00 GMT
    unsigned int m_nIP; // source ip when reading GPS files
    bool m_bFormatOK;

    std::list<DataRecord *> m_lDataRecords;

    bool
    getText(std::string &strResult, const unsigned int formatType); // appends value to strResult in formatType format
#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_data* getData(int byteoffset);
#endif
#if defined(PYTHON_WRAPPER)
    void getData(PyObject* plist, int verbose);
#endif

};

#endif /* DATABLOCK_H_ */
