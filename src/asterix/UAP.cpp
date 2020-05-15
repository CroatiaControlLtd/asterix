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

#include "UAP.h"

UAP::UAP()
        : m_nUseIfBitSet(0), m_nUseIfByteNr(0), m_nIsSetTo(0) {
}

UAP::~UAP() {
    // destroy UAP items
    std::list<UAPItem *>::iterator it = m_lUAPItems.begin();
    while (it != m_lUAPItems.end()) {
        delete (UAPItem *) (*it);
        it = m_lUAPItems.erase(it);
    }
}

UAPItem *UAP::newUAPItem() {
    UAPItem *uap = new UAPItem();
    m_lUAPItems.push_back(uap);
    return uap;
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* UAP::getWiresharkDefinitions()
{
  fulliautomatix_definitions* startDef = NULL;
  fulliautomatix_definitions* def = NULL;

  // get definitions for UAPs
  std::list<UAPItem*>::iterator uapit;
  for ( uapit=m_lUAPItems.begin() ; uapit != m_lUAPItems.end(); uapit++ )
  {
    UAPItem* ui = (UAPItem*)(*uapit);
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

  return startDef;
}
#endif

std::string UAP::getDataItemIDByUAPfrn(int uapfrn) {
    std::list<UAPItem *>::iterator uapit;
    for (uapit = m_lUAPItems.begin(); uapit != m_lUAPItems.end(); uapit++) {
        UAPItem *ui = (UAPItem *) (*uapit);
        if (ui != NULL && ui->m_nFRN == uapfrn) {
            return ui->m_strItemID;
        }
    }
    return "";
}
