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
#ifndef DEVICEFACTORY_HXX__
#define DEVICEFACTORY_HXX__

#include "singleton.hxx"

class CBaseDevice;


/**
 * @class CDeviceFactory
 * 
 * @brief The class responsible for registering input and output devices
 *        and providing unified access to them.
 *
 * @see   <CSingleton>
 */
class CDeviceFactory {
public:

    static const unsigned int MAX_DEVICES = 11;

private:

    // Singleton pattern
    static CSingleton<CDeviceFactory> _Instance;

    // To access private Ctor...
    friend class CSingleton<CDeviceFactory>;

    CBaseDevice *_Device[MAX_DEVICES];
    unsigned int _nDevices;

public:

    /**
     * Returns the pointer to the only instance of the Factory
     * (Singleton pattern). If the instance does not yet
     * exist (on a first call), it is created 
     *
     * @return Pointer to the only instance of the Factory.
     *
     * @see <CSingleton>::<Instance>
     */
    static CDeviceFactory *Instance() { return _Instance.Instance(); }

    /**
     * Deletes the one and only instance of the Factory.
     * (Singleton pattern).
     *
     * @see <CSingleton>::<DeleteInstance>
     */
    static void DeleteInstance() { _Instance.DeleteInstance(); }

    CDeviceFactory();

    ~CDeviceFactory();

    bool CreateDevice(const char *deviceName, const char *deviceDescriptor, unsigned int &deviceNo);

    unsigned int GetNDevices() { return _nDevices; }

    CBaseDevice *GetDevice(unsigned int DeviceNo) { return _Device[DeviceNo]; }

};

#endif

