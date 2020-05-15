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
#ifndef BASEDEVICE_HXX__
#define BASEDEVICE_HXX__

/**
 * @class CBaseDevice
 *
 * @brief The base class for all input and output devices.
 *        Used by <CDeviceFactory>.
 *
 * @see   <CDeviceFactory>
 */
class CBaseDevice {
protected:
    bool _opened;
    bool _onstart; // if true device is on start (e.g. beginning of file)
    unsigned int _nReadErrors;
    unsigned int _nWriteErrors;
    unsigned int _nSeqReadErrors;
    unsigned int _nSeqWriteErrors;

public:

    // IoCtrl commands
    enum {
        EReset,
        EPacketDone,
        EAllDone,
        EIsLastPacket
    };


    /**
     * Empty constructor
     */
    CBaseDevice() : _opened(false), _onstart(true),
                    _nReadErrors(0), _nWriteErrors(0),
                    _nSeqReadErrors(0), _nSeqWriteErrors(0) {}

    /**
     * Pure virtual destructor.
     */
    virtual ~CBaseDevice() {}

    virtual bool Read(void *data, size_t len) = 0;

    virtual bool Read(void *data, size_t *len) { return Read(data, *len); };

    virtual bool Write(const void *data, size_t len) = 0;

    virtual bool Select(const unsigned int secondsToWait = 0) = 0;

    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0) = 0;

    virtual bool IsPacketDevice() = 0;

    virtual unsigned int MaxPacketSize() { return 0; } // return maximal packet size (only for packet devices)
    virtual bool IsOpened() { return _opened; }

    virtual bool IsOnStart() { return _onstart; } // if true device is on start (e.g. beginning of file)
    virtual unsigned int BytesLeftToRead() { return 0; } // return number of bytes left to read or 0 if unknown
    virtual unsigned int GetNReadErrors(bool bSeq = false) { return bSeq ? _nSeqReadErrors : _nReadErrors; }

    virtual unsigned int GetNWriteErrors(bool bSeq = false) { return bSeq ? _nSeqWriteErrors : _nWriteErrors; }

    virtual unsigned int GetNErrors(bool bSeq = false) { return GetNReadErrors(bSeq) + GetNWriteErrors(bSeq); }

    virtual void ResetReadErrors(bool bSeq = false, unsigned int n = 0) { (bSeq ? _nSeqReadErrors : _nReadErrors) = n; }

    virtual void ResetWriteErrors(bool bSeq = false, unsigned int n = 0) {
        (bSeq ? _nSeqWriteErrors : _nWriteErrors) = n;
    }

    virtual void ResetErrors(bool bSeq = false) {
        ResetReadErrors(bSeq);
        ResetWriteErrors(bSeq);
    }

    virtual void ResetAllErrors() {
        ResetErrors(false);
        ResetErrors(true);
    }

protected: // to be used by derived class for error counting
    virtual void CountReadError() {
        _nReadErrors++;
        _nSeqReadErrors++;
    }

    virtual void CountWriteError() {
        _nWriteErrors++;
        _nSeqWriteErrors++;
    }
};

#endif

