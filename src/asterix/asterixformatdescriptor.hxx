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
#ifndef ASTERIXPCAPFORMATDESCRIPTOR_HXX__
#define ASTERIXPCAPFORMATDESCRIPTOR_HXX__

#include "baseformatdescriptor.hxx"
#include "InputParser.h"

class AsterixDefinition;

#define DELETE_BUFFER_IF_LARGER 64*1024

/**
 * @class CAsterixPcapFormatDescriptor
 *
 * @brief Description of pcap Asterix format
 *
 */
class CAsterixFormatDescriptor : public CBaseFormatDescriptor {
private:

public:
    /**
     * Empty constructor
     */
    CAsterixFormatDescriptor(AsterixDefinition *pDefinition) :
            m_InputParser(pDefinition),
            m_pAsterixData(NULL),
            m_ePcapNetworkType(CAsterixFormatDescriptor::ePcapNetworkType(0)),
            m_bInvertByteOrder(true),
            m_pBuffer(NULL),
            m_nBufferSize(0),
            m_nDataSize(0),
            m_nTimeStamp(0) {
    }

    /**
     * Pure virtual destructor.
     */
    virtual ~CAsterixFormatDescriptor() {
        if (m_pBuffer) {
            delete[] m_pBuffer;
        }
        if (m_pAsterixData) {
            delete m_pAsterixData;
        }
    }

    InputParser m_InputParser;
    AsterixData *m_pAsterixData;

    const unsigned char *GetNewBuffer(unsigned int len) {
        if (len > m_nBufferSize || m_nBufferSize > DELETE_BUFFER_IF_LARGER) {
            if (m_pBuffer)
                delete[] m_pBuffer;

            m_pBuffer = new unsigned char[len];
        }
        m_nBufferSize = len;
        m_nDataSize = 0;
        return m_pBuffer;
    }

    const unsigned char *GetBuffer() {
        return m_pBuffer;
    }

    unsigned int GetBufferLen() {
        return m_nBufferSize;
    }

    void SetBufferLen(unsigned int len) {
        m_nBufferSize = len;
    }

    unsigned int GetDataLen() {
        return m_nDataSize;
    }

    void SetDataLen(unsigned int len) {
        m_nDataSize = len;
    }

    // used only when format is GPS
    unsigned long GetTimeStamp() {
        return m_nTimeStamp;
    }

    void SetTimeStamp(unsigned long ts) {
        m_nTimeStamp = ts;
    }

    // used only in PCAP (TODO)
    typedef enum {
        NET_ETHERNET = 0,
        NET_LINUX = 1
    } ePcapNetworkType;

    ePcapNetworkType m_ePcapNetworkType;
    bool m_bInvertByteOrder;

    std::string printDescriptor() { return m_InputParser.printDefinition(); }

    bool filterOutItem(int cat, std::string item, const char *name) {
        return m_InputParser.filterOutItem(cat, item, name);
    }

    bool isFiltered(int cat, std::string item, const char *name) { return m_InputParser.isFiltered(cat, item, name); }

private:
    const unsigned char *m_pBuffer; // input buffer
    unsigned int m_nBufferSize; // input buffer size
    unsigned int m_nDataSize; // size of data in buffer
    unsigned long m_nTimeStamp; // Date and time when this packet was captured. This value is in seconds since January 1, 1970 00:00:00 GMT
};

#endif
