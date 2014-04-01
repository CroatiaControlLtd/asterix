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

#include <stdio.h>
#include <string.h>

#include "XMLParser.h"
#include "Tracer.h"

#include "DataItemFormatFixed.h"

/*!
 * Handling of CDATA
 */
void XMLParser::CharacterHandler(void *userData, const XML_Char *s, int len)
{
  XMLParser* p = (XMLParser*)userData;

  if (p == NULL)
  {
    return;
  }

  if (p->m_pstrCData != NULL)
  {
    // assign string value
	p->m_pstrCData->append(s,len);
  }
  else if (p->m_pintCData != NULL)
  {
    std::string tmpstr(s,len);
    *(p->m_pintCData) = atoi(tmpstr.c_str());
    p->m_pintCData = NULL;
  }
}

bool XMLParser::GetAttribute(const char* elementName, const char* attrName, std::string* ptrString)
{
  if (!strcmp(elementName, attrName))
  {
    if (ptrString == NULL)
    {
      Tracer::Error("XMLParser : Attribute parsing error");
    }
    else
    {
      GetCData(ptrString);
    }
    return true;
  }
  return false;
}

bool XMLParser::GetAttribute(const char* elementName, const char* attrName, int* ptrInt)
{
  if (!strcmp(elementName, attrName))
  {
    if (ptrInt == NULL)
    {
      Tracer::Error("XMLParser : Attribute parsing error");
    }
    else
    {
      GetCData(ptrInt);
    }
    return true;
  }
  return false;
}

void XMLParser::Error(const char* errstr, const char* param1)
{
  std::string tmpstr(errstr);
  tmpstr += param1;
  Error(tmpstr.c_str());
}

void XMLParser::Error(const char* errstr)
{
  char strLine[32];
  sprintf(strLine," line: %d", (int)XML_GetCurrentLineNumber(m_Parser));
  std::string tmpstr(errstr);
  tmpstr += strLine;
  tmpstr += "\n";
  Tracer::Error((char*)tmpstr.c_str());

  m_bErrorDetectedStopParsing = true;
}

/*!
 * Handling of element start
 */
void  XMLParser::ElementHandlerStart(void *data, const char *el, const char **attr)
{
  int i;
  XMLParser* p = (XMLParser*)data;

  if (p == NULL)
  {
    Tracer::Error("Missing Parser!");
    return;
  }

  if (p->m_bErrorDetectedStopParsing)
    return;

  if (!strcmp(el, "Category"))
  { // <!ELEMENT Category (CatID,CatName,CatVer,DataItemDescription*,UAP)>
    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "id"))
      { // <!ATTLIST id CDATA #REQUIRED>
        int id = atoi(attr[i+1]);

        if (id >= 0 && id <= 255)
        {
          p->m_pCategory = p->m_pDef->getCategory(id);
        }
        else
        {
          p->Error("XMLParser : Wrong category: ", attr[i+1]);
        }
      }
      else if (!strcmp(attr[i], "name"))
      { // <!ATTLIST name CDATA #REQUIRED>
        p->m_pCategory->m_strName = attr[i+1];
      }
      else if (!strcmp(attr[i], "ver"))
      { // <!ATTLIST ver CDATA #REQUIRED>
        p->m_pCategory->m_strVer = attr[i+1];
      }
      else
      {
        p->Error("XMLParser : Unknown attribute: ", attr[i]);
      }
    }
  }
  else if (!strcmp(el, "DataItem"))
  { // <!ELEMENT DataItemDescription (DataItemName,DataItemDefinition,DataItemFormat,DataItemNote?)>
    if (p->m_pCategory == NULL)
    {
      p->Error("XMLParser : <DataItem> without <Category> ");
    }
    else
    {
      for (i = 0; attr[i]; i += 2)
      {
        if (!strcmp(attr[i], "id"))
        { //<!ATTLIST DataItemDescription id CDATA #REQUIRED>
          int id = atoi(attr[i+1]);

          if (id >= 0)
          {
            p->m_pDataItem = p->m_pCategory->getDataItemDescription(id);
          }
          else
          {
            p->Error("XMLParser : Wrong DataItem id: ", attr[i+1]);
          }
        }
        else if (!strcmp(attr[i], "rule"))
        { // <!ATTLIST DataItemDescription rule (mandatory|optional|unknown) "unknown">
          if (p->m_pDataItem)
          {
            if (!strcmp(attr[i+1], "optional"))
              p->m_pDataItem->m_eRule = DataItemDescription::DATAITEM_OPTIONAL;
            else if (!strcmp(attr[i+1], "mandatory"))
              p->m_pDataItem->m_eRule = DataItemDescription::DATAITEM_MANDATORY;
            else
              p->m_pDataItem->m_eRule = DataItemDescription::DATAITEM_UNKNOWN;
          }
          else
          {
            p->Error("XMLParser : DataItem missing for rule");
          }
        }
        else
        {
          p->Error("XMLParser : Unknown attribute: ", attr[i]);
        }
      }
    }
  }
  else if (p->GetAttribute(el, "DataItemName", p->m_pDataItem ? &p->m_pDataItem->m_strName : NULL))
  { // <!ELEMENT DataItemName (#PCDATA)>
  }
  else if (p->GetAttribute(el, "DataItemDefinition", p->m_pDataItem ? &p->m_pDataItem->m_strDefinition : NULL))
  { // <!ELEMENT DataItemDefinition (#PCDATA)>
  }
  else if (p->GetAttribute(el, "DataItemNote", p->m_pDataItem ? &p->m_pDataItem->m_strNote : NULL))
  { // <!ELEMENT DataItemNote (#PCDATA)>
  }
  else if (!strcmp(el, "DataItemFormat"))
  { // <!ELEMENT DataItemFormat (Fixed|Variable|Compound)>
    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "desc"))
      { // <!ATTLIST DataItemFormat desc CDATA "" >
        if (p->m_pDataItem)
        {
          p->m_pDataItem->m_strFormat = attr[i+1];
        }
        else
        {
          p->Error("XMLParser : <Format> without <DataItem>");
        }
      }
      else
      {
        p->Error("XMLParser : Unknown attribute for <DataItemFormat>: ", attr[i]);
      }
    }
  }
  else if (!strcmp(el, "Fixed"))
  { // <!ELEMENT Fixed (Bits+)>
    if (!p->m_pDataItem)
    {
      p->Error("XMLParser : <Fixed> without <DataItem>");
      return;
    }

    p->m_pFormatFixed = new DataItemFormatFixed();
    if (p->m_pFormatVariable != NULL)
    {
      p->m_pFormatVariable->m_lParts.push_back(p->m_pFormatFixed);
    }
    else if (p->m_pFormatRepetitive != NULL)
    {
	  if (p->m_pFormatRepetitive->m_pFixed)
	  {
		Tracer::Error("XMLParser : Duplicate item");
	  }
      p->m_pFormatRepetitive->m_pFixed = p->m_pFormatFixed;
    }
    else if (p->m_pFormatExplicit != NULL)
    {
      if (p->m_pFormatExplicit->m_pFixed)
      {
      	Tracer::Error("XMLParser : Duplicate item");
      }
      p->m_pFormatExplicit->m_pFixed = p->m_pFormatFixed;
    }
    else if (p->m_pFormatCompound != NULL)
    {
      if (p->m_pFormatCompound->m_pCompoundPrimary == NULL)
      {
        p->Error("XMLParser : First part of <Compound> must be <Variable> and not <Fixed>");
        return;
      }
      p->m_pFormatCompound->m_lParts.push_back(p->m_pFormatFixed);
    }
    else
    {
      if (p->m_pDataItem->m_pFormat)
      {
    	Tracer::Error("XMLParser : Duplicate item");
      }
      p->m_pDataItem->m_pFormat = p->m_pFormatFixed;
      p->m_pFormat = p->m_pFormatFixed;
    }

    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "length"))
      { // <!ATTLIST Fixed length CDATA "1" >
        int len = atoi(attr[i+1]);

        if (len >= 0)
        {
          p->m_pFormatFixed->m_nLength = len;
        }
        else
        {
          p->Error("XMLParser : Wrong DataItem length: ", attr[i+1]);
        }
      }
      else
      {
        p->Error("XMLParser : Unknown attribute for Fixed: ", attr[i]);
      }
    }
  }
  else if (!strcmp(el, "Explicit"))
  { // <!ELEMENT Explicit (Bits+)>
    if (!p->m_pDataItem)
    {
      p->Error("XMLParser : <Explicit> without <DataItem>");
      return;
    }

    p->m_pFormatExplicit = new DataItemFormatExplicit();
    if (p->m_pFormatCompound != NULL)
    {
      if (p->m_pFormatCompound->m_pCompoundPrimary == NULL)
      {
        p->Error("XMLParser : First part of <Compound> must be <Variable> and not <Explicit>");
        return;
      }
      p->m_pFormatCompound->m_lParts.push_back(p->m_pFormatExplicit);
    }
    else
    {
      p->m_pDataItem->m_pFormat = p->m_pFormatExplicit;
      p->m_pFormat = p->m_pFormatExplicit;
    }
  }
  else if (!strcmp(el, "Repetitive"))
  { // <!ELEMENT Repetitive (Bits+)>
    if (!p->m_pDataItem)
    {
      p->Error("XMLParser : <Repetitive> without <DataItem>");
      return;
    }

    p->m_pFormatRepetitive = new DataItemFormatRepetitive();
    if (p->m_pFormatCompound != NULL)
    {
      if (p->m_pFormatCompound->m_pCompoundPrimary == NULL)
      {
        p->Error("XMLParser : First part of <Compound> must be <Variable> and not <Repetitive>");
        return;
      }
      p->m_pFormatCompound->m_lParts.push_back(p->m_pFormatRepetitive);
    }
    else
    {
      p->m_pDataItem->m_pFormat = p->m_pFormatRepetitive;
      p->m_pFormat = p->m_pFormatRepetitive;
    }
  }
  else if (!strcmp(el, "Variable"))
  { // <!ELEMENT Variable (Part+)>
    if (!p->m_pDataItem)
    {
      p->Error("XMLParser : <Variable> without <DataItem>");
      return;
    }

    p->m_pFormatVariable = new DataItemFormatVariable();
    if (p->m_pFormatCompound != NULL)
    {
      if (p->m_pFormatCompound->m_pCompoundPrimary == NULL)
      {
        p->m_pFormatCompound->m_pCompoundPrimary = p->m_pFormatVariable;
      }
      else
      {
        p->m_pFormatCompound->m_lParts.push_back(p->m_pFormatVariable);
      }
    }
    else
    {
      p->m_pDataItem->m_pFormat = p->m_pFormatVariable;
      p->m_pFormat = p->m_pFormatVariable;
    }
  }
  else if (!strcmp(el, "Compound"))
  { // <!ELEMENT Compound (CPart+)>
    if (!p->m_pDataItem)
    {
      p->Error("XMLParser : <Compound> without <DataItem>");
      return;
    }

    p->m_pFormatCompound = new DataItemFormatCompound();
    p->m_pDataItem->m_pFormat = p->m_pFormatCompound;
    p->m_pFormat = p->m_pFormatCompound;
  }
  else if (!strcmp(el, "Bits"))
  {
    if (!p->m_pFormat)
    {
      p->Error("XMLParser : <Bits> without <Format>");
      return;
    }

    p->m_pBits = new DataItemBits();
    p->m_pFormat->addBits(p->m_pBits);

    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "bit"))
      { // <!ATTLIST Bits bit CDATA "0">

        int bit = atoi(attr[i+1]);

        if (bit >= 0)
        {
          p->m_pBits->m_nFrom = p->m_pBits->m_nTo = bit;
          if (p->m_pFormatFixed)
          {
            if (p->m_pFormatFixed->m_nLength*8 < bit)
            {
              p->Error("XMLParser : Bit out of fixed length");
            }
          }
        }
        else
        {
          p->Error("XMLParser : Wrong bit: ", attr[i+1]);
        }
      }
      else if (!strcmp(attr[i], "from"))
      { // <!ATTLIST Bits from CDATA "0">
        int bit = atoi(attr[i+1]);

        if (bit >= 0)
        {
          p->m_pBits->m_nFrom = bit;
          if (p->m_pFormatFixed)
          {
            if (p->m_pFormatFixed->m_nLength*8 < bit)
            {
              p->Error("XMLParser : Bit out of fixed length");
            }
          }
        }
        else
        {
          p->Error("XMLParser : Wrong bit from: ", attr[i+1]);
        }
      }
      else if (!strcmp(attr[i], "to"))
      { // <!ATTLIST Bits to CDATA "0">
        int bit = atoi(attr[i+1]);

        if (bit >= 0)
        {
          p->m_pBits->m_nTo = bit;
          if (p->m_pFormatFixed)
          {
            if (p->m_pFormatFixed->m_nLength*8 < bit)
            {
              p->Error("XMLParser : Bit out of fixed length");
            }
          }
        }
        else
        {
          p->Error("XMLParser : Wrong bit to: ", attr[i+1]);
        }
      }
      else if (!strcmp(attr[i], "encode"))
      { // <!ATTLIST Bits encode (signed|6bitschar|octal|unsigned|ascii|hex) "unsigned">
        if (!strcmp(attr[i+1], "unsigned"))
        {
          p->m_pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_UNSIGNED;
        }
        else if (!strcmp(attr[i+1], "6bitschar"))
        {
          p->m_pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIX_BIT_CHAR;
        }
		else if (!strcmp(attr[i+1], "hex"))
        {
          p->m_pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_HEX_BIT_CHAR;
        }
        else if (!strcmp(attr[i+1], "octal"))
        {
          p->m_pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_OCTAL;
        }
        else if (!strcmp(attr[i+1], "signed"))
        {
          p->m_pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_SIGNED;
        }
        else if (!strcmp(attr[i+1], "ascii"))
        {
          p->m_pBits->m_eEncoding = DataItemBits::DATAITEM_ENCODING_ASCII;
        }
        else
        {
          p->Error("XMLParser : Wrong encode: ", attr[i]);
        }
      }
      else if (!strcmp(attr[i], "fx"))
      {
        p->m_pBits->m_bExtension = atoi(attr[i+1]) ? true : false;
      }
      else
      {
        p->Error("XMLParser : Unknown attribute: ", attr[i]);
      }
    }
  }
  else if (p->GetAttribute(el, "BitsShortName", p->m_pBits ? &p->m_pBits->m_strShortName : NULL))
  { // <!ELEMENT BitsShortName (#PCDATA)>
  }
  else if (p->GetAttribute(el, "BitsName", p->m_pBits ? &p->m_pBits->m_strName : NULL))
  { // <!ELEMENT BitsName (#PCDATA)>
  }
  else if (p->GetAttribute(el, "BitsPresence", p->m_pBits ? &p->m_pBits->m_nPresenceOfField : NULL))
  { // <!ELEMENT CPBitsPresence (#PCDATA)>
  }
  else if (p->GetAttribute(el, "BitsID", p->m_pBits ? &p->m_pBits->m_strXIDEF :
                                        p->m_pFormatFixed ? &p->m_pFormatFixed->m_strXIDEF :
                                        p->m_pFormat ? &p->m_pFormat->m_strXIDEF :
                                        NULL))
  { // <!ELEMENT XIDIF (#PCDATA)>
	  // TODO remove
  }
  else if (!strcmp(el, "BitsValue"))
  { // <!ELEMENT BitsValue (#PCDATA)>
    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "val"))
      { // <!ATTLIST BitsValue val CDATA "0" >
        int val = atoi(attr[i+1]);
        p->m_pBitsValue = new BitsValue(val);
        p->m_pBits->m_lValue.push_back(p->m_pBitsValue);
      }
/* TODO <!ATTLIST BitsValue from CDATA "0" > <!ATTLIST BitsValue to CDATA "0" >
      else if (!strcmp(attr[i], "from"))
      { // <!ATTLIST BitsValue from CDATA "0" >
        int val = atoi(attr[i+1]);
        p->m_pBitsValue = new BitsValue(val); // TODO from
        p->m_pBits->m_lValue.push_back(p->m_pBitsValue);
      }
      else if (!strcmp(attr[i], "to"))
      { // <!ATTLIST BitsValue to CDATA "0" >
        int val = atoi(attr[i+1]);
        p->m_pBitsValue = new BitsValue(val);// TODO to
        p->m_pBits->m_lValue.push_back(p->m_pBitsValue);
      }
*/
    }
    p->GetAttribute(el, "BitsValue", p->m_pBitsValue ? &p->m_pBitsValue->m_strDescription : NULL);
  }
  else if (!strcmp(el, "BitsUnit"))
  { // <!ELEMENT BitsUnit (#PCDATA)> <!ELEMENT PBitsUnit (#PCDATA)> <!ELEMENT CPBitsUnit (#PCDATA)>
    if (p->m_pBits)
    {
      for (i = 0; attr[i]; i += 2)
      {
        if (!strcmp(attr[i], "scale"))
        { // <!ATTLIST BitsUnit scale CDATA "1" >
          p->m_pBits->m_dScale = atof(attr[i+1]);
        }
        else if (!strcmp(attr[i], "min"))
        { // <!ATTLIST BitsUnit min CDATA "0" >
          p->m_pBits->m_dMinValue = atof(attr[i+1]);
          p->m_pBits->m_bMinValueSet = true;
        }
        else if (!strcmp(attr[i], "max"))
        { // <!ATTLIST BitsUnit max CDATA "0" >
          p->m_pBits->m_dMaxValue = atof(attr[i+1]);
          p->m_pBits->m_bMaxValueSet = true;
        }
      }

      p->GetAttribute(el, "BitsUnit", &p->m_pBits->m_strUnit);
    }
    else
    {
      p->Error("XMLParser : Missing Bits for BitsUnit");
    }
  }
  else if (!strcmp(el, "BitsConst"))
  {
    if (p->m_pBits)
    {
      p->m_pBits->m_bIsConst = true;
      p->GetAttribute(el, "BitsConst", &p->m_pBits->m_nConst);
    }
    else
    {
      p->Error("XMLParser : Missing Bits for BitsConst");
    }
  }
  else if (!strcmp(el, "UAP"))
  {
    if (p->m_pCategory == NULL)
    {
      p->Error("XMLParser : Missing <UAP> outside <Category>");
      return;
    }

    p->m_pUAP = p->m_pCategory->newUAP();

    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "use_if_bit_set"))
      { // <!ATTLIST UAP use_if_bit_set CDATA "0">
        p->m_pUAP->m_nUseIfBitSet = atoi(attr[i+1]);
      }
      else if (!strcmp(attr[i], "use_if_byte_nr"))
      { // <!ATTLIST UAP use_if_byte_nr CDATA "0">
        p->m_pUAP->m_nUseIfByteNr = atoi(attr[i+1]);
      }
      else if (!strcmp(attr[i], "is_set_to"))
      { // <!ATTLIST UAP is_set_to CDATA "0">
        p->m_pUAP->m_nIsSetTo = atoi(attr[i+1]);
      }
      else
      {
        p->Error("XMLParser : Unknown property for UAP: ", attr[i+1]);
      }
    }
  }
  else if (!strcmp(el, "UAPItem"))
  { // <!ELEMENT UAPItem (#PCDATA)>
    if (p->m_pCategory == NULL)
    {
      p->Error("XMLParser : Missing <UAPItem> outside <Category>");
      return;
    }
    if (p->m_pUAP == NULL)
    {
      p->Error("XMLParser : Missing <UAPItem> outside <UAP>");
      return;
    }

    p->m_pUAPItem = p->m_pUAP->newUAPItem();

    for (i = 0; attr[i]; i += 2)
    {
      if (!strcmp(attr[i], "bit"))
      { // <!ATTLIST UAPItem bit CDATA #REQUIRED>
        p->m_pUAPItem->m_nBit = atoi(attr[i+1]);
      }
      else if (!strcmp(attr[i], "frn"))
      { // <!ATTLIST UAPItem frn CDATA #REQUIRED>
        if (!strcmp(attr[i+1], "FX"))
        {
          p->m_pUAPItem->m_bFX = true;
        }
        else
        {
          p->m_pUAPItem->m_bFX = false;
          p->m_pUAPItem->m_nFRN = atoi(attr[i+1]);
        }
      }
      else if (!strcmp(attr[i], "len"))
      { // <!ATTLIST UAPItem len CDATA #REQUIRED>
        p->m_pUAPItem->m_nLen = atoi(attr[i+1]);
      }
      else
      {
        p->Error("XMLParser : Unknown property for UAPItem: ", attr[i+1]);
      }
    }

    p->GetAttribute(el, "UAPItem", p->m_pUAPItem ? &p->m_pUAPItem->m_nItemID : NULL);
  }
  else
  {
    p->Error("Unknown tag: ", el);
  }
}

/*!
 * Handling of element end
 */
void XMLParser::ElementHandlerEnd(void *data, const char *el)
{
  XMLParser* p = (XMLParser*)data;

  if (p == NULL)
  {
    Tracer::Error("Missing Parser!");
    return;
  }

  if (p->m_bErrorDetectedStopParsing)
    return;

  if (!strcmp(el, "Category"))
  {
    if (p->m_pCategory)
    {
      p->m_pCategory = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "DataItem"))
  {
    if (p->m_pDataItem)
    {
      p->m_pDataItem = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "Fixed"))
  {
    if (p->m_pFormatFixed)
    {
      p->m_pFormatFixed = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "Variable"))
  {
    if (p->m_pFormatVariable)
    {
      p->m_pFormatVariable = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "Explicit"))
  {
    if (p->m_pFormatExplicit)
    {
      p->m_pFormatExplicit = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "Repetitive"))
  {
    if (p->m_pFormatRepetitive)
    {
      p->m_pFormatRepetitive = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "Compound"))
  {
    if (p->m_pFormatCompound)
    {
      p->m_pFormatCompound = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "Bits"))
  {
    if (p->m_pBits)
    {
      p->m_pBits = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "BitsValue"))
  {
    if (p->m_pBitsValue)
    {
      p->m_pBitsValue = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "UAP"))
  {
    if (p->m_pUAP)
    {
      p->m_pUAP = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }
  else if (!strcmp(el, "UAPItem"))
  {
    if (p->m_pUAPItem)
    {
      p->m_pUAPItem = NULL;
    }
    else
    {
      p->Error("Closing unopened tag: ", el);
    }
  }

  p->m_pstrCData = NULL;
  p->m_pintCData = NULL;
}

/*!
 * XMLParser constructor
 */
XMLParser::XMLParser()
: m_bErrorDetectedStopParsing(false)
, m_pDef(NULL)
, m_pCategory(NULL)
, m_pDataItem(NULL)
, m_pFormat(NULL)
, m_pFormatFixed(NULL)
, m_pFormatExplicit(NULL)
, m_pFormatRepetitive(NULL)
, m_pFormatVariable(NULL)
, m_pFormatCompound(NULL)
, m_pBits(NULL)
, m_pUAPItem(NULL)
, m_pstrCData(NULL)
, m_pintCData(NULL)
{
  m_Parser = XML_ParserCreate(NULL);
  if (!m_Parser)
  {
    Tracer::Error("Couldn't allocate memory for parser");
  }

  XML_SetElementHandler(m_Parser, ElementHandlerStart, ElementHandlerEnd);

  XML_SetCharacterDataHandler(m_Parser, CharacterHandler);

  XML_SetUserData(m_Parser, this);

}

/*!
 * XMLParser destructor
 */
XMLParser::~XMLParser()
{
  XML_ParserFree(m_Parser);
}

/*!
 * Parse XML file and fill definition object
 */
bool XMLParser::Parse(FILE* pFile, AsterixDefinition* pDefinition)
{
  m_pDef = pDefinition;

  for (;;)
  {
     int done;
     int len;

     len = (int)fread(m_pBuff, 1, BUFFSIZE, pFile);
     if (ferror(pFile))
     {
       Tracer::Error("Format file read error.");
       return false;
     }
     done = feof(pFile);

     if (XML_Parse(m_Parser, m_pBuff, len, done) == XML_STATUS_ERROR)
     {
       std::string tmpStr("Format file parse error: ");
       tmpStr += XML_ErrorString(XML_GetErrorCode(m_Parser));
       Error(tmpStr.c_str());
       return false;
     }

     if (done)
       break;
   }

  return !m_bErrorDetectedStopParsing;
}
