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
#ifndef CATEGORY_H_
#define CATEGORY_H_

#include "DataItemDescription.h"
#include "UAP.h"
#include <string>
#include <list>

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
#include "WiresharkWrapper.h"
#endif

class Category {
public:
    Category(int id);

    virtual
    ~Category();

    unsigned int m_id;
    bool m_bFiltered; //! at least one item of category shall be printed when filter is applied

    std::string m_strName;
    std::string m_strVer;

    std::list<DataItemDescription *> m_lDataItems;
    std::list<UAP *> m_lUAPs;

    DataItemDescription *
    getDataItemDescription(std::string id); //!< returns requested data item or creates new if not existing
    UAP *newUAP(); //!< creates and returns new UAP
    UAP *getUAP(const unsigned char *data, unsigned long len); // get UAP that matches condition
    std::string printDescriptors(); // print item descriptors
    bool filterOutItem(std::string item, const char *name);

    bool isFiltered(std::string item, const char *name);

    const char *
    getDescription(const char *item, const char *field, const char *value); //!< return item string definition

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
    fulliautomatix_definitions* getWiresharkDefinitions(); //!< returns linked list of Wireshark definitions
#endif
};

#endif /* CATEGORY_H_ */
