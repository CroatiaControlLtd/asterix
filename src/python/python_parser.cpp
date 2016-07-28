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

#define LOGDEBUG(cond, ...)
#define LOGERROR(cond, ...)

#include "python_parser.h"
#include "AsterixDefinition.h"
#include "XMLParser.h"
#include "InputParser.h"

static AsterixDefinition* pDefinition = NULL;
static InputParser *inputParser = NULL;
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
    if (!pDefinition)
        pDefinition = new AsterixDefinition();

    if (!inputParser)
        inputParser = new InputParser(pDefinition);

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

PyObject *python_parse(const unsigned char* pBuf, unsigned int len)
{
    if (inputParser)
    {
        AsterixData* pData = inputParser->parsePacket(pBuf, len);
        if (pData)
        { // convert to Python format
          PyObject *lst = pData->getData();
          delete pData;
          return lst;
        }
    }
    return NULL;
}

/*
	CAsterixFormatDescriptor& Descriptor((CAsterixFormatDescriptor&)formatDescriptor);
	PyObject *lst = Descriptor.m_pAsterixData->getData();
	PyObject *arg = Py_BuildValue("(O)", lst);
	PyObject *result = PyObject_CallObject(my_callback, arg);
	Py_DECREF(lst);
	if (result != NULL)
		/// use result...
		Py_DECREF(result);
	return true;
*/