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

#include "Category.h"

Category::Category(int id)
        : m_id(id), m_bFiltered(false) {
}

Category::~Category() {
    // destroy data items
    std::list<DataItemDescription *>::iterator it = m_lDataItems.begin();
    while (it != m_lDataItems.end()) {
        delete (DataItemDescription *) (*it);
        it = m_lDataItems.erase(it);
    }

    // destroy UAPs
    std::list<UAP *>::iterator it2 = m_lUAPs.begin();
    while (it2 != m_lUAPs.end()) {
        delete (UAP *) (*it2);
        it2 = m_lUAPs.erase(it2);
    }
}

DataItemDescription *Category::getDataItemDescription(std::string id) {
    std::list<DataItemDescription *>::iterator it;
    DataItemDescription *di = NULL;

    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        di = (DataItemDescription *) (*it);
        if (di->m_strID == id) {
            return di;
        }
    }

    // create new DataItemDescription
    di = new DataItemDescription(id);
    m_lDataItems.push_back(di);

    return di;
}

const char *Category::getDescription(const char *item, const char *field = NULL, const char *value = NULL) {
    std::list<DataItemDescription *>::iterator it;
    DataItemDescription *di = NULL;

    std::string item_number = format("%s", &item[1]);

    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        di = (DataItemDescription *) (*it);
        if (di->m_strID.compare(item_number) == 0) {
            if (field == NULL)
                return di->m_strName.c_str();
            return di->m_pFormat->getDescription(field, value);
        }
    }
    return NULL;
}

UAP *Category::newUAP() {
    UAP *uap = new UAP();
    m_lUAPs.push_back(uap);
    return uap;
}

UAP *Category::getUAP(const unsigned char *data, unsigned long len) {
    std::list<UAP *>::iterator uapit;
    for (uapit = m_lUAPs.begin(); uapit != m_lUAPs.end(); uapit++) {
        UAP *uap = (UAP *) (*uapit);

        if (uap) {
            if (uap->m_nUseIfBitSet) { // check if bit matches
                unsigned long bittomatch = uap->m_nUseIfBitSet;

                unsigned long pos = 0;

                // skip FSPEC
                while (pos < len && (data[pos] & 0x01))
                    pos++;

                pos++;

                pos += (bittomatch - 1) / 8;

                unsigned char mask = 0x01;
                mask <<= (7 - (bittomatch - 1) % 8);

                if (pos < len && (data[pos] & mask))
                    return uap;
            } else if (uap->m_nUseIfByteNr) { // check if byte matches
                unsigned long pos = 0;

                // skip FSPEC
                while (pos < len && (data[pos] & 0x01))
                    pos++;

                pos++;

                pos += uap->m_nUseIfByteNr - 1;

                if (pos < len && data[pos] == uap->m_nIsSetTo) {
                    return uap;
                }
            } else { // no need to match
                return uap;
            }
        }
    }
    return NULL;
}

std::string Category::printDescriptors() {
    std::string strDef = "";
    char header[32];

    std::list<DataItemDescription *>::iterator it;
    DataItemDescription *di = NULL;

    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        di = (DataItemDescription *) (*it);

        snprintf(header, 32, "CAT%03d:I%s:", m_id, di->m_strID.c_str());

        strDef += di->m_pFormat->printDescriptors(header);
    }

    return strDef;
}

bool Category::filterOutItem(std::string item, const char *name) {
    std::list<DataItemDescription *>::iterator it;
    DataItemDescription *di = NULL;

    // At least one item of category shall be printed when filter is applied
    m_bFiltered = true;

    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        di = (DataItemDescription *) (*it);
        if (di->m_strID == item) {
            return di->m_pFormat->filterOutItem(name);
        }
    }
    return false;
}

bool Category::isFiltered(std::string item, const char *name) {
    std::list<DataItemDescription *>::iterator it;
    DataItemDescription *di = NULL;

    for (it = m_lDataItems.begin(); it != m_lDataItems.end(); it++) {
        di = (DataItemDescription *) (*it);
        if (di->m_strID == item) {
            if (true == di->m_pFormat->isFiltered(name))
                return true;
        }
    }
    return false;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* Category::getWiresharkDefinitions()
{
  fulliautomatix_definitions* startDef = NULL;
  fulliautomatix_definitions* def = NULL;

  // get definitions for UAPs
  std::list<UAP*>::iterator uapit;
  for ( uapit=m_lUAPs.begin() ; uapit != m_lUAPs.end(); uapit++ )
  {
    UAP* ui = (UAP*)(*uapit);
    if (def)
    {
      def->next = ui->getWiresharkDefinitions();
    }
    else
    {
      startDef = def = ui->getWiresharkDefinitions();
    }
    while(def->next)
      def = def->next;
  }

  // get definitions for items
  std::list<DataItemDescription*>::iterator it;
  DataItemDescription* di = NULL;

  for ( it=m_lDataItems.begin() ; it != m_lDataItems.end(); it++ )
  {
    di = (DataItemDescription*)(*it);
    if (def)
    {
      def->next = di->m_pFormat->getWiresharkDefinitions();
    }
    else
    {
      startDef = def = di->m_pFormat->getWiresharkDefinitions();
    }
    while(def->next)
    {
      def = def->next;
    }
  }

  return startDef;
}
#endif
