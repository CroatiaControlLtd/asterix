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
//#include <stdio.h>
//#include <string>

#define LOGDEBUG(cond, ...)
#define LOGERROR(cond, ...)


#include <Python.h>

#include "python_parser.h"
#include "AsterixDefinition.h"
#include "XMLParser.h"
#include "InputParser.h"
#include "../engine/converterengine.hxx"
#include "../engine/channelfactory.hxx"


static AsterixDefinition* pDefinition = NULL;
bool gFiltering = false;
bool gSynchronous = false;
const char* gAsterixDefinitionsFile = NULL;
bool gVerbose = false;
bool gForceRouting = false;
int gHeartbeat = 0;


/*
 * Initialize Asterix Python with XML configuration file
 */
int python_init(const char* xml_config_file)
{
    FILE* fp = fopen(xml_config_file, "rt");
    if (!fp)
    {
      return -11;
    }
    // parse format file
    XMLParser Parser;
    if (!Parser.Parse(fp, pDefinition, xml_config_file))
    {
        fclose(fp);
        return -2;
    }
    fclose(fp);
    return 0;
}

/*
 * Initialize Asterix Python wrapper
 */
int python_start(const char* ini_file_path)
{
  std::string strFilePath;
  std::string strInputFile;
  char inputFile[256];

  int ret = 0;
  // initialize Fulliautomatix
//  Tracer::Configure(pPrintFunc);
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

  gAsterixDefinitionsFile = strInputFile.c_str();


  // open asterix.ini file
  FILE* fpini = fopen(strInputFile.c_str(), "rt");
  if (!fpini)
  {
//    Tracer::Error("Failed to open %s\n", strInputFile.c_str());
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
//      Tracer::Error("Failed to open definitions file: %s\n", inputFile);
      continue;
    }

    // parse format file
    XMLParser Parser;
    if (!Parser.Parse(fp, pDefinition, strInputFile.c_str()))
    {
//      Tracer::Error("Failed to parse definitions file: %s\n", strInputFile.c_str());
      ret = 1;
    }

    fclose(fp);
  };

  fclose(fpini);

  return ret;
}


PyObject *python_parse(const unsigned char* pBuf, unsigned int len)
{
  InputParser inputParser(pDefinition);
  AsterixData* pData = inputParser.parsePacket(pBuf, len);
  if (pData)
  { // convert to Python format

	  PyObject *lst = pData->getData();
    // delete AsterixData
	  delete pData;
	  return lst;
  }
  return NULL;
}

