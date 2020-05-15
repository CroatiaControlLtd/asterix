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

#ifndef ASTERIXDEFINITION_H_
#define ASTERIXDEFINITION_H_

#include "Category.h"

#define MAX_CATEGORIES  256+1

// Special category ID for BDS register
#define BDS_CAT_ID    256

class AsterixDefinition {
public:
    AsterixDefinition();

    virtual
    ~AsterixDefinition();

    Category *getCategory(int i);

    void setCategory(Category *newCategory);

    bool CategoryDefined(int i);

    std::string printDescriptors();

    bool filterOutItem(int cat, std::string item, const char *name);

    bool isFiltered(int cat, std::string item, const char *name);

    const char *getDescription(int category, const char *item, const char *field, const char *value);

private:
    Category *m_pCategory[MAX_CATEGORIES];
};

#endif /* ASTERIXDEFINITION_H_ */
