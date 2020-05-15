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
#include <stdio.h>
#include <string.h>

#include "asterix.h"
#include "devicefactory.hxx"
#include "basedevice.hxx"
#include "tcpdevice.hxx"
#include "udpdevice.hxx"
#include "diskdevice.hxx"
#include "stddevice.hxx"
#include "serialdevice.hxx"


CSingleton<CDeviceFactory> CDeviceFactory::_Instance;


CDeviceFactory::CDeviceFactory() {
    for (unsigned int i = 0; i < MAX_DEVICES; i++) {
        _Device[i] = NULL;
    }

    _nDevices = 0;
}


CDeviceFactory::~CDeviceFactory() {
    for (unsigned int i = 0; i < MAX_DEVICES; i++) {
        if (_Device[i]) {
            delete _Device[i];
        }
    }

}


bool CDeviceFactory::CreateDevice(const char *deviceName, const char *deviceDescriptor, unsigned int &deviceNo) {
    // Check for free slots
    if (_nDevices >= MAX_DEVICES) {
        // No free slots
        LOGERROR(1, "Maximum number of devices has been reached. Cannot create new device slot.\n");
        return false;
    }

    // Search for specified device and create it
    if (strcasecmp(deviceName, "tcp") == 0) {
        CDescriptor descriptor(deviceDescriptor, ":");
        _Device[_nDevices] = new CTcpDevice(descriptor);
    } else if (strcasecmp(deviceName, "udp") == 0) {
        CDescriptor descriptor(deviceDescriptor, "@");
        _Device[_nDevices] = new CUdpDevice(descriptor);
    } else if (strcasecmp(deviceName, "std") == 0) {
        _Device[_nDevices] = new CStdDevice();  // no need for descriptor currently
    } else if (strcasecmp(deviceName, "disk") == 0) {
        CDescriptor descriptor(deviceDescriptor, "|");
        _Device[_nDevices] = new CDiskDevice(descriptor);
    } else if (strcasecmp(deviceName, "serial") == 0) {
        CDescriptor descriptor(deviceDescriptor, ":");
        _Device[_nDevices] = new CSerialDevice(descriptor);
    } else {
        LOGERROR(1, "Unknown device '%s'\n", deviceName);
        _Device[_nDevices] = NULL; // redundant since it already must have that value
    }


    // Check for memory allocation failure
    if (_Device[_nDevices] == NULL) {
        return false;
    }

    // Check if the device was properly initialized
    if (!_Device[_nDevices]->IsOpened()) {
        return false;
    }

    // Device successfully installed
    deviceNo = _nDevices++;
    return true;
}
