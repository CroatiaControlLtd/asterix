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

#ifndef ASTERIXFORMAT_HXX__
#define ASTERIXFORMAT_HXX__

#include "baseformat.hxx"
#include "baseformatdescriptor.hxx"

class CBaseDevice;
//class CBaseFormatDescriptor;

/**
 * @class CAsterixFormat
 *
 * @brief The Asterix formatter class.
 */
class CAsterixFormat : public CBaseFormat {
public:

    /**
     * Public enumerator for supported formats identification. Should be used by
     * public methods which require format type as a parameter.
     */
    enum {
        ERaw = 0,         // Raw Asterix format
        EPcap,          // PCAP file format
        ETxt,           // textual output (human readable)
        EFinal,         // Final format
        EXML,           // XML (Extensible Markup Language) format, compact line-delimited form (suitable for parsing)
        EXMLH,          // XML (Extensible Markup Language) format, human readable form (suitable for file storage)
        EJSON,          // JSON (JavaScript Object Notation) format, compact line-delimited form (suitable for parsing)
        EJSONH,         // JSON (JavaScript Object Notation) format, human readable form (suitable for file storage)
        EJSONE,         // JSON (JavaScript Object Notation) format, with both hex and scaled value and description of each item
        EHDLC,          // HDLC format
        EOradisRaw,     // Raw Asterix format with ORADIS header
        EOradisPcap,    // PCAP file format with ORADIS header
        EOut,           // textual output (one line text, easy for parsing)
        EGPS,           // GPS (timestamped datablocks + 2200 bytes header)
        ETotalFormats
    };

private:

    /**
     * Basic structure (packet) for all format conversions
     */
//    CAsterixPacket _packet;

public:

    /**
     * Default class constructor
     */
    CAsterixFormat() : m_pFormatDescriptor(NULL) {}

    /**
     * Default class destructor.
     */
    virtual ~CAsterixFormat() { if (m_pFormatDescriptor) delete m_pFormatDescriptor; }

    virtual bool ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                            const unsigned int formatType, bool &discard);

    virtual bool WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                             const unsigned int formatType, bool &discard);

    virtual bool ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                               const unsigned int formatType, bool &discard);

    virtual bool HeartbeatProcessing(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                                     const unsigned int formatType);

    virtual CBaseFormatDescriptor *CreateFormatDescriptor
            (const unsigned int formatType, const char *sFormatDescriptor);

    virtual bool GetFormatNo(const char *formatName, unsigned int &formatNo);

    virtual int GetStatus(CBaseDevice &device,
                          const unsigned int formatType, int query = 0);

    virtual bool OnResetInputChannel(CBaseFormatDescriptor &formatDescriptor);

    virtual bool OnResetOutputChannel(unsigned int channel, CBaseFormatDescriptor &formatDescriptor);


private:

    // Supported formats name string list
    static const char *_FormatName[ETotalFormats];

    CBaseFormatDescriptor *m_pFormatDescriptor;


};

#endif
