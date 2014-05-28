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
