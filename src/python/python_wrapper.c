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

    if (!PyArg_ParseTuple(args, "s#", &data, &len))
        return NULL;

	if (!bInitialized)
	{
	    printf("Not initialized!");
	    return NULL;
	}

	fulliautomatix_data* parsed = fulliautomatix_parse((const unsigned char*) data, len);
	fulliautomatix_data* p = parsed;
	while(p != NULL)
	{
		if (p->description != NULL)
		{
			printf("\n%s", p->description);
		}
		else
		{
			char* name = pDefinitions[p->pid]->name;
			char* abbrev = pDefinitions[p->pid]->abbrev;

			if (p->type == FA_FT_UINT_STRING)
				printf("\n\t%s\t\t%s\t%s", abbrev, p->val.str, name);
			else
				printf("\n\t%s\t\t%ld\t%s", abbrev, p->val.ul, name);
		}
		p = p->next;
	}

	fulliautomatix_data_destroy(parsed);

    Py_RETURN_NONE;
}


