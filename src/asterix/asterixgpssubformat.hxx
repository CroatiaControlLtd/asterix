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
#ifndef ASTERIXGPSSUBFORMAT_HXX__
#define ASTERIXGPSSUBFORMAT_HXX__

/*
struct sFinalRecordHeaderGPS
{
	unsigned char m_nByteCountMSB;
	unsigned char m_nByteCountLSB;
	unsigned char m_nBoard;
	unsigned char m_nLine;
	unsigned char m_nRecordingDay;
	unsigned char m_nTimeMMSB;
	unsigned char m_nTimeMSB;
	unsigned char m_nTimeLSB;
	unsigned char m_nTimeLSB1;
	unsigned char m_nTimeLSB2;
};
*/

class CBaseDevice;

/**
 * @class CAsterixInSubformat
 *
 * @brief Asterix native sub-format
 *
 * Specifies format of Asterix message.
 *
 */
class CAsterixGPSSubformat {
public:

    static bool
    ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard, bool oradis = false);

    static bool
    WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard, bool oradis = false);

    static bool
    ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard, bool oradis = false);

    static bool Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool oradis = false);

private:

};

#endif
