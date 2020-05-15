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
#ifndef BASEFORMAT_HXX__
#define BASEFORMAT_HXX__

#include "basedevice.hxx"

class CBasePacket;

class CBaseFormatDescriptor;

/**
 * @class CBaseFormat
 * 
 * @brief The base format virtual class.
 */
class CBaseFormat {

public:

    /**
     * Empty constructor
     */
    CBaseFormat() {}

    /**
     * Pure virtual destructor.
     */
    virtual ~CBaseFormat() {}

    virtual bool ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                            const unsigned int formatType, bool &discard) = 0;

    virtual bool WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                             const unsigned int formatType, bool &discard) = 0;

    virtual bool ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                               const unsigned int formatType, bool &discard) = 0;

    virtual bool HeartbeatProcessing(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                                     const unsigned int formatType) = 0;

    virtual CBaseFormatDescriptor *CreateFormatDescriptor
            (const unsigned int formatNo, const char *sFormatDescriptor) = 0;

    virtual bool GetFormatNo(const char *formatName, unsigned int &formatNo) = 0;

    virtual int GetStatus(CBaseDevice &device,
                          const unsigned int formatType, int query = 0) = 0;

    virtual bool OnResetInputChannel(CBaseFormatDescriptor &formatDescriptor) = 0;

    virtual bool OnResetOutputChannel(unsigned int channel, CBaseFormatDescriptor &formatDescriptor) = 0;
};

#endif

