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
#ifndef CHANNELFACTORY_HXX__
#define CHANNELFACTORY_HXX__

#include "singleton.hxx"
#include "channel.hxx"


class CBaseFormat;

class CBaseFormatDescriptor;

/**
 * @class CChannelFactory
 *
 * @brief Factory for channel creation and access.
 *
 * @see   <CSingleton>
 */
class CChannelFactory {
public:

    static const unsigned int MAX_OUTPUT_CHANNELS = 10;

private:

    // Singleton pattern
    static CSingleton<CChannelFactory> _Instance;

    // To access private Ctor...
    friend class CSingleton<CChannelFactory>;

    CChannel *_inputChannel;
    CChannel *_outputChannel[MAX_OUTPUT_CHANNELS];
    unsigned int _nOutputChannels;
    CBaseFormat *_formatEngine;
    unsigned int _activeFailoverOutputChannel;

public:

    /**
     * Returns the pointer to the only instance of the factory
     * (Singleton pattern). If the instance does not yet
     * exist (on a first call), it is created
     *
     * @return Pointer to the only instance of the factory.
     *
     * @see <CSingleton>::<Instance>
     */
    static CChannelFactory *Instance() { return _Instance.Instance(); }

    /**
     * Deletes the one and only instance of the factory.
     * (Singleton pattern).
     *
     * @see <CSingleton>::<DeleteInstance>
     */
    static void DeleteInstance() { _Instance.DeleteInstance(); }

    /**
     * Default engine constructor. Allocates any necessary resources.
     *
     */
    CChannelFactory();

    /**
     * Default engine destructor. Frees all allocated resources.
     *
     */
    ~CChannelFactory();

    bool CreateInputChannel(const char *sDeviceName, const char *sDeviceDescriptor,
                            const char *sFormatName, const char *sFormatDescriptor);

    bool CreateOutputChannel(const char *sDeviceName, const char *sDeviceDescriptor,
                             const char *sFormatName, const char *sFormatDescriptor,
                             const bool bFailover, const char *sHeartbeat);

    CChannel *GetInputChannel() { return _inputChannel; };


    unsigned int GetNOutputChannels() { return _nOutputChannels; }

    unsigned int GetActiveFailoverOutputChannel() { return _activeFailoverOutputChannel; };

    unsigned int GetNextFailoverOutputChannel();

    bool IsFailoverOutputChannel(const unsigned int ch);

    bool WaitForPacket(const unsigned int secondsToWait);

    bool ReadPacket();

    bool WritePacket(const unsigned int outputChannel);

    bool ProcessPacket(bool &discard);

    bool HeartbeatProcessing(const unsigned int outputChannel);

    int GetStatus(int query = 0);

    bool ResetInputChannel();

    bool ResetOutputChannel(const unsigned int outputChannel);

    bool IoCtrl(const int channel, const unsigned int command, const void *data = 0, size_t len = 0);

private:

    bool AttachFormatter(const char *sFormatName, const char *sFormatDescriptor,
                         unsigned int &formatNo, CBaseFormatDescriptor **formatDesc);
};

#endif
