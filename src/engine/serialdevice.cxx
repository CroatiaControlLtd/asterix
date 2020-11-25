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
#include <fcntl.h>
#include <termios.h>
#include <ctype.h>
#include <stdlib.h> //atoi
#include <errno.h>
#include <sys/select.h> // fd_set


// Local includes
#include "asterix.h"
#include "serialdevice.hxx"


CSerialDevice::CSerialDevice(CDescriptor &descriptor) {
    const char *sdevice = descriptor.GetFirst();
    const char *sbaudRate = descriptor.GetNext();
    int speed;


    // Device name argument
    if (sdevice == NULL) {
        LOGERROR(1, "Serial device name not specified\n");
    } else {
        LOGINFO(gVerbose, "Opening serial device %s\n", sdevice);
    }

    // Baud rate argument

    ConvertSpeed(_defaultBaudRate, _baudRate);  // Firstly, set the baud rate to default
    if (sbaudRate == NULL) {
        LOGWARNING(gVerbose, "Serial port baud rate not specified. Using default: %d\n", _defaultBaudRate);
    } else {
        speed = atoi(sbaudRate);
        if (!ConvertSpeed(speed, _baudRate)) {
            LOGWARNING(1, "Specified baud rate (%d) is not valid. Using default baud rate: %d\n", speed,
                       _defaultBaudRate);
        } else {
            LOGINFO(gVerbose, "Serial port baud rate set to %d\n", speed);
        }
    }

    Init(sdevice);
}


CSerialDevice::~CSerialDevice() {
    if (_fileDesc >= 0) {
        close(_fileDesc);
    }

}

/* Read all available data but not moer than value in *len.
 * Return number of bytes read in *len
 */
bool CSerialDevice::Read(void *data, size_t *len) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot read due to not properly initialized interface.\n");
        CountReadError();
        *len = 0;
        return false;
    }

    // Initialize
    ssize_t bytesRead = 0;

    bytesRead = read(_fileDesc, data, *len);
    if (bytesRead < 0) {
        LOGERROR(1, "Error %d reading from serial device (%s).\n", errno, strerror(errno));
        CountReadError();
        *len = 0;
        return false;
    }

    *len = bytesRead;

    ResetReadErrors(true);
    return true;
}

bool CSerialDevice::Read(void *data, size_t len) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot read due to not properly initialized interface.\n");
        CountReadError();
        return false;
    }

    // Initialize
    ssize_t bytesRead = 0;
    size_t bytesLeft = len;
    char *buffer = (char *) data;

    // Read all required bytes (len) in a loop
    do {
        bytesRead = read(_fileDesc, buffer, bytesLeft);
        if (bytesRead < 0) {
            LOGERROR(1, "Error %d reading from serial device (%s).\n", errno, strerror(errno));
            CountReadError();
            return false;
        } else {
            bytesLeft -= bytesRead;
            buffer += bytesRead;
        }
        if (bytesLeft > 0)
            // TODO: Solve this call to select with a more generic algorithm
            // This is just workaround for testing purposes
            Select(0); // WARNING: Wait for some more characters
    } while (bytesLeft > 0);


    ResetReadErrors(true);
    return true;
}


bool CSerialDevice::Write(const void *data, size_t len) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot write due to not properly initialized interface.\n");
        CountWriteError();
        return false;
    }

    // Write the message to the standard output (blocking)
    ssize_t bytesWrote = write(_fileDesc, data, len);
    if (bytesWrote != (ssize_t)len) {
        LOGERROR(1, "Error writing to serial device.\n");
        CountWriteError();
        return false;
    }

    ResetWriteErrors(true);
    return true;
}


bool CSerialDevice::Select(const unsigned int secondsToWait) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot select due to not properly initialized interface.\n");
        return false;
    }

    // Configure 'set' of single stdout file descriptor
    fd_set descToRead;
    int selectVal;
    FD_ZERO(&descToRead);
    FD_SET(_fileDesc, &descToRead);

    if (secondsToWait) {
        // Set the timeout as specified
        struct timeval timeout;
        timeout.tv_sec = secondsToWait;
        timeout.tv_usec = 0;

        selectVal = select(_fileDesc + 1, &descToRead, NULL, NULL, &timeout);
    } else {
        // secondsToWait is zero => Wait indefinitely
        selectVal = select(_fileDesc + 1, &descToRead, NULL, NULL, NULL);
    }

    return (selectVal == 1);
}


bool CSerialDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
    static bool result;

    switch (command) {
        case EReset:
            result = false;
            break;
        case EAllDone:
            //TODO
            result = true;
            break;
        case EPacketDone:
            //TODO
            result = false;
            break;
        default:
            result = false;
    }

    return result;
}


void CSerialDevice::Init(const char *device) {
    _opened = false;
    _fileDesc = -1;
    ResetAllErrors();

    // Open the serial device
    _fileDesc = open(device, O_RDWR | O_NOCTTY);
    if (_fileDesc < 0) {
        LOGERROR(1, "Cannot open serial device %s\n", device);
        return;
    }

/*    // Set file descriptor as blocking
    int fd_flags = fcntl(_fileDesc, F_GETFL, 0);
    int fd_flags_block = fd_flags & ~FNDELAY;  // Blocking on
    int fd_flags_noblock = fd_flags | FNDELAY; // Blocking off
    fcntl(_fileDesc, F_SETFL, fd_flags_block);*/


    // Get the current termios settings
    struct termios my_termios;
    if (tcgetattr(_fileDesc, &my_termios) < 0) {
        LOGERROR(1, "Cannot get the termios attributes.\n");
        return;
    }
    LOGDEBUG(ZONE_SERIALDEVICE, "old cflag=%08x\n", my_termios.c_cflag);
    LOGDEBUG(ZONE_SERIALDEVICE, "old oflag=%08x\n", my_termios.c_oflag);
    LOGDEBUG(ZONE_SERIALDEVICE, "old iflag=%08x\n", my_termios.c_iflag);
    LOGDEBUG(ZONE_SERIALDEVICE, "old lflag=%08x\n", my_termios.c_lflag);
#ifndef __APPLE__
    // c_line is not defined on OS X
    LOGDEBUG(ZONE_SERIALDEVICE, "old line=%02x\n", my_termios.c_line);
#endif

    // Flush the serial port
    tcflush(_fileDesc, TCIFLUSH);

    // Set required termios settings
    my_termios.c_cflag = B9600 | CS8 | CREAD | CLOCAL | HUPCL;
    cfsetospeed(&my_termios, _baudRate);
    cfsetispeed(&my_termios, _baudRate);
    if (tcsetattr(_fileDesc, TCSANOW, &my_termios) < 0) {
        LOGERROR(1, "Cannot set the termios attributes.\n");
        return;
    } else {
        LOGINFO(gVerbose, "Serial port set to: 8 data-bits, no parity, 1 stop bit, no flow control.\n");
    }
    LOGDEBUG(ZONE_SERIALDEVICE, "new cflag=%08x\n", my_termios.c_cflag);
    LOGDEBUG(ZONE_SERIALDEVICE, "new oflag=%08x\n", my_termios.c_oflag);
    LOGDEBUG(ZONE_SERIALDEVICE, "new iflag=%08x\n", my_termios.c_iflag);
    LOGDEBUG(ZONE_SERIALDEVICE, "new lflag=%08x\n", my_termios.c_lflag);
#ifndef __APPLE__
    // c_line is not defined on OS X
    LOGDEBUG(ZONE_SERIALDEVICE, "new line=%02x\n", my_termios.c_line);
#endif

    _opened = true;
}


bool CSerialDevice::ConvertSpeed(int speed, speed_t &baudRate) {
    switch (speed) {
        case 600:
            baudRate = B600;
            break;
        case 1200:
            baudRate = B1200;
            break;
        case 1800:
            baudRate = B1800;
            break;
        case 2400:
            baudRate = B2400;
            break;
        case 4800:
            baudRate = B4800;
            break;
        case 9600:
            baudRate = B9600;
            break;
        case 19200:
            baudRate = B19200;
            break;
        case 38400:
            baudRate = B38400;
            break;
        case 57600:
            baudRate = B57600;
            break;
        case 115200:
            baudRate = B115200;
            break;
        case 230400:
            baudRate = B230400;
            break;
        default:
            return false;
    }
    return true;
}
