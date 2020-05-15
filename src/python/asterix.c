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

#if PY_MAJOR_VERSION >= 3
#define PyInt_Check             PyLong_Check
#define PyInt_AS_LONG           PyLong_AsLong
#define PyInt_FromLong          PyLong_FromLong
#define PyString_Check          PyBytes_Check
#define PyString_GET_SIZE       PyBytes_GET_SIZE
#define PyString_AS_STRING      PyBytes_AS_STRING
#define PyString_FromString     PyUnicode_FromString
#endif

PyObject *say_hello(PyObject *self, PyObject *args, PyObject *kwargs);

PyObject *init(PyObject *self, PyObject *args, PyObject *kwargs);

PyObject *parse(PyObject *self, PyObject *args, PyObject *kwargs);

PyObject *parse_with_offset(PyObject *self, PyObject *args, PyObject *kwargs);

PyObject *describe(PyObject *self, PyObject *args, PyObject *kwargs);

PyObject *set_callback(PyObject *self, PyObject *args);

#define ENCODER_HELP_TEXT "Use ensure_ascii=false to output UTF-8. Pass in double_precision to alter the maximum digit precision of doubles. Set encode_html_chars=True to encode < > & as unicode escape sequences."

static PyMethodDef asterixMethods[] = {
        {"hello",             (PyCFunction) say_hello,         METH_VARARGS |
                                                               METH_KEYWORDS,                "Say hello" ENCODER_HELP_TEXT},
        {"init",              (PyCFunction) init,              METH_VARARGS |
                                                               METH_KEYWORDS,                "Initializes asterix converter" ENCODER_HELP_TEXT},
        {"describe",          (PyCFunction) describe,          METH_VARARGS,                 "Describe ASTERIX item" ENCODER_HELP_TEXT},
        {"parse",             (PyCFunction) parse,             METH_VARARGS |
                                                               METH_KEYWORDS,                "Parse ASTERIX data" ENCODER_HELP_TEXT},
        {"parse_with_offset", (PyCFunction) parse_with_offset, METH_VARARGS |
                                                               METH_KEYWORDS,                "Parse ASTERIX data with bytes offset" ENCODER_HELP_TEXT},
        {"set_callback",      (PyCFunction) set_callback,      METH_VARARGS,                 "Set callback function" ENCODER_HELP_TEXT},
        {NULL,                NULL,                            0,                            NULL}       /* Sentinel */
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "_asterix",
  0,              /* m_doc */
  -1,             /* m_size */
  asterixMethods,   /* m_methods */
  NULL,           /* m_reload */
  NULL,           /* m_traverse */
  NULL,           /* m_clear */
  NULL            /* m_free */
};

#define PYMODINITFUNC       PyObject *PyInit__asterix(void)
#define PYMODULE_CREATE()   PyModule_Create(&moduledef)
#define MODINITERROR        return NULL

#else

#define PYMODINITFUNC       PyMODINIT_FUNC init_asterix(void)
#define PYMODULE_CREATE()   Py_InitModule("_asterix", asterixMethods)
#define MODINITERROR        return

#endif

PYMODINITFUNC {
    PyObject *module = PYMODULE_CREATE();

    if (module == NULL) {
        MODINITERROR;
    }

#if PY_MAJOR_VERSION >= 3
    return module;
#endif
}
