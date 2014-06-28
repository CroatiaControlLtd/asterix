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


#include <Python.h>
#include <WiresharkWrapper.h>

static int bInitialized = 0;
static fulliautomatix_definitions** pDefinitions = NULL;
static fulliautomatix_definitions* pListOfDefinitions = NULL;

static fulliautomatix_data* getList(fulliautomatix_data* p, PyObject* head);

PyObject*
say_hello(PyObject* self, PyObject* args, PyObject *kwargs)
{
    const char* name;

    if (!PyArg_ParseTuple(args, "s", &name))
        return NULL;

    printf("Hello %s!\n", name);

    Py_RETURN_NONE;
}

PyObject*
init(PyObject* self, PyObject* args, PyObject *kwargs)
{
    const char* filename;

    if (!PyArg_ParseTuple(args, "s", &filename))
        return NULL;

	int ret = fulliautomatix_start(printf, filename);
	if (ret == 0)
		bInitialized = 1;

	pListOfDefinitions = fulliautomatix_get_definitions();

	// find maximal pid
	int maxpid = 0;
	fulliautomatix_definitions* d = pListOfDefinitions;
	while(d != NULL )
	{
		if (d->pid > maxpid)
			maxpid = d->pid;
		d = d->next;
	}

	// create array of definitions
	pDefinitions = (fulliautomatix_definitions**) malloc((maxpid+1)*sizeof(fulliautomatix_definitions*));
	memset(pDefinitions, (maxpid+1)*sizeof(fulliautomatix_definitions*), 0);

	// sort definitions in array
	d = pListOfDefinitions;
	while(d != NULL)
	{
		pDefinitions[d->pid] = d;
		d = d->next;
	}

	printf("Total definitions = %d", maxpid);

    Py_RETURN_NONE;
}

PyObject*
parse(PyObject* self, PyObject* args, PyObject *kwargs)
{
    const char* data;
    int len;
    int i;

    if (!PyArg_ParseTuple(args, "s#", &data, &len))
        return NULL;

	if (!bInitialized)
	{
	    printf("Not initialized!");
	    return NULL;
	}

	fulliautomatix_data* parsed = fulliautomatix_parse((const unsigned char*) data, len);


	PyObject *lstBlocks = PyList_New(0);
	getList(parsed, lstBlocks);
	fulliautomatix_data_destroy(parsed);

	return lstBlocks;

}

fulliautomatix_data* getList(fulliautomatix_data* p, PyObject* lst)
{
	// create holder for record category
	char* name = NULL;

	int dataBlock = 0;
	PyObject* dataRecord = NULL;
	PyObject* lstItems = NULL;
	PyObject* dataItem = NULL;
	PyObject* lstValues = NULL;

	while(p != NULL)
	{
		if (p->tree == 1)
		{
			PyObject* item = PyDict_New();
			PyObject *sublist = PyList_New(0);
			if (name != NULL)
				PyDict_SetItemString(item, name, sublist);
			else
				PyDict_SetItemString(item, p->description, sublist);
			PyList_Append(lst, item);
			p = p->next;
			p = getList(p, sublist);


/*
			if (!dataBlock)
			{
				dataBlock = 1;
			}
			else if (dataRecord == NULL)
			{
				dataRecord = PyDict_New();
				lstItems = PyList_New(0);
				PyDict_SetItemString(dataRecord, recordCat, lstItems);

				if (PyList_Append(lst, dataRecord))
				{
					printf("Error in list append");
					Py_DECREF(lst);
					Py_RETURN_NONE;
				}
			}
			else if (dataItem == NULL)
			{
				dataItem = PyDict_New();
				lstValues = PyList_New(0);
				PyDict_SetItemString(dataItem, "Ixxx", lstValues);

				if (PyList_Append(lstItems, dataItem))
				{
					printf("Error in list append");
					Py_DECREF(lstItems);
					Py_RETURN_NONE;
				}
			}
*/
		}
		else if (p->tree == -1)
		{
			if (name)
				free(name);
			return p;
/*
			if (dataItem != NULL)
			{
				dataItem = NULL;
			}
			else if (dataRecord != NULL)
			{
				dataRecord = NULL;
			}
			else if (dataBlock)
			{
				dataBlock = 0;
			}
*/
		}
		else if (p->description != NULL)
		{
			printf("Description: %s\n", p->description);
		}
		else if (p->pid == 0)
		{
			// create record category
			PyObject* block = PyDict_New();
			char tmp[7];
			sprintf(tmp, "CAT%03d", p->val.ul);
			if (name)
				free(name);
			name = strdup(tmp);
/*
			// create list of record in block
			lstRecords = PyList_New(0);
			PyDict_SetItemString(block, name, lstBlocks);

//			    PyObject *num = PyLong_FromLong(p->val.ul);
			if (PyList_Append(lstBlocks, record))
			{
				printf("Error in list append");
				Py_DECREF(lstBlocks);
				Py_RETURN_NONE;
			}
*/
		}
		else
		{
			char* name = pDefinitions[p->pid]->name;
			char* abbrev = pDefinitions[p->pid]->abbrev;

			PyObject* item = PyDict_New();
			if (p->type == FA_FT_UINT_STRING)
			{
				PyDict_SetItemString(item, abbrev, PyString_FromString(p->val.str));
				printf("\t%s\t\t%s\t%s\n", abbrev, p->val.str, name);
			}
			else
			{
				PyDict_SetItemString(item, abbrev, PyLong_FromLong(p->val.ul));
				printf("\t%s\t\t%ld\t%s\n", abbrev, p->val.ul, name);
			}
			PyList_Append(lst, item);
		}
		p = p->next;
	}

	if (name)
		free(name);
	return p;
}
