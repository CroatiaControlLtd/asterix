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
 * AUTHORS: Jurica Baricevic, Croatia Control Ltd.
 *
 */
// Standard includes
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/time.h>
#include <unistd.h>
#include <sys/select.h> // fd_set

// Local includes
#include "asterix.h"
#include "stddevice.hxx"


CStdDevice::CStdDevice() {
    _opened = true;  // STDIO/STDOUT are always opened
}


CStdDevice::~CStdDevice() {
}


bool CStdDevice::Read(void *data, size_t len) {
    // Read a message from the standard input (blocking)
    ssize_t bytesLeftToRead = len;
    char* pData = (char*)data;

    while(bytesLeftToRead > 0) {
        ssize_t bytesRead = read(STDIN_FILENO, pData, bytesLeftToRead);
        if (bytesRead < 0) {
            LOGERROR(1, "Error reading from stdin.\n");
            CountReadError();
            return false;
        } else if (bytesRead == 0) {
            _opened = false;
            return false;
        }
        pData += bytesRead;
        bytesLeftToRead -= bytesRead;
    }
    _onstart = false;

    ResetReadErrors(true);
    return true;
}


bool CStdDevice::Write(const void *data, size_t len) {
    // Write the message to the standard output (blocking)
    ssize_t bytesLeft = (ssize_t)len;
    char *pData = (char *) data;

    while (bytesLeft > 0) {
        ssize_t bytesWrote = write(STDOUT_FILENO, pData, bytesLeft);

        if (bytesWrote < 0) {
            LOGERROR(1, "Error writing to stdout.\n");
            CountWriteError();
            return false;
        }
        bytesLeft -= bytesWrote;
        pData += bytesWrote;
    }

    ResetWriteErrors(true);
    return true;
}


bool CStdDevice::Select(const unsigned int secondsToWait) {
    fd_set descToRead;
    int selectVal;

    while (1) {
        // Configure 'set' of single stdin file descriptor
        FD_ZERO(&descToRead);
        FD_SET(STDIN_FILENO, &descToRead);

        if (secondsToWait) {
            // Set the timeout as specified
            struct timeval timeout;
            timeout.tv_sec = secondsToWait;
            timeout.tv_usec = 0;

            selectVal = select(STDIN_FILENO + 1, &descToRead, NULL, NULL, &timeout);
        } else {
            // secondsToWait is zero => Wait indefinitely
            selectVal = select(STDIN_FILENO + 1, &descToRead, NULL, NULL, NULL);
        }

        if (selectVal <= 0 || FD_ISSET(STDIN_FILENO, &descToRead)) {
            break;
        }
    }

    return (selectVal == 1);
}
