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

#include "AsterixDefinition.h"
#include <memory.h>

AsterixDefinition::AsterixDefinition()
{
  for (int i=0; i<MAX_CATEGORIES; i++)
  {
    m_pCategory[i] = NULL;
  }
}

AsterixDefinition::~AsterixDefinition()
{
  for (int i=0; i<MAX_CATEGORIES; i++)
  {
    if (m_pCategory[i] != NULL)
    {
      delete m_pCategory[i];
    }
  }
}

Category* AsterixDefinition::getCategory(int i)
{
  if (m_pCategory[i] == NULL)
  {
    m_pCategory[i] = new Category(i);
  }

  return m_pCategory[i];
}

Category* AsterixDefinition::newCategory(int i)
{
  if (m_pCategory[i] != NULL)
  {
    delete m_pCategory[i];
    m_pCategory[i] = NULL;
  }
  return getCategory(i);
}

bool AsterixDefinition::CategoryDefined(int i)
{
  return (m_pCategory[i] != NULL) ? true : false;
}

std::string AsterixDefinition::printDescriptors()
{
	std::string strDef = "";
	int i = 1;

	for (i=1; i<MAX_CATEGORIES-1; i++)
	{
		if (CategoryDefined(i))
		{
			strDef += getCategory(i)->printDescriptors();
		}
	}
	return strDef;
}

bool AsterixDefinition::filterOutItem(int cat, std::string item, const char* name)
{
	if (CategoryDefined(cat))
	{
		return getCategory(cat)->filterOutItem(item, name);
	}
	return false;
}

bool AsterixDefinition::isFiltered(int cat, std::string item, const char* name)
{
	if (CategoryDefined(cat))
	{
		return getCategory(cat)->isFiltered(item, name);
	}
	return false;
}

const char* AsterixDefinition::getDescription(int category, const char* item=NULL, const char* field=NULL, const char* value=NULL)
{
    if (m_pCategory[category] != NULL)
    {
        if(item == NULL && field == NULL && value == NULL)
            return m_pCategory[category]->m_strName.c_str();
        return m_pCategory[category]->getDescription(item, field, value);
    }
    return NULL;
}
