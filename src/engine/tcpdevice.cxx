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
 * 			Zeljko Harjac, Croatia Control Ltd.
 *
 */

// Networking
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/select.h> // fd_set
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
#include <fcntl.h>

// Local includes
#include "asterix.h"
#include "tcpdevice.hxx"


CTcpDevice::CTcpDevice(CDescriptor &descriptor) {
    const char *server = descriptor.GetFirst();
    const char *serverAddress = descriptor.GetNext();
    const char *serverPort = descriptor.GetNext();
    const char *clientAddress = descriptor.GetNext();
    const char *clientPort = descriptor.GetNext();
    int clientPortNo = 0;
    int serverPortNo = 0;
    bool isServer = false;

    // Server argument
    if ((server == NULL) || (strlen(server) == 0)) {
        LOGERROR(1, "Server flag not specified\n");
    } else {
        if (toupper(server[0]) == 'S') {
            LOGINFO(gVerbose, "Server ");
            isServer = true;
            if (toupper(server[1]) == 'P') {
                LOGINFO(gVerbose, "in pre-emptive mode: accept any pending connection immediately.\n");
                _acceptPending = true;
            } else {
                LOGINFO(gVerbose,
                        "in single connection mode: all pending connections wait for current session to close\n");
                _acceptPending = false;
            }

        } else {
            LOGINFO(gVerbose, "Client mode\n");
        }
    }

    // Server address argument may be null only in server mode
    // In that case INADDR_ANY is used.
    if ((serverAddress == NULL) || (strlen(serverAddress) == 0)) {
        if (isServer) {
            LOGINFO(gVerbose, "INADDR_ANY used as server interface\n");
        } else {
            LOGERROR(1, "Server address not specified\n");
        }

    } else {
        LOGINFO(gVerbose, "Server address: %s\n", serverAddress);
    }

    // Server port argument may be 0 only in client mode
    if ((serverPort == NULL) || (strlen(serverPort) == 0)) {
        if (isServer) LOGERROR(1, "Server port not specified\n")
        else LOGWARNING(gVerbose, "Server port not specified (%d by default)\n", serverPortNo);
    } else {
        serverPortNo = atoi(serverPort);
        LOGINFO(gVerbose, "Server port: %d\n", serverPortNo);
    }

    // Client address argument may be null
    // In that case INADDR_ANY is used.
    if ((clientAddress == NULL) || (strlen(clientAddress) == 0)) {
        LOGINFO(gVerbose, "INADDR_ANY used as client address\n");
    } else {
        LOGINFO(gVerbose, "Client address: %s\n", clientAddress);
    }

    // Client port argument
    if ((clientPort == NULL) || (strlen(clientPort) == 0)) {
        LOGINFO(gVerbose, "Client port not specified (%d by default)\n", clientPortNo);
    } else {
        clientPortNo = atoi(clientPort);
        LOGINFO(gVerbose, "Client port: %d\n", clientPortNo);
    }


    // Call default initialization
    Init(serverAddress, serverPortNo, clientAddress, clientPortNo, isServer);
}


CTcpDevice::~CTcpDevice() {
    if (_socketDescSession >= 0) {
        close(_socketDescSession);
    }

    if (_socketDesc >= 0) {
        close(_socketDesc);
    }
}


bool CTcpDevice::Read(void *data, size_t len) {
    // Check if interface was set-up correctly (server)
    if (!_opened) {
        LOGERROR(1, "Cannot read due to not properly initialized interface.\n");
        CountReadError();
        return false;
    }

    // Connect or accept connection if necessary
    if (!Connect()) {
        LOGERROR(1, "Cannot read, could not establish connection.\n");
        CountReadError();
        return false;
    }

    // Chose appropriate socket for reading
    int socketToRecv = _server ? _socketDescSession : _socketDesc;
    ASSERT(socketToRecv >= 0);

    // Get the message (blocking)

    int bytesReceived = 0;

#ifdef __CYGWIN__
    int totalReceived = 0;
    while (((unsigned int) totalReceived) < len) {
        bytesReceived = recv(socketToRecv, ((char *) data) + totalReceived, len - totalReceived, MSG_NOSIGNAL);
#else
        bytesReceived = recv(socketToRecv, data, len, MSG_NOSIGNAL | MSG_WAITALL);
#endif

        if (bytesReceived < 0) {
            LOGERROR(1, "Error %d reading from socket %d. %s\n",
                     errno,
                     socketToRecv,
                     strerror(errno));

            Disconnect(false); // error, do not linger
            CountReadError();
            return false;
        } else if (bytesReceived == 0) {
            LOGNOTIFY(gVerbose, "Connection closed by remote host\n");
            Disconnect(false); // error, do not linger
//        CountReadError(); // this can be a normal disconnection, not an error
            return false;
        }

#ifdef __CYGWIN__
        totalReceived += bytesReceived;
    } // while
#else
    else if ((unsigned int) bytesReceived != len)
    {
        LOGWARNING(1, "Read only %d bytes from %d requested from socket %d.\n", 
           bytesReceived,
           (int)len,
           socketToRecv);
    }
#endif


    LOGDEBUG(ZONE_TCPDEVICE, "Read message from socket %d with length %d.\n",
             socketToRecv,
             bytesReceived);

    ResetReadErrors(true);
    return true;
}


bool CTcpDevice::Write(const void *data, size_t len) {
    // Check if interface was set-up correctly (client)
    if (!_opened) {
        LOGERROR(1, "Cannot write due to not properly initialized interface.\n");
        CountWriteError();
        return false;
    }

    // Connect or accept connection if necessary
    if (!Connect()) {
        CountWriteError();
        return false;
    }

    // Chose appropriate socket for writing
    int socketToSend = _server ? _socketDescSession : _socketDesc;
    ASSERT(socketToSend >= 0);

    // Write the message (blocking)
    if (send(socketToSend, data, len, MSG_NOSIGNAL) < 0) {
        LOGERROR(1, "Error %d writing to socket %d. %s\n",
                 errno, socketToSend, strerror(errno));

        Disconnect(false); // error, do not linger
        CountWriteError();
        return false;

    }

    LOGDEBUG(ZONE_TCPDEVICE, "Wrote message to socket %d.\n", socketToSend);

    ResetWriteErrors(true);
    return true;
}


bool CTcpDevice::Select(const unsigned int secondsToWait) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot select() due to not properly initialized interface.\n");
        return false;
    }

    // Connect or accept connection if necessary
    if (!Connect()) {
        return false;
    }

    // Chose appropriate socket for select function
    int socketToSelect = _server ? _socketDescSession : _socketDesc;
    ASSERT(socketToSelect >= 0);

    // Configure 'set' of single class file descriptor
    fd_set descToRead;
    int selectVal;
    FD_ZERO(&descToRead);
    FD_SET(socketToSelect, &descToRead);
    if (_server && _acceptPending) {
        // Add also the listening socket to the select list to be able to accept pending connections
        FD_SET(_socketDesc, &descToRead);
    }

    if (secondsToWait) {
        // Set the timeout as specified
        struct timeval timeout;
        timeout.tv_sec = secondsToWait;
        timeout.tv_usec = 0;

        selectVal = select(socketToSelect + 1, &descToRead, NULL, NULL, &timeout);
    } else {
        // secondsToWait is zero => Wait indefinitely
        selectVal = select(socketToSelect + 1, &descToRead, NULL, NULL, NULL);
    }

    if (selectVal < 0) {
        LOGERROR(1, "Error %d during select(). %s\n", errno, strerror(errno));
        return false;
    }

    // Check if there are connections to accept
    if (_server && _acceptPending) {
        if (FD_ISSET(_socketDesc, &descToRead)) {
            AcceptPending();
        }
    }

    // Now, check if there are data to read
    return (FD_ISSET(socketToSelect, &descToRead));
}


bool CTcpDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
    static bool result;

    switch (command) {
        case EReset:
            Disconnect(true); // normal disconnection, do linger
            result = !_connected;
            break;
        case EAllDone:
            sleep(1); // according to ICD, this delay is not needed, but is is suggested in T04-TPB-CSW
            result = Disconnect(true); // normal disconnection, do linger
            break;
        case EPacketDone:
            result = false;
            break;
        case EIsLastPacket:
            result = !_connected;
            break;
        default:
            result = false;
    }

    return result;
}


bool CTcpDevice::Connect() {

    if (!_connected) {
        // Device not connected - try to connect!
        if (_server) {
            // Server will wait (BLOCKING) for a client to connect if there is no pending connection
            _socketDescSession = accept(_socketDesc, (struct sockaddr *)
                    &_clientAddr, (socklen_t *) &_clientAddrLen);
            if (_socketDescSession < 0) {
                LOGERROR(1, "Error %d accepting connection. %s\n", errno, strerror(errno));
                return false;
            }
            LOGNOTIFY(gVerbose, "Accepted connection from %s on socket %d\n",
                      inet_ntoa(((struct sockaddr_in *) &_clientAddr)->sin_addr), _socketDescSession);
            _connected = true;
        } else {
            // Client will open and bind socket again
            _socketDesc = socket(AF_INET, SOCK_STREAM, 0);
            if (_socketDesc < 0) {
                LOGERROR(1, "Cannot re-open socket\n");
                return false;
            }

            // Enable local address reuse
            int opt = 1;
            if (setsockopt(_socketDesc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
                LOGWARNING(1, "Error %d on setsockopt(). %s\n", errno, strerror(errno));
                close(_socketDesc);
            }

            if (bind(_socketDesc, (struct sockaddr *) &_clientAddr, sizeof(_clientAddr)) < 0) {
                LOGERROR(1, "Cannot re-bind %s\n", inet_ntoa(_clientAddr.sin_addr));
                close(_socketDesc);
                return false;
            }



            // Connect to the server
            if (connect(_socketDesc, (struct sockaddr *) &_serverAddr, sizeof(_serverAddr)) < 0) {
                LOGERROR(1, "Cannot connect to server %s (error %d). %s\n", inet_ntoa(_serverAddr.sin_addr), errno,
                         strerror(errno));
                close(_socketDesc);
                return false;
            }
            _connected = true;
        }
    } else {
        if (_server) {
            // if (accept_pending_option)
            // Device is already connected, but check if there are pending connections in the listen queue to handle.
//            AcceptPending();
        }
    }

    return true;
}


bool CTcpDevice::AcceptPending() {
    int fd_flags = fcntl(_socketDesc, F_GETFL, 0);
    int fd_flags_block = fd_flags & ~FNDELAY;  // Blocking on
    int fd_flags_noblock = fd_flags | FNDELAY; // Blocking off

    // Accept (NON-BLOCKING) if there is a pending connection        
    fcntl(_socketDesc, F_SETFL, fd_flags_noblock);
    int socketDescNewSession = accept(_socketDesc, (struct sockaddr *)
            &_clientAddr, (socklen_t *) &_clientAddrLen);
    fcntl(_socketDesc, F_SETFL, fd_flags_block);         // Get back to blocking mode

    if (socketDescNewSession < 0) {
        if (errno != EAGAIN) {
            LOGERROR(1, "Error %d accepting pending connection. %s\n", errno, strerror(errno));
        }
        return false;
    }
    LOGINFO(gVerbose, "Accepted pending connection from %s on socket %d\n",
            inet_ntoa(((struct sockaddr_in *) &_clientAddr)->sin_addr), socketDescNewSession);

    // if there is already an accepted connection - force its disconnection
    if (_connected && (_socketDescSession >= 0)) {
        close(_socketDescSession);
        LOGINFO(gVerbose, "Closed connection from %s on socket %d\n",
                inet_ntoa(((struct sockaddr_in *) &_clientAddr)->sin_addr), _socketDescSession);
    }
    _socketDescSession = socketDescNewSession; // Initialize session socket descriptor

    _connected = true;
    return true;
}


bool CTcpDevice::Disconnect(bool bLinger) {
    bool result = false;

    if (_connected) {
        // set 60 second linger time, if requested by bLinger argument
        struct linger sl = {
                (short unsigned int) (bLinger ? 1 : 0),
                (short unsigned int) (bLinger ? 60 * 100 : 0)
        };

        if (bLinger) {
            LOGDEBUG(1, "Disconnecting in linger mode\n");
        } else {
            LOGDEBUG(1, "Disconnecting without lingering\n");
        }

        if (_server) {
            ASSERT(_socketDescSession >= 0);

            if (setsockopt(_socketDescSession, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) != 0) {
                LOGWARNING(1, "Error %d on setsockopt(). %s\n", errno, strerror(errno));
            }

            if (close(_socketDescSession) < 0) {
                LOGWARNING(1, "Error %d closing connection. Forcing disconnection. %s\n", errno, strerror(errno));
                result = false;
            } else {
                LOGNOTIFY(gVerbose, "Disconnected from client %s\n",
                          inet_ntoa(((struct sockaddr_in *) &_clientAddr)->sin_addr));
                result = true;
            }
        } else {
            ASSERT(_socketDesc >= 0);

            if (setsockopt(_socketDesc, SOL_SOCKET, SO_LINGER, &sl, sizeof(sl)) != 0) {
                LOGWARNING(1, "Error %d on setsockopt(). %s\n", errno, strerror(errno));
            }

            if (close(_socketDesc) < 0) {
                LOGERROR(1, "Error %d closing connection.  Forcing disconnection. %s\n", errno, strerror(errno));
                result = false;
            } else {
                LOGINFO(gVerbose, "Disconnected from server %s\n", inet_ntoa(_serverAddr.sin_addr));
                result = true;
            }
        }

        _connected = false;
    }
    return result;
}


bool CTcpDevice::InitServer(const char *serverAddress, const int serverPort) {

    if ((serverAddress == NULL) || (strlen(serverAddress) == 0)) {
        // Bind any server address
        _serverAddr.sin_family = AF_INET;
        _serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        _serverAddr.sin_port = htons(serverPort);
    } else {
        // Bind specified server address
        struct hostent *host;

        host = gethostbyname(serverAddress);
        if (host == NULL) {
            LOGERROR(1, "Unknown server address '%s'\n", serverAddress);
            return false;
        }
        _serverAddr.sin_family = host->h_addrtype;
        memcpy(&_serverAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
        _serverAddr.sin_port = htons(serverPort);
    }

    if (bind(_socketDesc, (struct sockaddr *) &_serverAddr, sizeof(_serverAddr)) < 0) {
        LOGERROR(1, "Cannot bind port number %d\n", serverPort);
        return false;
    }

    // Receive SYN packets on unconnected socket
    // backlog parameter 0 for server in single connection mode should refuse all connections
    // backlog parameter for server in pre-emptive mode will always allow establishing
    // connections in background.
    int backlog = _acceptPending ? MAX_BACKLOG : 0;
    listen(_socketDesc, backlog);


    // Server will accept connections on a call to appropriate function when necessary

    return true;
}


bool CTcpDevice::InitClient(const char *serverAddress, const int serverPort, const char *clientAddress,
                            const int clientPort) {
    struct hostent *host;

    // Server address/port
    ASSERT(serverAddress);
    host = gethostbyname(serverAddress);
    if (host == NULL) {
        LOGERROR(1, "Unknown server address '%s'\n", serverAddress);
        return false;
    }

    _serverAddr.sin_family = host->h_addrtype;
    memcpy(&_serverAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
    _serverAddr.sin_port = htons(serverPort);

    // Enable local address reuse
    int opt = 1;
    if (setsockopt(_socketDesc, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)) != 0) {
        LOGWARNING(1, "Error %d on setsockopt(). %s\n", errno, strerror(errno));
    }

    if ((clientAddress == NULL) || (strlen(clientAddress) == 0)) {
        // Bind any client address
        _clientAddr.sin_family = AF_INET;
        _clientAddr.sin_addr.s_addr = htonl(INADDR_ANY);
        _clientAddr.sin_port = htons(clientPort);
    } else {
        // Bind specified client address
        host = gethostbyname(clientAddress);
        if (host == NULL) {
            LOGERROR(1, "Unknown client address '%s'\n", clientAddress);
            return false;
        }
        _clientAddr.sin_family = host->h_addrtype;
        memcpy(&_clientAddr.sin_addr.s_addr, host->h_addr_list[0], host->h_length);
        _clientAddr.sin_port = htons(clientPort);
    }

    if (bind(_socketDesc, (struct sockaddr *) &_clientAddr, sizeof(_clientAddr)) < 0) {
        LOGERROR(1, "Cannot bind %s\n", inet_ntoa(_clientAddr.sin_addr));
        return false;
    }

    // Client will establish connection on on a call to appropriate function when necessary


    return true;
}


void CTcpDevice::Init(const char *serverAddress, const int serverPort, const char *clientAddress, const int clientPort,
                      const bool server) {
    _opened = false;
    _connected = false;
    _socketDesc = -1;
    _socketDescSession = -1;
    _server = server;


    // 1. Global initialization

    // Get Socket
    _socketDesc = socket(AF_INET, SOCK_STREAM, 0);
    if (_socketDesc < 0) {
        LOGERROR(1, "Cannot open socket\n");
        return;
    }

    // 2. Client/server dependent initialization
    if (_server) {
        if (!InitServer(serverAddress, serverPort)) {
            LOGERROR(1, "Server not initialized\n");
            return;
        }
    } else {
        if (!InitClient(serverAddress, serverPort, clientAddress, clientPort)) {
            LOGERROR(1, "Client not initialized\n");
            return;
        }
    }

    _opened = true;

}
