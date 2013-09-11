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

#include "UAPItem.h"

UAPItem::UAPItem()
: m_nBit(0)
, m_nFRN(0)
, m_bFX(false)
, m_nLen(0)
, m_nItemID(0)
{
}

UAPItem::~UAPItem()
{
}

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
fulliautomatix_definitions* UAPItem::getWiresharkDefinitions()
{
  fulliautomatix_definitions* def = new fulliautomatix_definitions;

  def->pid = getPID();

  if (m_bFX)
  {
    def->name = strdup("Field extension");
    def->abbrev = strdup("FX");
  }
  else
  {
    char tmp[128];

    sprintf(tmp, "Item %d", m_nItemID);
    def->name = strdup(tmp);

    sprintf(tmp, "UAP%d", m_nItemID);
    def->abbrev = strdup(tmp);
  }
  def->type = FA_FT_UINT8;
  def->display = FA_BASE_DEC;
  def->bitmask = 0x01;
  def->bitmask <<= (7-m_nBit%8);

  def->strings = (fulliautomatix_value_string*)malloc(3 * sizeof(fulliautomatix_value_string));
  def->strings[0].value = 0;
  def->strings[0].strptr = strdup(" Not present");
  def->strings[1].value = 1;
  def->strings[1].strptr = strdup(" Present");
  def->strings[2].value = 0;
  def->strings[2].strptr = NULL;

  def->blurb = NULL;
  def->next = NULL;
  return def;
}

fulliautomatix_data* UAPItem::getData(unsigned char* pData, long len, int byteoffset)
{
  int byteShift = m_nBit/8;
  if (byteShift<len)
    return newDataUL(NULL, getPID(), byteoffset+byteShift, 1, *(pData+byteShift));
  return NULL;
}
#endif
