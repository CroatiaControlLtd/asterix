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

#ifndef DATAITEMDESCRIPTION_H_
#define DATAITEMDESCRIPTION_H_

#include "DataItemFormat.h"

class DataItemDescription {
public:
    DataItemDescription(std::string id);

    virtual
    ~DataItemDescription();

    std::string m_strID;
    int m_nID; //! Data item ID in hexadecimal (this is used for BDS registers deconding)

    void setName(char *name) { m_strName = name; }

    void setDefinition(char *definition) { m_strDefinition = definition; }

    void setFormat(char *format) { m_strFormat = format; }

    bool getText(std::string &strResult, std::string &strHeader, const unsigned int formatType, unsigned char *pData,
                 long nLength) // appends value to strResult
    {
        return m_pFormat->getText(strResult, strHeader, formatType, pData, nLength);
    };

    std::string m_strName;
    std::string m_strDefinition;
    std::string m_strFormat;
    std::string m_strNote;

    DataItemFormat *m_pFormat;

    typedef enum {
        DATAITEM_UNKNOWN = 0,
        DATAITEM_OPTIONAL,
        DATAITEM_MANDATORY
    } _eRule;

    _eRule m_eRule;

};

#endif /* DATAITEMDESCRIPTION_H_ */
