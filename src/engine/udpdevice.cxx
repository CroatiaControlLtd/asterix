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
 *			Damir Salantic, Croatia Control Ltd.
 */

// Networking
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

// Standard includes
#include <sys/time.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <ctype.h>
#include <errno.h>

// Local includes
#include "asterix.h"
#include "udpdevice.hxx"



CUdpDevice::CUdpDevice(CDescriptor &descriptor)
{
    const char *mcastAddress     = descriptor.GetFirst();
    const char *interfaceAddress = descriptor.GetNext();
    const char *port             = descriptor.GetNext();
    const char *sourceAddress 	 = descriptor.GetNext();
    const char *server           = descriptor.GetNext();
    int portNo = 0;
    bool isServer = false;

    ASSERT(mcastAddress && interfaceAddress && port && server);

    // Mcast address argument
    if (strlen(mcastAddress) == 0)
    {
        LOGINFO(gVerbose, "Mcast address not specified\n");
    }

    // Interface address argument may be null.
    // In that case INADDR_ANY will be used.
    if (strlen(interfaceAddress) == 0)
    {
        LOGINFO(gVerbose, "INADDR_ANY used as interface\n");
    }

    // Mcast port argument
    if (port == NULL)
    {
        LOGWARNING(gVerbose, "Mcast port not specified (%d by default)\n", portNo);
    }
    else
    {
        portNo = atoi(port);
    }

    // Server argument
    if (strlen(server) == 0)
    {
        LOGWARNING(1, "Server flag not specified (%d by default)\n", isServer);
    }
    else
    {
        if (toupper(*server) == 'S')
        {
            isServer = true;
        }
    }


    // Call default initialization
    Init(mcastAddress, interfaceAddress, sourceAddress, portNo, isServer);
}



CUdpDevice::~CUdpDevice()
{
    if (_socketDesc >= 0)
    {
        close(_socketDesc);
    }
}



bool CUdpDevice::Read(void *data, size_t len)
{
	return Read(data, &len);
}

bool CUdpDevice::Read(void *data, size_t* len)
{
    // Check if interface was set-up correctly (server)
    if ((!_opened) || (!_server))
    {
        LOGERROR(1, "Cannot write due to not properly initialized interface.\n");
        CountReadError();
        return false;
    }

    // Get the message (blocking)
    struct sockaddr_in clientAddr;
    socklen_t clientLen = sizeof(clientAddr);
    ssize_t lenread = recvfrom(_socketDesc, data, *len, MSG_NOSIGNAL, (struct sockaddr *) &clientAddr, &clientLen);
    if (lenread < 0)
    {
        LOGERROR(1, "Error reading from %s on address %s.\n",
           inet_ntoa(clientAddr.sin_addr),
           inet_ntoa(_mcastAddr.sin_addr));

        CountReadError();
        return false;
    }

    *len = lenread;

    LOGDEBUG(ZONE_UDPDEVICE, "Read message from %s on address %s.\n",
           inet_ntoa(clientAddr.sin_addr),
           inet_ntoa(_mcastAddr.sin_addr));

    ResetReadErrors(true);
    return true;
}



bool CUdpDevice::Write(const void *data, size_t len)
{
    // Check if interface was set-up correctly (client)
    if ((!_opened) || (_server))
    {
        LOGERROR(1, "Cannot write due to not properly initialized interface.\n");
        CountWriteError();
        return false;
    }

    // Set the server address

    // Write the message (blocking)
    if (sendto(_socketDesc, data, len, MSG_NOSIGNAL, (struct sockaddr*) &_mcastAddr, sizeof(_mcastAddr)) < 0)
    {
        LOGERROR(1, "Error %d writing to %s.\n",
            errno, inet_ntoa(_mcastAddr.sin_addr));

        CountWriteError();
        return false;

    }

    LOGDEBUG(ZONE_UDPDEVICE, "Wrote message to %s.\n", inet_ntoa(_mcastAddr.sin_addr));

    ResetWriteErrors(true);
    return true;
}



bool CUdpDevice::Select(const unsigned int secondsToWait)
{
    // Check if interface was set-up correctly (server)
    if ((!_opened) || (!_server))
    {
        LOGERROR(1, "Cannot write due to not properly initialized interface.\n");
        return false;
    }

    // Configure 'set' of single class file descriptor
    fd_set descToRead;
    int selectVal;
    FD_ZERO(&descToRead);
    FD_SET(_socketDesc, &descToRead);

    if (secondsToWait)
    {
        // Set the timeout as specified
        struct timeval timeout;
        timeout.tv_sec  = secondsToWait;
        timeout.tv_usec = 0;

        selectVal = select(_socketDesc + 1, &descToRead,  NULL, NULL, &timeout);
    }
    else
    {
        // secondsToWait is zero => Wait indefinitely
        selectVal = select(_socketDesc + 1, &descToRead,  NULL, NULL, NULL);
    }

    return (selectVal == 1);
}



bool CUdpDevice::InitServer()
{
    struct sockaddr_in serverAddr;

    // Bind server port
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;

    if (IN_MULTICAST(ntohl(_mcastAddr.sin_addr.s_addr)))
    {
       serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    }
    else
    {
       serverAddr.sin_addr.s_addr = _interfaceAddr.s_addr;
    }
    serverAddr.sin_port = htons(_port);

    if (bind(_socketDesc, (struct sockaddr*) &serverAddr, sizeof(serverAddr)) < 0)
    {
        LOGERROR(1, "Cannot bind port number %d\n", _port);
        return false;
    }

    if (IN_MULTICAST(ntohl(_mcastAddr.sin_addr.s_addr)))
    {
    	if (_sourceAddr.s_addr != htonl(INADDR_ANY))
    	{
 	       // Join multicast group but only from specified address
 	       struct ip_mreq_source mreq; // Request block for multicast address.
 	       mreq.imr_multiaddr.s_addr = _mcastAddr.sin_addr.s_addr;
 	       mreq.imr_interface.s_addr = _interfaceAddr.s_addr;
 	       mreq.imr_sourceaddr.s_addr = _sourceAddr.s_addr;// source address

 	       if (setsockopt(_socketDesc, IPPROTO_IP, IP_ADD_SOURCE_MEMBERSHIP, (void*) &mreq, sizeof(mreq)) < 0)
 	       {
 	          LOGERROR(1, "Cannot join multicast address %s src: %s\n", inet_ntoa(_mcastAddr.sin_addr), inet_ntoa(_sourceAddr));
 	          return false;
 	       }
 		   else
 		   {
 		      LOGDEBUG(1, "Successfully joined multicast %s src: %s\n", inet_ntoa(_mcastAddr.sin_addr), inet_ntoa(_sourceAddr));
 		   }
    	}
    	else
    	{
			// Join multicast group
			struct ip_mreq mreq; // Request block for multicast address.
			mreq.imr_multiaddr.s_addr = _mcastAddr.sin_addr.s_addr;
			mreq.imr_interface.s_addr = _interfaceAddr.s_addr;

			if (setsockopt(_socketDesc, IPPROTO_IP, IP_ADD_MEMBERSHIP, (void*) &mreq, sizeof(mreq)) < 0)
			{
			  LOGERROR(1, "Cannot join multicast address %s\n", inet_ntoa(_mcastAddr.sin_addr));
			  return false;
			}
			else
			{
			  LOGDEBUG(1, "Successfully joined multicast %s\n", inet_ntoa(_mcastAddr.sin_addr));
			}
    	}
    }
    return true;
}



bool CUdpDevice::InitClient()
{
    struct sockaddr_in clientAddr;

    // Bind any port
    clientAddr.sin_family = AF_INET;
    clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    clientAddr.sin_port = htons(0);

    if (bind(_socketDesc, (struct sockaddr*) &clientAddr, sizeof(clientAddr)) < 0)
    {
        LOGERROR(1, "Cannot bind \n");
        return false;
    }

    if (IN_MULTICAST(ntohl(_mcastAddr.sin_addr.s_addr)))
    {
        unsigned char ttl = 1;  // send multicast on subnet only (ttl = 1)
        // Set ttl on the socket
        if (setsockopt(_socketDesc, IPPROTO_IP, IP_MULTICAST_TTL, &ttl, sizeof(ttl)) < 0)
        {
           LOGERROR(1, "Cannot set ttl = %d\n", ttl);
           return false;
        }
    }
    return true;
}



void CUdpDevice::Init(const char *mcastAddress, const char *interfaceAddress, const char *srcAddress, const int port, const bool server)
{
    struct hostent *host;
    u_int yes=1;

    _opened = false;
    _socketDesc = -1;
    _server = server;
    _port   = port;

    // 1. Global initialization

    // 1.1 Multicast address
    ASSERT(mcastAddress);
    host = gethostbyname(mcastAddress);
    if (host == NULL)
    {
        LOGERROR(1, "Unknown multicast group '%s'\n", mcastAddress);
        return;
    }

    _mcastAddr.sin_family = host->h_addrtype;
    memcpy(&_mcastAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
/*
    if (!IN_MULTICAST(ntohl(_mcastAddr.sin_addr.s_addr)))
    {
        LOGERROR(1, "Destination address %s is not multicast\n", mcastAddress);
        return;
    }
*/
    _mcastAddr.sin_port = htons(_port);


    // 1.1b Source interface
    if (srcAddress != NULL && strlen(srcAddress) != 0)
	{
		// Specific interface is chosen
		host = gethostbyname(srcAddress);
		if (host == NULL)
		{
			LOGERROR(1, "Unknown source address '%s'\n", srcAddress);
			return;
		}
		memcpy(&_sourceAddr, host->h_addr_list[0], host->h_length);
	}
	else
	{
		// Source address not chosen
		_sourceAddr.s_addr = htonl(INADDR_ANY);
	}

    // 1.2 Multicast interface
    if (strlen(interfaceAddress) != 0)
    {
        // Specific interface is chosen
        host = gethostbyname(interfaceAddress);
        if (host == NULL)
        {
            LOGERROR(1, "Unknown interface address '%s'\n", interfaceAddress);
            return;
        }
        memcpy(&_interfaceAddr, host->h_addr_list[0], host->h_length);
    }
    else
    {
        // Interface not chosen
        _interfaceAddr.s_addr = htonl(INADDR_ANY);
    }

    // 1.3 Socket
    _socketDesc = socket(AF_INET, SOCK_DGRAM, 0);
    if (_socketDesc < 0)
    {
        LOGERROR(1, "Cannot open socket\n");
        return;
    }

    // Allow multiple sockets to use the same PORT number
    if (setsockopt(_socketDesc, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes)) < 0)
    {
        LOGERROR(1, "Cannot reuse socket address\n");
        return;
    }

    // 2. Client/server dependant initialization
    if (_server)
    {
        if (!InitServer())
        {
            LOGERROR(1, "Server not initialized\n");
            return;
        }
    }
    else
    {
        if (!InitClient())
        {
            LOGERROR(1, "Client not initialized\n");
            return;
        }
    }

    _opened = true;

}
