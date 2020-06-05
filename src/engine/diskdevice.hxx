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
#ifndef DISKDEVICE_HXX__
#define DISKDEVICE_HXX__

#include <sys/param.h>
#include <unistd.h>

#include "basedevice.hxx"
#include "descriptor.hxx"

// mode descriptor values (binary OR-ed)

// If set, read the input file from beginning to the end, close it when reaching EOF.
// otherwise, continue trying to read even on EOF (useful for pipes and tail-like operations)
// Applies to single-file read operations only.
// decimal value:                    1
#define DD_MODE_READONCE    0x00000001

// if set, write file in create mode (i.e. erase previous file, if any, and start from scratch)
// otherwise, append to an existing file, if any
// Applies to single or multi-file write operations only.
// decimal value:                    2
#define DD_MODE_WRITENEW    0x00000002

// if set, file(s) successfully read will be renamed to <filename>.cvt_<pid>
// otherwise, no action is performed
// DD_MODE_READONCE must also be set. Applies to single or multi-file read operations only.
// decimal value:                    4
#define DD_MODE_MARKDONE    0x00000004

// if set, and the file does not exist, device will wait for the file to be created (uses delay parameter in descriptor)
// otherwise, if not set, device will fail immediately
// decimal value:                    8
#define DD_MODE_WAITFILE    0x00000008

// if set, will force fflush() after each fwrite()
// useful for debug/console output
// decimal value:                   16
#define DD_MODE_FLUSHWRITE    0x00000010

// if set, each packet will be written in a separate file (supported for output only)
// output files will be named base_nnnnnnnn.ext (or base_nnnnnnnn), where '_nnnnnnnn' will be inserted (or appended) as in examples
// otherwise, a single file is used
// decimal value:                   32
#define DD_MODE_PACKETFILE    0x00000020

// if set, the file will be read continuously, i.e. file pointer will be reset to 0 when reaching EOF
// must not be used with DD_MODE_READONCE and DD_MODE_MARKDONE
// decimal value:                   64
#define DD_MODE_READLOOP    0x00000040

// if set, the output file will be opened under a temporary name until it is closed
// only then it will be renamed to the specified file name; supported for output only
// cannot be combined with DD_MODE_MARKDONE
// must be combined with DD_MODE_WRITENEW
// decimal value:                  128
//#define DD_MODE_TEMPNAME    0x00000080

/**
 * @class CDiskDevice
 *
 * @brief The disk device.
 *
 * @see   <CDeviceFactory>
 *        <CBaseDevice>
 *        <CDescriptor>
 */
class CDiskDevice : public CBaseDevice {
private:
    FILE *_fileStream;
    bool _input;
    unsigned int _inputDelay;
    unsigned int _mode;
    char _fileName[MAXPATHLEN+1];
    char _baseName[MAXPATHLEN+1];
    char _tempName[MAXPATHLEN+1];
    unsigned int _seqNo;
    bool _delayOpen;

public:

    /**
     * Class constructor which uses descriptor
     */
    CDiskDevice(CDescriptor &descriptor);


    /**
     * Class destructor.
     */
    virtual ~CDiskDevice();

    virtual bool Read(void *data, size_t len);

    virtual bool Write(const void *data, size_t len);

    virtual bool Select(const unsigned int secondsToWait);

    virtual bool IoCtrl(const unsigned int command, const void *data = 0, size_t len = 0);

    virtual bool IsPacketDevice() { return false; };

    virtual bool IsOpened();

    virtual unsigned int BytesLeftToRead(); // return number of bytes left to read or 0 if unknown

private:
    bool Init(const char *path);

    bool OpenOutputFile(const char *path, bool openNow = false);

    void Close();

    bool DoneWithFile(bool allDone = false);

    bool DoneAll();

    bool NextFile();

    char *NextFileName();
};

#endif

