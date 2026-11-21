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

#ifndef INPUTPARSER_H_
#define INPUTPARSER_H_

#include "AsterixDefinition.h"
#include "AsterixData.h"
#include "DataBlock.h"
#include <ios>
#include <iostream>
#include <iomanip>
#include <sstream>

class InputParser {
public:
    InputParser(AsterixDefinition *pDefinition);

    ~InputParser() { if (m_pDefinition) delete m_pDefinition; }

    AsterixData *parsePacket(const unsigned char *m_pBuffer, unsigned int m_nBufferSize, double nTimestamp = 0.0, unsigned int nIP = 0);

    DataBlock *parse_next_data_block(const unsigned char *m_pData, unsigned int &m_nPos, unsigned int m_nBufferSize,
                                     double nTimestamp, unsigned int nIP, unsigned int &m_nDataLength);

    std::string printDefinition();

    bool filterOutItem(int cat, std::string item, const char *name);

    bool isFiltered(int cat, std::string item, const char *name);

private:
    AsterixDefinition *m_pDefinition; // Asterix definitions

};

#endif /* INPUTPARSER_H_ */
