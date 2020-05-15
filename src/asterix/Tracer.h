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

#ifndef TRACER_H_
#define TRACER_H_

#include <stdlib.h>

typedef int(*ptExtPrintf)(char const *, ...);

typedef void(*ptExtVoidPrintf)(char const *, ...);

class Tracer {
public:
    Tracer();

    static void Error(const char *format, ...);

    static void Configure(ptExtPrintf pFunc);

    static void Configure(ptExtVoidPrintf pFunc);

    static void Delete();

    static Tracer *g_TracerInstance;

    static Tracer &instance();

    ptExtPrintf pPrintFunc;
    ptExtVoidPrintf pPrintFunc2;
};

#endif /* TRACER_H_ */
