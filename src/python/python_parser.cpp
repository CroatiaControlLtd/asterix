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
#include <sys/time.h>

static AsterixDefinition *pDefinition = NULL;
static InputParser *inputParser = NULL;
bool gFiltering = false;
bool gSynchronous = false;
const char *gAsterixDefinitionsFile = NULL;
bool gVerbose = false;
bool gForceRouting = false;
int gHeartbeat = 0;

static void debug_trace(char const *format, ...) {
    /* TODO
    char buffer[1024];
    va_list args;
    va_start (args, format);
    vsnprintf (buffer, 1023, format, args);
    va_end (args);
    strcat(buffer, "\n");
    LOGERROR(1, "%s", buffer); // TODO
    */
    // TODO PyErr_SetString(PyExc_RuntimeError, buffer);
}

/*
 * Initialize Asterix Python with XML configuration file
 */
int python_init(const char *xml_config_file) {
    Tracer::Configure(debug_trace);

    if (!pDefinition)
        pDefinition = new AsterixDefinition();

    if (!inputParser)
        inputParser = new InputParser(pDefinition);

    FILE *fp = fopen(xml_config_file, "rt");
    if (!fp) {
        PyErr_SetString(PyExc_IOError, "Input file not found.");
        return -1;
    }
    // parse format file
    XMLParser Parser;
    if (!Parser.Parse(fp, pDefinition, xml_config_file)) {
        fclose(fp);
        return -2;
    }
    fclose(fp);
    return 0;
}

PyObject *python_parse(const unsigned char *pBuf, unsigned int len, int verbose) {
    // get current timstamp in ms since epoch
    struct timeval tp;
    gettimeofday(&tp, NULL);
    unsigned long nTimestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    if (inputParser) {
        AsterixData *pData = inputParser->parsePacket(pBuf, len, nTimestamp);
        if (pData) { // convert to Python format
            PyObject *lst = pData->getData(verbose);
            delete pData;
            return lst;
        }
    }
    return NULL;
}

PyObject *
python_parse_with_offset(const unsigned char *pBuf, unsigned int len, unsigned int offset, unsigned int blocks_count,
                         int verbose)
/* AUTHOR: Krzysztof Rutkowski, ICM UW, krutk@icm.edu.pl
*/
{
    // get current timstamp in ms since epoch
    struct timeval tp;
    gettimeofday(&tp, NULL);
    unsigned long nTimestamp = tp.tv_sec * 1000 + tp.tv_usec / 1000;

    if (inputParser) {
        AsterixData *pData = new AsterixData();
        unsigned int m_nPos = offset;
        unsigned int current_blocks_count = 0;
        while (m_nPos < len && current_blocks_count < blocks_count) {
            unsigned int m_nDataLength = len - m_nPos;
            while (m_nDataLength > 3 && current_blocks_count < blocks_count) {
                const unsigned char *pBuf_offset = (pBuf + m_nPos);
                DataBlock *block = inputParser->parse_next_data_block(
                        pBuf_offset, m_nPos, len, nTimestamp, m_nDataLength);
                if (block) {
                    pData->m_lDataBlocks.push_back(block);
                    current_blocks_count++;
                }
            }
        }
        if (pData) { // convert to Python format
            PyObject *lst = pData->getData(verbose);
            delete pData;
            PyObject *py_m_nPos = Py_BuildValue("l", m_nPos);
            PyObject *py_output = PyTuple_Pack(2, lst, py_m_nPos);
            return py_output;
        }
    }
    return NULL;
}

PyObject *python_describe(int category, const char *item = NULL, const char *field = NULL, const char *value = NULL) {
    if (!pDefinition)
        return Py_BuildValue("s", "Not initialized");

    const char *description = pDefinition->getDescription(category, item, field, value);
    if (description == NULL)
        return Py_BuildValue("s", "");
    return Py_BuildValue("s", description);

/*
    Category* cat = pDefinition->getCategory(category);
    if (!cat)
    {
        return Py_BuildValue("s", "Unknown category");
    }

    if (item == NULL && field == NULL && value == NULL)
    {   // return Category description
        return Py_BuildValue("s", cat->m_strName.c_str());
    }

	std::list<DataItemDescription*>::iterator it;
	DataItemDescription* di = NULL;

    std::string item_number = format("%s", &item[1]);
	for ( it=cat->m_lDataItems.begin() ; it != cat->m_lDataItems.end(); it++ )
    {
        di = (DataItemDescription*)(*it);
        if (di->m_strID.compare(item_number) == 0)
            break;
        di = NULL;
    }
    if (di == NULL)
        return Py_BuildValue("s", "Unknown item");

    if (field == NULL && value == NULL)
    { // Return Item name and description
        return Py_BuildValue("s", (di->m_strName+" ("+di->m_strDefinition+" )").c_str());
    }

    if (value == NULL)
    {
        return Py_BuildValue("s", "field todo");
    }
    return Py_BuildValue("s", "value todo");
*/
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
