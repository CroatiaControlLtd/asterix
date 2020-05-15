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

#ifndef XMLPARSER_H_
#define XMLPARSER_H_

#include "AsterixDefinition.h"
#include "expat.h"
#include "DataItemFormatFixed.h"
#include "DataItemFormatVariable.h"
#include "DataItemFormatCompound.h"
#include "DataItemFormatExplicit.h"
#include "DataItemFormatRepetitive.h"
#include "DataItemFormatBDS.h"
#include "UAP.h"

//! parsing buffer size
#define BUFFSIZE        8192

class XMLParser {
public:
    XMLParser();

    virtual
    ~XMLParser();

    bool Parse(FILE *pFile, AsterixDefinition *pDefinition, const char *filename);

    bool m_bErrorDetectedStopParsing; //!< Flag to stop parsing if error detected

    AsterixDefinition *m_pDef;
    Category *m_pCategory; //<! Currently parsed <Category>
    DataItemDescription *m_pDataItem; //!< Currently parsed <DataItemDescription>
    DataItemFormat *m_pFormat; //!< Currently parsed <Format>
    BitsValue *m_pBitsValue; //!< Currently parsed <BitsValue>
    UAPItem *m_pUAPItem; //!< Currently parsed UAPItem
    UAP *m_pUAP; //!< Currently parsed UAP

    // pointer to string to which to assign next CDATA
    std::string *m_pstrCData;

    // pointer to int to which to assign next CDATA
    int *m_pintCData;

    // current parsed file name
    const char *m_pFileName;

    void GetCData(std::string *pstr) { m_pstrCData = pstr; }

    void GetCData(int *pint) {
        m_pintCData = pint;
        m_pstrCData = NULL;
    }

    static void XMLCALL
    ElementHandlerStart(void *data, const char *el, const char **attr);

    static void XMLCALL
    ElementHandlerEnd(void *data, const char *el);

    static void XMLCALL
    CharacterHandler(void *userData, const XML_Char *s, int len);

    void Error(const char *errstr); //!< print error message with line number
    void Error(const char *errstr, const char *param1);

private:
    XML_Parser m_Parser;

    char m_pBuff[BUFFSIZE];

    bool GetAttribute(const char *elementName, const char *attrName, std::string *ptrString);

    bool GetAttribute(const char *elementName, const char *attrName, int *ptrInt);
};

#endif /* XMLPARSER_H_ */
