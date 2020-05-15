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

#ifndef PYTHONPARSER_H_
#define PYTHONPARSER_H_

#include <Python.h>

#ifdef __cplusplus
extern "C" {
#endif

int python_start(const char *ini_file_path);
int python_init(const char *ini_file_path);
PyObject *python_describe(int category, const char *item, const char *field, const char *value);
PyObject *python_parse(const unsigned char *pBuf, unsigned int len, int verbose);
PyObject *
python_parse_with_offset(const unsigned char *pBuf, unsigned int len, unsigned int offset, unsigned int blocks_count,
                         int verbose);
void asterix_start(const char *ini_filename, const char *filename);

#ifdef __cplusplus
}
#endif

#endif /* PYTHONPARSER_H_ */
