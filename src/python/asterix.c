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

#include "py_defines.h"
#include "version.h"


PyObject* say_hello(PyObject* self, PyObject *args, PyObject *kwargs);
void initAsterix(void);

#define ENCODER_HELP_TEXT "Use ensure_ascii=false to output UTF-8. Pass in double_precision to alter the maximum digit precision of doubles. Set encode_html_chars=True to encode < > & as unicode escape sequences."

static PyMethodDef asterixMethods[] = {
  {"init", (PyCFunction) say_hello, METH_VARARGS | METH_KEYWORDS, "Initializes asterix converter" ENCODER_HELP_TEXT},
  {NULL, NULL, 0, NULL}       /* Sentinel */
};

#if PY_MAJOR_VERSION >= 3

static struct PyModuleDef moduledef = {
  PyModuleDef_HEAD_INIT,
  "asterix",
  0,              /* m_doc */
  -1,             /* m_size */
  asterixMethods,   /* m_methods */
  NULL,           /* m_reload */
  NULL,           /* m_traverse */
  NULL,           /* m_clear */
  NULL            /* m_free */
};

#define PYMODINITFUNC       PyObject *PyInit_asterix(void)
#define PYMODULE_CREATE()   PyModule_Create(&moduledef)
#define MODINITERROR        return NULL

#else

#define PYMODINITFUNC       PyMODINIT_FUNC initasterix(void)
#define PYMODULE_CREATE()   Py_InitModule("asterix", asterixMethods)
#define MODINITERROR        return

#endif

PYMODINITFUNC
{
  PyObject *module;
  PyObject *version_string;

  initAsterix();

  module = PYMODULE_CREATE();

  if (module == NULL)
  {
    MODINITERROR;
  }

  version_string = PyString_FromString (PYASTERIX_VERSION);
  PyModule_AddObject (module, "__version__", version_string);

#if PY_MAJOR_VERSION >= 3
  return module;
#endif
}
