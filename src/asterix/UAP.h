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

#ifndef UAP_H_
#define UAP_H_

#include "UAPItem.h"

class UAP {
public:
    UAP();

    virtual
    ~UAP();

    unsigned long m_nUseIfBitSet;
    unsigned long m_nUseIfByteNr;
    unsigned char m_nIsSetTo;
    std::list<UAPItem *> m_lUAPItems;

    UAPItem *newUAPItem(); //!< creates and returns new UAP Item
#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions();
#endif

    std::string getDataItemIDByUAPfrn(int uapfrn);
};

#endif /* UAP_H_ */
