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

//#include <stdlib.h>
//#include <memory.h>
#include <stdio.h>
#include <string>

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)

#include "WiresharkWrapper.h"
#include "AsterixDefinition.h"
#include "XMLParser.h"
#include "InputParser.h"


static AsterixDefinition* pDefinition = NULL;
bool gFiltering = false;
bool gSynchronous = false;

/*
 * Initialize Asterix Wireshark wrapper
 */
int fulliautomatix_start(ptExtVoidPrintf pPrintFunc, const char* ini_file_path)
{
  std::string strFilePath;
  std::string strInputFile;
  char inputFile[256];

  int ret = 0;
  // initialize Fulliautomatix
  Tracer::Configure(pPrintFunc);
  pDefinition = new AsterixDefinition();

  if (ini_file_path)
  {
    strFilePath = ini_file_path;
#ifdef LINUX
    strFilePath += "/";
#else
    strFilePath += "\\";
#endif
  }

  strInputFile = strFilePath + "asterix.ini";

  // open asterix.ini file
  FILE* fpini = fopen(strInputFile.c_str(), "rt");
  if (!fpini)
  {
    Tracer::Error("Failed to open %s\n", strInputFile.c_str());
    return 1;
  }
  while(fgets(inputFile, sizeof(inputFile), fpini))
  {
    // remove trailing /n from filename
    int lastChar = strlen(inputFile)-1;
    while (lastChar>=0 && (inputFile[lastChar] == '\r' || inputFile[lastChar] == '\n'))
    {
      inputFile[lastChar] = 0;
      lastChar--;
    }
    if (lastChar <= 0)
      continue;

    strInputFile = strFilePath + inputFile;

    FILE* fp = fopen(strInputFile.c_str(), "rt");
    if (!fp)
    {
      Tracer::Error("Failed to open definitions file: %s\n", inputFile);
      continue;
    }

    // parse format file
    XMLParser Parser;
    if (!Parser.Parse(fp, pDefinition, strInputFile.c_str()))
    {
      Tracer::Error("Failed to parse definitions file: %s\n", strInputFile.c_str());
      ret = 1;
    }

    fclose(fp);
  };

  fclose(fpini);

  return ret;
}

static fulliautomatix_definitions* newDefinition(fulliautomatix_definitions* prev, int pid, char *name, char *abbrev, int type, int display, void *strings, unsigned long bitmask, const char *blurb)
{
  fulliautomatix_definitions* newdef = (fulliautomatix_definitions*)malloc(sizeof(fulliautomatix_definitions));
  memset(newdef,0,sizeof(fulliautomatix_definitions));

  newdef->pid = pid;
  newdef->name = strdup(name);
  newdef->abbrev = strdup(abbrev);
  newdef->type = type;
  newdef->display = display;
  newdef->strings = (fulliautomatix_value_string*)strings;
  newdef->bitmask = bitmask;
  newdef->blurb = strdup(blurb);
  if (prev)
    prev->next = newdef;
  return newdef;
}

fulliautomatix_definitions* fulliautomatix_get_definitions()
{
  fulliautomatix_definitions* first_def = NULL;
  fulliautomatix_definitions* def = NULL;

  first_def = def = newDefinition(NULL, PID_CATEGORY, (char*)"Category       ",(char*)"asterix.category", FA_FT_UINT32, FA_BASE_DEC, NULL, 0, (char*)"category");
  def = newDefinition(def, PID_LENGTH, (char*)"Payload length ", (char*)"asterix.payload_len", FA_FT_UINT32, FA_BASE_DEC, NULL, 0, (char*)"Payload length");
  def = newDefinition(def, PID_FSPEC, (char*)"FSPEC ", (char*)"asterix.fspec", FA_FT_BYTES, FA_FT_NONE, NULL, 0, (char*)"FSPEC");
  def = newDefinition(def, PID_REP, (char*)"Repetition factor ", (char*)"asterix.rep", FA_FT_UINT32, FA_BASE_DEC, NULL, 0, (char*)"REP");
  def = newDefinition(def, PID_LEN, (char*)"Data Item Length ", (char*)"asterix.len", FA_FT_UINT32, FA_BASE_DEC, NULL, 0, (char*)"LEN");

  for (int i=0; i<MAX_CATEGORIES; i++)
  {
    if (pDefinition->CategoryDefined(i))
    {
      fulliautomatix_definitions* newdefs = pDefinition->getCategory(i)->getWiresharkDefinitions();
      def->next = newdefs;
      while(def->next)
        def = def->next;
    }
  }

  return first_def;
}

void fulliautomatix_destroy_definitions(fulliautomatix_definitions* pDef)
{
  while(pDef)
  {
    if (pDef->name) free(pDef->name);
    if (pDef->abbrev) free(pDef->abbrev);
    if (pDef->blurb) free(pDef->blurb);

    if (pDef->strings)
    {
      fulliautomatix_value_string *pString = pDef->strings;
      int i=0;

      while(pString[i].strptr)
      {
        free(pString[i].strptr);
        i++;
      }
      free(pDef->strings);
    }

    fulliautomatix_definitions* pDefTmp = pDef;
    pDef = pDef->next;
    free(pDefTmp);
  }
}

fulliautomatix_data* newDataString(fulliautomatix_data* prev, int pid, int bytenr, int length, char* val)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  newdata->tree = 0;
  newdata->pid = pid;
  if (prev)
  {
    prev->next = newdata;
  }
  newdata->bytenr = bytenr;
  newdata->length = length;
  newdata->type = FA_FT_STRINGZ;
  newdata->val.str = strdup(val);
  return newdata;
}

fulliautomatix_data* newDataMessage(fulliautomatix_data* prev, int bytenr, int length, int err, char* val)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  if (prev)
  {
    prev->next = newdata;
  }
  newdata->bytenr = bytenr;
  newdata->length = length;
  newdata->type = FA_FT_NONE;
  newdata->val.str = strdup(val);
  newdata->err = err;
  return newdata;
}

fulliautomatix_data* newDataBytes(fulliautomatix_data* prev, int pid, int bytenr, int length, unsigned char* val)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  newdata->tree = 0;
  newdata->pid = pid;
  if (prev)
  {
    prev->next = newdata;
  }
  newdata->bytenr = bytenr;
  newdata->length = length;
  newdata->type = FA_FT_BYTES;
  newdata->val.str = (char*)malloc(length);
  memcpy(newdata->val.str, val, length);
  return newdata;
}

fulliautomatix_data* newDataUL(fulliautomatix_data* prev, int pid, int bytenr, int length, unsigned long val)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  newdata->tree = 0;
  newdata->pid = pid;
  if (prev)
  {
    prev->next = newdata;
  }
  newdata->bytenr = bytenr;
  newdata->length = length;
  newdata->type = FA_FT_UINT32;
  newdata->val.ul = val;
  return newdata;
}

fulliautomatix_data* newDataSL(fulliautomatix_data* prev, int pid, int bytenr, int length, signed long val)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  newdata->tree = 0;
  newdata->pid = pid;
  if (prev)
  {
    prev->next = newdata;
  }
  newdata->bytenr = bytenr;
  newdata->length = length;
  newdata->type = FA_FT_INT32;
  newdata->val.sl = val;
  return newdata;
}

fulliautomatix_data* newDataTree(fulliautomatix_data* prev, int bytenr, int length, char* description)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  newdata->tree = 1;
  if (prev)
  {
    prev->next = newdata;
  }
  newdata->bytenr = bytenr;
  newdata->length = length;
  newdata->description = strdup(description);
  return newdata;
}

fulliautomatix_data* newDataTreeEnd(fulliautomatix_data* prev, int offset)
{
  fulliautomatix_data* newdata = (fulliautomatix_data*)malloc(sizeof(fulliautomatix_data));
  memset(newdata,0,sizeof(fulliautomatix_data));

  newdata->tree = -1;
  newdata->bytenr = offset;
  if (prev)
  {
    prev->next = newdata;
  }
  return newdata;
}

fulliautomatix_data* fulliautomatix_parse(const unsigned char* pBuf, unsigned int len)
{
  InputParser inputParser(pDefinition);
  AsterixData* pData = inputParser.parsePacket(pBuf, len);
  if (pData)
  { // convert to Wireshark format
    fulliautomatix_data* data = pData->getData();

    // delete AsterixData
    delete pData;

    return data;
  }
  return NULL;
}

void fulliautomatix_data_destroy(fulliautomatix_data* pData)
{
  fulliautomatix_data* pDataTmp = pData;
  while(pDataTmp)
  {
    if (pDataTmp->description)
    {
      free(pDataTmp->description);
    }
    if (pDataTmp->value_description)
    {
      free(pDataTmp->value_description);
    }
    if (pDataTmp->type == FA_FT_STRING || pDataTmp->type == FA_FT_STRINGZ || pDataTmp->type == FA_FT_BYTES || pDataTmp->type == FA_FT_NONE)
    {
      if(pDataTmp->val.str)
      {
        free(pDataTmp->val.str);
      }
    }
    fulliautomatix_data* pDataTmp2 = pDataTmp;
    pDataTmp = pDataTmp->next;
    free(pDataTmp2);
  }
}

#endif
