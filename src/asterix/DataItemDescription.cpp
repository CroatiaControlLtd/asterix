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

#include "DataItemDescription.h"
#include <stdlib.h>

DataItemDescription::DataItemDescription(std::string id)
        : m_strID(id), m_pFormat(NULL), m_eRule(DATAITEM_UNKNOWN) {
    m_nID = strtol(id.c_str(), NULL, 16);
}

DataItemDescription::~DataItemDescription() {
    if (m_pFormat != NULL) {
        delete m_pFormat;
        m_pFormat = NULL;
    }
}
