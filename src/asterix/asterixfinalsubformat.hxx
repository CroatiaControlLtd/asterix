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

#ifndef ASTERIXFINALSUBFORMAT_HXX__
#define ASTERIXFINALSUBFORMAT_HXX__
/**
 * @file asterixfinalsubformat.hxx
 *
 * @brief Asterix final file sub-format processor. This is Asterix format encapsulated in final file.
 *
 * Copyright (c) 2009 Croatia Control Ltd. All rights reserved.
 *
 * MODIFICATION/HISTORY:
 *
 * rev.1.0 2009/10/13 Damir Salantic, [damir.salantic@crocontrol.hr]
 * Created.
 *
 * @author Damir Salantic, Croatia Control Ltd.
 */

/*
 * Final file format:
 *
 * [ One or more final records ]
 *
 * Final record format:
 *
 * 1 byte [Byte count 1 MSB]
 * 1 byte [Byte count 2 LSB]
 * 1 byte [Board number]
 * 1 byte [Line number]
 * 1 byte [Recording day]
 * 1 byte [Time MMSB 0.01s]
 * 1 byte [Time MSB 0.01s]
 * 1 byte [Time LSB 0.01s]
 * n bytes [One or more Asterix blocks]
 * 1 byte [Pad 0xA5]
 * 1 byte [Pad 0xA5]
 * 1 byte [Pad 0xA5]
 * 1 byte [Pad 0xA5]
 *
 */


class CBaseDevice;

class CAsterixFormatDescriptor;

class InputParser;


/**
 * @class CAsterixFinalSubformat
 *
 * @brief Asterix native sub-format
 *
 * Specifies format of Asterix message.
 *
 */
class CAsterixFinalSubformat {
public:

    static bool ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard);

    static bool WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard);

    static bool ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard);

    static bool Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device);

private:

    struct sFinalRecordHeader {
        unsigned char m_nByteCountMSB;
        unsigned char m_nByteCountLSB;
        unsigned char m_nBoard;
        unsigned char m_nLine;
        unsigned char m_nRecordingDay;
        unsigned char m_nTimeMMSB;
        unsigned char m_nTimeMSB;
        unsigned char m_nTimeLSB;
    };

};

#endif
