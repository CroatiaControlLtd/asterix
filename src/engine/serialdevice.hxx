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
#ifndef SERIALDEVICE_HXX__
#define SERIALDEVICE_HXX__

#include <termios.h>  // speed_t

#include "basedevice.hxx"
#include "descriptor.hxx"

/**
 * @class CSerialDevice
 *
 * @brief The serial device.
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 */
class CSerialDevice : public CBaseDevice {
private:
    int _fileDesc;
    static const int _defaultBaudRate = 9600;
    speed_t _baudRate;

public:

    /**
     * Default class constructor
     */
    CSerialDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    virtual ~CSerialDevice();

    virtual bool Read(void *data, size_t len);

    virtual bool Read(void *data, size_t *len);

    virtual bool Write(const void *data, size_t len);

    virtual bool Select(const unsigned int secondsToWait);

    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0);

    virtual bool IsPacketDevice() { return false; }

    virtual unsigned int BytesLeftToRead() { return 0; } // return number of bytes left to read or 0 if unknown

private:
    void Init(const char *device);

    static bool ConvertSpeed(int speed, speed_t &baudRate);
};

#endif

