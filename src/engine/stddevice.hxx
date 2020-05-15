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
#ifndef STDDEVICE_HXX__
#define STDDEVICE_HXX__

#include "basedevice.hxx"

/**
 * @class CStdDevice
 *
 * @brief The standard input/output device.
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 */
class CStdDevice : public CBaseDevice {
private:

public:

    /**
     * Default class constructor
     */
    CStdDevice();

    /**
     * Class destructor.
     */
    virtual ~CStdDevice();

    virtual bool Read(void *data, size_t len);

    virtual bool Write(const void *data, size_t len);

    virtual bool Select(const unsigned int secondsToWait);

    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0) { return false; }

    virtual bool IsPacketDevice() { return false; }

    virtual unsigned int BytesLeftToRead() { return 0; } // return number of bytes left to read or 0 if unknown
};

#endif

