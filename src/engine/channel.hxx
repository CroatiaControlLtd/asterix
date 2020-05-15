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
#ifndef CHANNEL_HXX__
#define CHANNEL_HXX__

#include <time.h>

#include "basedevice.hxx"
#include "devicefactory.hxx"
#include "baseformatdescriptor.hxx" //class CBaseFormatDescriptor;


/**
 * @class CChannel
 * 
 * @brief Channel defines device-format relationship
 *        
 */
class CChannel {
private:
    // Device-format parameters
    unsigned int _DeviceNo;
    unsigned int _FormatNo;
    CBaseFormatDescriptor *_FormatDescriptor;

    // Failover parameters
    bool _bFailover;

    // Heartbeat parameters
    bool _bHeartbeat;
    int _HeartbeatInterval;
    int _HeartbeatMode;
    time_t _LastHeartbeatTime;

public:

    // enum used for HeartbeatMode parameter
    enum {
        EAlways,
        ENoTraffic
    };

    CChannel(unsigned int DeviceNo, unsigned int FormatNo, CBaseFormatDescriptor *FormatDescriptor,
             bool bFailover,
             bool bHeartbeat, int heartbeatInterval, int heartbeatMode)
            : _DeviceNo(DeviceNo), _FormatNo(FormatNo), _FormatDescriptor(FormatDescriptor),
              _bFailover(bFailover),
              _bHeartbeat(bHeartbeat), _HeartbeatInterval(heartbeatInterval),
              _HeartbeatMode(heartbeatMode) { _LastHeartbeatTime = 0; }

    ~CChannel() {
        // if (_FormatDescriptor != NULL) delete _FormatDescriptor;
    }

    unsigned int GetDeviceNo() { return _DeviceNo; }

    unsigned int GetFormatNo() { return _FormatNo; }

    CBaseFormatDescriptor *GetFormatDescriptor() { return _FormatDescriptor; }

    bool IsFailoverChannel() { return _bFailover; }

    bool IsHeartbeat() { return _bHeartbeat; }

    int GetHeartbeatInterval() { return _HeartbeatInterval; }

    time_t GetLastHeartbeatTime() { return _LastHeartbeatTime; }

    void SetLastHeartbeatTime(time_t newTime) { _LastHeartbeatTime = newTime; };

    void ResetHeartbeatTime() { _LastHeartbeatTime = time(NULL); }

    int GetHeartbeatMode() { return _HeartbeatMode; }

    bool Reset() {
        CBaseDevice *pDevice = CDeviceFactory::Instance()->GetDevice(GetDeviceNo());
        if (pDevice == NULL) {
            LOGERROR(1, "Reset() - Device not installed.\n");
            return false;
        }

        ResetHeartbeatTime();
        return pDevice->IoCtrl(CBaseDevice::EReset);
    }
};

#endif
