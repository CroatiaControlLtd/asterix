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
 * AUTHORS: Damir Salantic, Croatia Control Ltd.
 *
 */

#include <stdio.h>
#include <string.h>
#include <sys/time.h>

#include "asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixgpssubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"

/*
 * Read packet and store it in Descriptor.m_pBuffer
 */
bool CAsterixGPSSubformat::ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard,
                                      bool oradis) {
    CAsterixFormatDescriptor &Descriptor((CAsterixFormatDescriptor &) formatDescriptor);
    size_t readSize = 0;

    if (device.IsPacketDevice()) { // if using packet device read complete packet
        readSize = device.MaxPacketSize();

        const unsigned char *pBuffer = Descriptor.GetNewBuffer(readSize);

        // Read packet
        if (!device.Read((void *) pBuffer, &readSize)) {
            LOGERROR(1, "Couldn't read packet.\n");
            return false;
        }

        Descriptor.SetBufferLen(readSize);

        if (Descriptor.GetBufferLen() >= device.MaxPacketSize()) {
            LOGERROR(1, "Packet too big! Size = %d, limit = %d.\n", Descriptor.GetBufferLen(), device.MaxPacketSize());
        }
    } else { // if this is not packet device (e.g. it is file), read one by one Asterix record
        int leftBytes = device.BytesLeftToRead();

        if (oradis) {
            // Read ORADIS header (6 bytes)
            unsigned char oradisHeader[6];
            readSize = 6;
            if (!device.Read((void *) oradisHeader, &readSize)) {
                LOGERROR(1, "Couldn't read ORADIS header.\n");
                return false;
            }

            // Calculate ORADIS packet size
            unsigned short dataLen = oradisHeader[0]; // length
            dataLen <<= 8;
            dataLen |= oradisHeader[1];

            if (dataLen <= 6) {
                LOGERROR(1, "Wrong ORADIS data length (%d)\n", dataLen);
                return false;
            }
            if (leftBytes != 0 && dataLen > leftBytes) {
                LOGERROR(1, "Not enough data for packet! Size = %d, left = %d.\n", dataLen, leftBytes);
                return false;
            }

            readSize = dataLen - 6;
            const unsigned char *pBuffer = Descriptor.GetNewBuffer(dataLen);

            // copy header
            memcpy((void *) pBuffer, (void *) oradisHeader, 6);

            // Read rest of packet
            if (!device.Read((void *) &pBuffer[6], &readSize)) {
                LOGERROR(1, "Couldn't read packet.\n");
                return false;
            }
        } else {
            // Do only once, read GPS header (2200 bytes)
            if (device.IsOnStart()) {
                unsigned char gpsHeader[2200];
                readSize = 2200;
                if (!device.Read((void *) gpsHeader, &readSize)) {
                    LOGERROR(1, "Couldn't read GPS header.\n");
                    return false;
                }
            }

            // Read Asterix header (3 bytes)
            unsigned char asterixHeader[3];
            readSize = 3;
            if (!device.Read((void *) asterixHeader, &readSize)) {
                LOGERROR(1, "Couldn't read Asterix header.\n");
                return false;
            }

            // Calculate Asterix packet size
            unsigned short dataLen = asterixHeader[1]; // length
            dataLen <<= 8;
            dataLen |= asterixHeader[2];

            if (dataLen <= 3) {
                LOGERROR(1, "Wrong Asterix data length (%d)\n", dataLen);
                return false;
            }
            if (leftBytes != 0 && dataLen > leftBytes) {
                LOGERROR(1, "Not enough data for packet! Size = %d, left = %d.\n", dataLen, leftBytes);
                return false;
            }

            readSize = dataLen - 3;
            const unsigned char *pBuffer = Descriptor.GetNewBuffer(dataLen);

            // copy header
            memcpy((void *) pBuffer, (void *) asterixHeader, 3);

            // Read rest of packet
            if (!device.Read((void *) &pBuffer[3], &readSize)) {
                LOGERROR(1, "Couldn't read packet.\n");
                return false;
            }

            if (leftBytes != 0 && 10 > leftBytes) // size of GPS post bytes
            {
                LOGERROR(1, "Not enough data for GPS post bytes Size = 10, left = %d.\n", leftBytes);
                return false;
            }

            // Read GPS postbytes (10 bytes)
            unsigned char GPSPost[10];
            readSize = 10;
            if (!device.Read((void *) GPSPost, &readSize)) {
                LOGERROR(1, "Couldn't read GPS post bytes.\n");
                return false;
            }

            float fTimeStamp = ((GPSPost[6] << 16) + (GPSPost[7] << 8) + (GPSPost[8])) / 128.0;
            unsigned long nTimeStamp = ((long) fTimeStamp) * 1000 + (fTimeStamp - ((long) fTimeStamp)) * 1000;
#ifdef _DEBUG
            LOGDEBUG(1, "GPS Bytes [%02X %02X %02X %02X %02X %02X %02X %02X %02X %02X]\n", GPSPost[0], GPSPost[1],
                GPSPost[2], GPSPost[3], GPSPost[4], GPSPost[5], GPSPost[6], GPSPost[7],
                GPSPost[8], GPSPost[9]);
            LOGDEBUG(1, "GPS Timestamp [%3.4f]\n", fTimeStamp);
#endif
            Descriptor.SetTimeStamp(nTimeStamp);

        }
    }
    return true;
}

bool CAsterixGPSSubformat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard,
                                       bool oradis) {
    return false; //TODO
}

/*
 * Parse packet read from UDP and stored to Descriptor.m_pBuffer
 */
bool CAsterixGPSSubformat::ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard,
                                         bool oradis) {
    CAsterixFormatDescriptor &Descriptor((CAsterixFormatDescriptor &) formatDescriptor);

    // check data size
    if (Descriptor.GetBufferLen() < 3) {
        LOGERROR(1, "Packet too small.\n");
        return false;
    }

    // delete old data
    if (Descriptor.m_pAsterixData) {
        delete Descriptor.m_pAsterixData;
        Descriptor.m_pAsterixData = NULL;
    }

    // parse packet
    if (oradis) {

        // get current timstamp in ms since midnight
        struct timeval tp;
        gettimeofday(&tp, NULL);
        unsigned long nTimestamp = (tp.tv_sec % 86400) * 1000 + tp.tv_usec / 1000;

        unsigned char *pPacketPtr = (unsigned char *) Descriptor.GetBuffer();
        int m_nDataLength = Descriptor.GetBufferLen();

        while (m_nDataLength > 0) {
            // parse ORADIS header (6 bytes)
            // Byte count (1) (MSB)
            // Byte count (2) (LSB)
            // Time (1) MSB
            // Time (2)
            // Time (3)
            // Time (4) LSB
            // ASTERIX (byte counts)
            unsigned short byteCount = *pPacketPtr; // length
            pPacketPtr++;
            byteCount <<= 8;
            byteCount |= *pPacketPtr;
            pPacketPtr++;

            // skip time
            pPacketPtr += 4;

            if (byteCount > m_nDataLength)
                break;

            // Parse ASTERIX data
            AsterixData *m_ptmpAsterixData = Descriptor.m_InputParser.parsePacket(pPacketPtr, byteCount - 6,
                                                                                  nTimestamp);

            if (Descriptor.m_pAsterixData == NULL) {
                Descriptor.m_pAsterixData = m_ptmpAsterixData;
            } else {
                Descriptor.m_pAsterixData->m_lDataBlocks.splice(Descriptor.m_pAsterixData->m_lDataBlocks.end(),
                                                                m_ptmpAsterixData->m_lDataBlocks);
            }

            pPacketPtr += (byteCount - 6);
            m_nDataLength -= byteCount;
        }
    } else {
        unsigned long nTimeStamp = Descriptor.GetTimeStamp();
        Descriptor.m_pAsterixData = Descriptor.m_InputParser.parsePacket(Descriptor.GetBuffer(),
                                                                         Descriptor.GetBufferLen(), nTimeStamp);
    }

    return true;
}

bool CAsterixGPSSubformat::Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool oradis) {
    // nothing to do
    return true;
}



