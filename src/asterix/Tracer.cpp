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

#include "Tracer.h"
#include <string>
#include <stdio.h>
#include <iostream>
#include <stdarg.h>
#include <string.h>

using namespace std;

Tracer *Tracer::g_TracerInstance = NULL;

Tracer &Tracer::instance() {
    if (!Tracer::g_TracerInstance) {
        Tracer::g_TracerInstance = new Tracer();
    }
    return *Tracer::g_TracerInstance;
}

Tracer::Tracer()
        : pPrintFunc(NULL), pPrintFunc2(NULL) {
}

void Tracer::Configure(ptExtPrintf pFunc) {
    Tracer::instance().pPrintFunc = pFunc;
}

void Tracer::Configure(ptExtVoidPrintf pFunc) {
    Tracer::instance().pPrintFunc2 = pFunc;
}

void Tracer::Delete() {
    if (Tracer::g_TracerInstance)
        delete Tracer::g_TracerInstance;
    Tracer::g_TracerInstance = NULL;
}

void Tracer::Error(const char *format, ...) {
    // any length: some messages carry a whole data block in hex
    char small[1024];
    va_list args, again;
    va_start(args, format);
    va_copy(again, args);
    int n = vsnprintf(small, sizeof(small), format, args);
    va_end(args);
    std::string message;
    if (n >= (int) sizeof(small)) {
        message.resize(n);
        vsnprintf(&message[0], n + 1, format, again);
    } else if (n >= 0) {
        message = small;
    }
    va_end(again);
    const char *buffer = message.c_str();
    Tracer &instance = Tracer::instance();
    // the print functions are printf-like: buffer is already formatted, so it must not be the format
    // again (a '%' coming from the data or a file name would read arguments that do not exist)
    if (instance.pPrintFunc) {
        instance.pPrintFunc("%s", buffer);
    } else if (instance.pPrintFunc2) {
        instance.pPrintFunc2("%s", buffer);
    } else {
        puts(buffer);
    }
}
