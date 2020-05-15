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
#ifndef TCPDEVICE_HXX__
#define TCPDEVICE_HXX__

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include "basedevice.hxx"
#include "descriptor.hxx"

/*
 * MSG_NOSIGNAL is not defined on OS X
 * but has an equivalent (SO_NOSIGPIPE)
 */
#ifdef __APPLE__
#define MSG_NOSIGNAL SO_NOSIGPIPE
#endif

#define MAX_TCP_PACKET_SIZE     1500

/**
 * @class CTcpDevice
 *
 * @brief The TCP/IP connection device.
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CTcpDevice : public CBaseDevice {
public:
    static const unsigned int MAX_BACKLOG = 5;


private:
    bool _server;
    struct sockaddr_in _clientAddr;
    int _clientAddrLen;
    struct sockaddr_in _serverAddr;
    int _socketDesc;
    int _socketDescSession;
    bool _connected;
    bool _acceptPending;

private:
    bool InitServer(const char *serverAddress, const int serverPort);

    bool InitClient(const char *serverAddress, const int serverPort, const char *clientAddress, const int clientPort);

    void Init(const char *serverAddress, const int serverPort, const char *clientAddress, const int clientPort,
              const bool server);

    bool Connect();

    bool AcceptPending();

    bool Disconnect(bool bLinger = false);

public:

    /**
     * Class constructor which uses descriptor
     */
    CTcpDevice(CDescriptor &descriptor);

    /**
     * Class destructor.
     */
    virtual ~CTcpDevice();

    virtual bool Read(void *data, size_t len);

    virtual bool Write(const void *data, size_t len);

    virtual bool Select(const unsigned int secondsToWait);

    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0);

    virtual bool IsPacketDevice() { return true; }

    virtual unsigned int
    MaxPacketSize() { return MAX_TCP_PACKET_SIZE; } // return maximal packet size (only for packet devices
    virtual unsigned int BytesLeftToRead() { return 0; } // return number of bytes left to read or 0 if unknown

};

#endif

