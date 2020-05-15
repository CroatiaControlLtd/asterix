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
#include <unistd.h>
#include <stdlib.h> //atoi
#include <string.h>

#include "asterix.h"
#include "channelfactory.hxx"
#include "devicefactory.hxx"
#include "basedevice.hxx"
#include "descriptor.hxx"

#include "baseformat.hxx"
#include "asterixformat.hxx"


CSingleton<CChannelFactory> CChannelFactory::_Instance;


CChannelFactory::CChannelFactory() {
    // Create format engine.
    _formatEngine = (CBaseFormat *) new CAsterixFormat;
    LOGDEBUG(_formatEngine, "CChannelFactory successfully initialized with CAsterixFormat.\n");

    if (_formatEngine == NULL) {
        LOGERROR(1, "Fatal error - Could not initialize format engine.\n");
    }

    // Make sure that all channels are put to uninitialized state.
    _inputChannel = NULL;
    for (unsigned int i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
        _outputChannel[i] = NULL;
    }

    _nOutputChannels = 0;

    _activeFailoverOutputChannel = MAX_OUTPUT_CHANNELS;
}


CChannelFactory::~CChannelFactory() {
    if (_inputChannel) {
        delete _inputChannel;
    }

    for (unsigned int i = 0; i < MAX_OUTPUT_CHANNELS; i++) {
        if (_outputChannel[i]) {
            delete _outputChannel[i];
        }
    }

    if (_formatEngine) {
        delete _formatEngine;
    }

}


bool CChannelFactory::CreateInputChannel(const char *sDeviceName, const char *sDeviceDescriptor,
                                         const char *sFormatName, const char *sFormatDescriptor) {
    ASSERT(_formatEngine);

    // Check if input channel has been already initialized
    if (_inputChannel) {
        // Already installed
        LOGERROR(1, "Input channel has been already installed.\n");
        return false;
    }

    // Attach device to the input channel
    unsigned int deviceNo;
    if (!CDeviceFactory::Instance()->CreateDevice(sDeviceName, sDeviceDescriptor, deviceNo)) {
        LOGERROR(1, "Couldn't create device '%s'.\n", sDeviceName);
        return false;
    }

    // Attach formatter to the input channel
    unsigned int formatNo;
    CBaseFormatDescriptor *formatDesc;
    if (!CChannelFactory::Instance()->AttachFormatter(sFormatName, sFormatDescriptor, formatNo, &formatDesc)) {
        LOGERROR(1, "Couldn't attach formatter '%s'.\n", sFormatDescriptor);
        return false;
    }

    // Activate input channel
    _inputChannel = new CChannel(deviceNo, formatNo, formatDesc, false, false, 0, 0);

    return (_inputChannel != NULL);
}


bool CChannelFactory::CreateOutputChannel(const char *sDeviceName, const char *sDeviceDescriptor,
                                          const char *sFormatName, const char *sFormatDescriptor,
                                          const bool bFailover, const char *sHeartbeat) {
    ASSERT(_formatEngine);

    // Check for free output channel slots
    if (_nOutputChannels >= MAX_OUTPUT_CHANNELS) {
        // No free slots
        LOGERROR(1, "Couldn't create device '%s'. Maximum number of output channels reached.\n", sDeviceName);
        return false;
    }

    // Attach device to the output channel
    unsigned int deviceNo;
    if (!CDeviceFactory::Instance()->CreateDevice(sDeviceName, sDeviceDescriptor, deviceNo)) {
        LOGERROR(1, "Couldn't create device '%s'.\n", sDeviceName);
        return false;
    }

    // Attach formatter to the output channel
    unsigned int formatNo;
    CBaseFormatDescriptor *formatDesc;
    if (!CChannelFactory::Instance()->AttachFormatter(sFormatName, sFormatDescriptor, formatNo, &formatDesc)) {
        LOGERROR(1, "Couldn't attach formatter '%s'.\n", sFormatName);
        return false;
    }

    // Heartbeat descriptor decoding
    bool bHeartbeat = false;
    unsigned int heartbeatInterval = 0;
    unsigned int heartbeatMode = CChannel::EAlways;
    if ((sHeartbeat != NULL) && (strlen(sHeartbeat) > 0)) {
        CDescriptor heartbeatDescriptor(sHeartbeat, ":");
        const char *sHeartbeatInterval = heartbeatDescriptor.GetFirst();
        const char *sHeartbeatMode = heartbeatDescriptor.GetNext();

        if ((sHeartbeatInterval != NULL) && (strlen(sHeartbeatInterval) > 0)) {
            heartbeatInterval = abs(atoi(sHeartbeatInterval));
            bHeartbeat = true;
            // N mode means that heartbeat works only when there are no packets on output
            if ((sHeartbeatMode != NULL) && (sHeartbeatMode[0] == 'N')) {
                heartbeatMode = CChannel::ENoTraffic;
            }
            LOGINFO(gVerbose, "%d seconds heartbeat (mode %d) set on device '%s' with format '%s'.\n",
                    heartbeatInterval, heartbeatMode, sDeviceName, sFormatDescriptor);
        }
    }


    // Activate output channel
    _outputChannel[_nOutputChannels] =
            new CChannel(deviceNo, formatNo, formatDesc,
                         bFailover, bHeartbeat, heartbeatInterval, heartbeatMode);

    if (_outputChannel[_nOutputChannels] == NULL) {
        // Cannot allocate memory
        LOGERROR(1, "Not enough memory to create output device '%s'.\n", sDeviceName);
        return false;
    }

    // Channel successfully installed

    if (bFailover && (_activeFailoverOutputChannel >= MAX_OUTPUT_CHANNELS))
        _activeFailoverOutputChannel = _nOutputChannels;

    _nOutputChannels++;

    return true;
}


bool CChannelFactory::AttachFormatter(const char *sFormatName, const char *sFormatDescriptor, unsigned int &formatNo,
                                      CBaseFormatDescriptor **formatDesc) {
    LOGDEBUG(1, "AttachFormatter %s\n", sFormatName);

    if (!_formatEngine->GetFormatNo(sFormatName, formatNo)) {
        LOGERROR(1, "Unknown format '%s'.\n", sFormatName);
        return false;
    }

    // Create format descriptor
    if ((*formatDesc = _formatEngine->CreateFormatDescriptor(formatNo, sFormatDescriptor)) == NULL) {
        LOGERROR(1, "Couldn't create format descriptor'%s' for '%s'.\n", sFormatDescriptor, sFormatName);
        return false;
    }

    return true;
}


bool CChannelFactory::WaitForPacket(const unsigned int secondsToWait) {
    ASSERT(_formatEngine);

    if (_inputChannel == NULL) {
        LOGERROR(1, "Select() - Input device not installed.\n");
        return false;
    }

    // Get the reference to the input device
    CBaseDevice *inputDevice = CDeviceFactory::Instance()->GetDevice(_inputChannel->GetDeviceNo());

    if (inputDevice == NULL) {
        LOGERROR(1, "Select() - Cannot get the input device.\n");
        return false;
    }

    return inputDevice->Select(secondsToWait);
}


bool CChannelFactory::ReadPacket() {
    ASSERT(_formatEngine);

    if (_inputChannel == NULL) {
        LOGERROR(1, "ReadPacket() - Input device not installed.\n");
        return false;
    }

    // Get the reference to the input device
    CBaseDevice *inputDevice = CDeviceFactory::Instance()->GetDevice(_inputChannel->GetDeviceNo());

    if (inputDevice == NULL) {
        LOGERROR(1, "ReadPacket() - Cannot get the input device.\n");
        return false;
    }

    // Get thr format descriptor and number
    CBaseFormatDescriptor *formatDescriptor = _inputChannel->GetFormatDescriptor();
    if (formatDescriptor == NULL) {
        LOGERROR(1, "ReadPacket() - Cannot get the format descriptor.\n");
        return false;
    }

    unsigned int formatNo = _inputChannel->GetFormatNo();
    bool discard = false;

    // FormatEngine - read packet from the input device using predefined format
    return _formatEngine->ReadPacket(*formatDescriptor, *inputDevice, formatNo, discard);
}


bool CChannelFactory::WritePacket(const unsigned int outputChannel) {
    ASSERT(_formatEngine);

    if (_outputChannel[outputChannel] == NULL) {
        LOGERROR(1, "WritePacket() - Output device not installed.\n");
        return false;
    }

    // Get the reference to the output device
    CBaseDevice *outputDevice =
            CDeviceFactory::Instance()->GetDevice(_outputChannel[outputChannel]->GetDeviceNo());

    if (outputDevice == NULL) {
        LOGERROR(1, "WritePacket() - Cannot get the input device.\n");
        return false;
    }

    // Get the format descriptor and number
    CBaseFormatDescriptor *formatDescriptor = _outputChannel[outputChannel]->GetFormatDescriptor();
    if (formatDescriptor == NULL) {
        LOGERROR(1, "WritePacket() - Cannot get the format descriptor.\n");
        return false;
    }
    unsigned int formatNo = _outputChannel[outputChannel]->GetFormatNo();
    bool discard = false;

    // FormatEngine - send packet to the output device using predefined format/filter
    bool status = _formatEngine->WritePacket(*formatDescriptor, *outputDevice, formatNo, discard);

    // Reset heartbit timestamp if heartbeat is on and heartbeat mode requires so
    if (_outputChannel[outputChannel]->IsHeartbeat() &&
        (_outputChannel[outputChannel]->GetHeartbeatMode() == CChannel::ENoTraffic)) {
        _outputChannel[outputChannel]->ResetHeartbeatTime();
    }

    return status;
}


bool CChannelFactory::ProcessPacket(bool &discard) {
    ASSERT(_formatEngine);

    if (_inputChannel == NULL) {
        LOGERROR(1, "ProcessPacket() - Input device not installed.\n");
        return false;
    }

    // Get the reference to the input device
    CBaseDevice *inputDevice = CDeviceFactory::Instance()->GetDevice(_inputChannel->GetDeviceNo());

    if (inputDevice == NULL) {
        LOGERROR(1, "ProcessPacket() - Cannot get the input device.\n");
        return false;
    }

    // Get the format descriptor and number
    CBaseFormatDescriptor *formatDescriptor = _inputChannel->GetFormatDescriptor();
    if (formatDescriptor == NULL) {
        LOGERROR(1, "ProcessPacket() - Cannot get the format descriptor.\n");
        return false;
    }
    unsigned int formatNo = _inputChannel->GetFormatNo();

    // FormatEngine - process packet from the input device using predefined format
    return _formatEngine->ProcessPacket(*formatDescriptor, *inputDevice, formatNo, discard);
}


bool CChannelFactory::HeartbeatProcessing(const unsigned int outputChannel) {
    ASSERT(_formatEngine);

    if (_outputChannel[outputChannel] == NULL) {
        LOGERROR(1, "HeartbeatProcessing() - Output device not installed.\n");
        return false;
    }

    // See if channel requires heartbeat processing at all
    if (!_outputChannel[outputChannel]->IsHeartbeat()) {
        // No need to execute heartbeat, just return
        return true;
    }

    // Get current time and calculate if heartbeat interval expired
    time_t currentTime = time(NULL);
    time_t lastHeartbeatTime = _outputChannel[outputChannel]->GetLastHeartbeatTime();
    if (currentTime < 0) {
        LOGERROR(1, "HeartbeatProcessing() - Cannot get the current time with system call time().\n");
        return false;
    }
    if (currentTime < lastHeartbeatTime) {
        LOGERROR(1, "HeartbeatProcessing() - System clock changed backwards.\n");
        // Reset heartbit timestamp to remedy the system clock issue
        _outputChannel[outputChannel]->SetLastHeartbeatTime(currentTime);
        return false;
    }
    if ((currentTime - lastHeartbeatTime) < _outputChannel[outputChannel]->GetHeartbeatInterval()) {
        // No need to execute heartbeat, just return
        return true;
    }


    // Get the reference to the output device
    CBaseDevice *outputDevice =
            CDeviceFactory::Instance()->GetDevice(_outputChannel[outputChannel]->GetDeviceNo());

    if (outputDevice == NULL) {
        LOGERROR(1, "HeartbeatProcessing() - Cannot get the input device.\n");
        return false;
    }

    // Get the format descriptor and number
    CBaseFormatDescriptor *formatDescriptor = _outputChannel[outputChannel]->GetFormatDescriptor();
    if (formatDescriptor == NULL) {
        LOGERROR(1, "HeartbeatProcessing() - Cannot get the format descriptor.\n");
        return false;
    }
    unsigned int formatNo = _outputChannel[outputChannel]->GetFormatNo();

    // FormatEngine - call heart beat on the output device
    bool status = _formatEngine->HeartbeatProcessing(*formatDescriptor, *outputDevice, formatNo);

    // Reset heartbit timestamp
    _outputChannel[outputChannel]->SetLastHeartbeatTime(currentTime);

    return status;
}


int CChannelFactory::GetStatus(int query) {
    ASSERT(_formatEngine);

    // Get the reference to the input device
    CBaseDevice *inputDevice = CDeviceFactory::Instance()->GetDevice(_inputChannel->GetDeviceNo());

    if (inputDevice == NULL) {
        LOGERROR(1, "GetStatus() - Cannot get the input device.\n");
        return false;
    }

    CBaseDevice &devRef = *inputDevice;
    unsigned int formatNo = _inputChannel->GetFormatNo();

    // FormatEngine - get status for the input device and format
    return _formatEngine->GetStatus(devRef, formatNo, query);
/*
    if (_activeFailoverOutputChannel < MAX_OUTPUT_CHANNELS)
    {
        // Get the reference to the active output device
        CBaseDevice *outputDevice = CDeviceFactory::Instance()->GetDevice(_outputChannel[_activeFailoverOutputChannel]->GetDeviceNo());

        if (outputDevice == NULL)
        {
            LOGERROR(1, "GetStatus() - Cannot get the active failover output device.\n");
            return false;
        }

        devRef = *outputDevice;
        formatNo = _outputChannel[_activeFailoverOutputChannel]->GetFormatNo();
    }
    else
    {
        LOGINFO(1, "GetStatus() - No active failover output device.\n");
    }

    // FormatEngine - get status for the input device and format
    return _formatEngine->GetStatus(devRef, formatNo, query);
*/
}


unsigned int CChannelFactory::GetNextFailoverOutputChannel() {
    _activeFailoverOutputChannel++;

    // wrap if we're past the last channel
    if (_activeFailoverOutputChannel >= _nOutputChannels) {
        for (unsigned int i = 0; i < _nOutputChannels; i++) {
            if (IsFailoverOutputChannel(i)) {
                _activeFailoverOutputChannel = i;
                break;
            }
        }
    }

    return _activeFailoverOutputChannel;
}


bool CChannelFactory::IsFailoverOutputChannel(const unsigned int ch) {
    ASSERT(ch < _nOutputChannels);

    if (_outputChannel[ch])
        return _outputChannel[ch]->IsFailoverChannel();

    return false;
}


bool CChannelFactory::ResetInputChannel() {
    ASSERT(_formatEngine);

    if (_inputChannel == NULL) {
        LOGERROR(1, "ResetInputChannel() - Input device not installed.\n");
        return false;
    }

    CBaseFormatDescriptor *formatDescriptor = _inputChannel->GetFormatDescriptor();
    if (formatDescriptor == NULL) {
        LOGERROR(1, "ResetInputChannel() - Cannot get the format descriptor.\n");
        return false;
    }

    bool rc = _inputChannel->Reset();
    rc = _formatEngine->OnResetInputChannel(*formatDescriptor) && rc;

    return rc;
}


bool CChannelFactory::ResetOutputChannel(const unsigned int outputChannel) {
    // Get the reference to the output device

    if (_outputChannel[outputChannel] == NULL) {
        LOGERROR(1, "ResetOutputChannel() - Output device not installed.\n");
        return false;
    }

    CBaseFormatDescriptor *formatDescriptor = _outputChannel[outputChannel]->GetFormatDescriptor();
    if (formatDescriptor == NULL) {
        LOGERROR(1, "ResetOutputChannel() - Cannot get the format descriptor.\n");
        return false;
    }

    bool rc = _outputChannel[outputChannel]->Reset();
    rc = _formatEngine->OnResetOutputChannel(outputChannel, *formatDescriptor) && rc;

    return rc;
}


bool CChannelFactory::IoCtrl(const int channel, const unsigned int command, const void *data, size_t len) {
    CBaseDevice *pDev = NULL;

    if (channel == -1) {
        // input channel
        if (_inputChannel == NULL) {
            LOGERROR(1, "IoCtrl() - Input device not installed.\n");
            return false;
        }

        pDev = CDeviceFactory::Instance()->GetDevice(_inputChannel->GetDeviceNo());
    } else {
        // output channel
        if (_outputChannel[channel] == NULL) {
            LOGERROR(1, "IoCtrl() - Output device not installed.\n");
            return false;
        }

        pDev = CDeviceFactory::Instance()->GetDevice(_outputChannel[channel]->GetDeviceNo());
    }

    if (pDev == NULL) {
        LOGERROR(1, "IoCtrl() - Cannot get the device.\n");
        return false;
    }

    return pDev->IoCtrl(command, data, len);
}
