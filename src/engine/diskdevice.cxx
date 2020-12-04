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
// Standard includes
#include <stdio.h>
#include <string>
#include <string.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <time.h>
#include <errno.h>

// Local includes
#include "asterix.h"
#include "diskdevice.hxx"


// TO-DO: implement closing existing file and creating a new after defined period
// (for logging purposes - circular logs)

void findAndReplaceAll(std::string &data, std::string toSearch, std::string replaceStr) {
    // Get the first occurrence
    size_t pos = data.find(toSearch);

    // Repeat till end is reached
    while (pos != std::string::npos) {
        // Replace this occurrence of Sub String
        data.replace(pos, toSearch.size(), replaceStr);
        // Get the next occurrence from the current position
        pos = data.find(toSearch, pos + toSearch.size());
    }
}

CDiskDevice::CDiskDevice(CDescriptor &descriptor)
        : _inputDelay(0), _mode(0), _seqNo(0) {
    memset(_fileName, 0, sizeof(_fileName));
    memset(_baseName, 0, sizeof(_baseName));
    memset(_tempName, 0, sizeof(_tempName));

    const char *spath = descriptor.GetFirst();

    // Remove '\\ ' with ' '. This is for Linux paths with space inside.
    std::string strPath = spath;
    findAndReplaceAll(strPath, "\\ ", " ");
    spath = strPath.c_str();

    const char *inputDelay = descriptor.GetNext();
    const char *smode = descriptor.GetNext();

    // Disk is by default output device, but with second argument
    // it can be set as input device (mainly for testing purposes)
    _input = false;

    // if true, opening an output file will be delayed until first write
    // no effect in input mode
    _delayOpen = true;

    // Path argument
    if (spath == NULL) {
        LOGERROR(1, "Path not specified\n");
    }

    // Input delay, for testing: specifies the delay between consequent Read() or Write() operations
    if ((inputDelay != NULL) && (strlen(inputDelay) > 0)) {
        _inputDelay = atoi(inputDelay);
        _input = true;
    }

    // specifies the mode of operation on files, directories or groups of files
    if ((smode != NULL) && (strlen(smode) > 0)) {
        _mode = atoi(smode);
    }

    // Call default initialization
    Init(spath);
}


CDiskDevice::~CDiskDevice() {
    Close();
}


bool CDiskDevice::Read(void *data, size_t len) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot read due to not properly initialized interface.\n");
        CountReadError();
        return false;
    }

    if (!_input) {
        LOGERROR(1, "Read not supported in output device mode.\n");
        CountReadError();
        return false;
    }

    // Read the message from a file (blocking)
    size_t bytesRead = fread(data, 1, len, _fileStream);
    if (bytesRead != len) {
        LOGERROR(1, "Error reading from file.\n");
        CountReadError();
        return false;
    }

    _onstart = false;

//    LOGDEBUG(ZONE_DISKDEVICE, "Read message from file.\n");

    if (BytesLeftToRead() == 0) {
        if (_mode & DD_MODE_READLOOP) {
            // restart reading from the beginning of the same file
            _onstart = (fseek(_fileStream, 0, SEEK_SET) == 0);
        } else {
            // finished reading current file
            DoneWithFile();
        }
    }

    ResetReadErrors(true);
    return true;
}


bool CDiskDevice::Write(const void *data, size_t len) {
    // Check if interface was set-up correctly
    if (!_opened) {
        LOGERROR(1, "Cannot write: file not open.\n");
        CountWriteError();
        return false;
    }

    if (_input) {
        LOGERROR(1, "Write not supported in input device mode.\n");
        CountWriteError();
        return false;
    }

    if (_fileStream == NULL) {
        if (_delayOpen) {
            if (!OpenOutputFile(_fileName, true)) {
                LOGERROR(1, "Cannot write: opening of file '%s' failed.\n", _fileName);
                CountWriteError();
                return false;
            }
        } else {
            LOGERROR(1, "Cannot write due to not properly initialized interface.\n");
            CountWriteError();
            return false;
        }
    }

    // Write the message to a file (blocking)
    size_t bytesWrote = fwrite(data, 1, len, _fileStream);
    if (bytesWrote != len) {
        LOGERROR(1, "Error writing to file.\n");
        CountWriteError();
        return false;
    }

    _onstart = false;

    if (_mode & DD_MODE_FLUSHWRITE)
        fflush(_fileStream);

    LOGDEBUG(ZONE_DISKDEVICE, "Wrote message to file.\n");

    ResetWriteErrors(true);
    return true;
}


bool CDiskDevice::Select(const unsigned int secondsToWait) {
    struct timeval tv;
    memset(&tv, 0, sizeof(tv));

    if ((!_opened) && (_mode & DD_MODE_WAITFILE)) // wait for the file to appear
    {
        // DD_MODE_WAITFILE makes sense only for input
        ASSERT(_input);

        bool rc = false;
        double timeElapsed = 0;

        do {
            LOGDEBUG(1, "Waiting for input file...\n");
            if (secondsToWait > 0) {
                // use heartbeat interval
                tv.tv_sec = secondsToWait;
                tv.tv_usec = 0;
            } else {
                // use 10*_inputDelay interval
                tv.tv_sec = (_inputDelay * 1000 * 10) / 1000000;
                tv.tv_usec = (_inputDelay * 1000 * 10) % 1000000;
            }

            // wait...
            select(0, NULL, NULL, NULL, &tv);

            timeElapsed += tv.tv_sec + (tv.tv_usec / 1000000.0);

            rc = Init(_fileName);

            if ((secondsToWait > 0) && (timeElapsed >= secondsToWait))
                break;
        } while (!(_opened && rc));
    }

    if (!_opened) // Check if interface was set-up correctly
    {
        if (!(_mode & DD_MODE_WAITFILE)) {
            LOGNOTIFY(gVerbose, "Done with file. Exiting application.\n");
            return true;
        }

        return false;
    }

    // This is only dummy select to simulate delay between reading/writing messages from/to file
    tv.tv_sec = (_inputDelay * 1000) / 1000000;
    tv.tv_usec = (_inputDelay * 1000) % 1000000;
    select(0, NULL, NULL, NULL, &tv);
//    sleep(_inputDelay);

    if (_input && _opened) {
        // if not in continuous read mode, stop on EOF
        if ((_mode & DD_MODE_READONCE) && (BytesLeftToRead() == 0))
            return DoneWithFile();
    }

    return true;
}


bool CDiskDevice::Init(const char *path) {
    _opened = false;
    _fileStream = NULL;
    ResetAllErrors();

    char *fname = (char *) path;

    if (_input) {
        // Initialize as input device

        ASSERT(!(_mode & DD_MODE_PACKETFILE)); // not supported for input
//         ASSERT( !(_mode & DD_MODE_TEMPNAME) ); // not supported for input

        _fileStream = fopen(fname, "rb");
        _onstart = true;
    } else {
/*
        if( _mode & DD_MODE_TEMPNAME )
        {
            if( (_tempName == NULL) || (strlen(_tempName) == 0) )
            {
                strncpy(_tempName, fname, MAXPATHLEN); // store actual name in _tempName
                static char tn[MAXPATHLEN];
                strncpy(tn, fname, MAXPATHLEN-11);
                strcat(tn, ".tmp.XXXXXX");
                fname = mktemp(tn);
                if((fname == NULL) || (strlen(fname) == 0))
                {
                    LOGERROR(1, "Cannot initialize temporary file name.\n");
                    return false;
                }
            }
        }
*/
        // Initialize as output device
        _opened = OpenOutputFile(fname);
    }

    if (_fileStream == NULL) {
        // special case when DD_MODE_WAITFILE is set and we're waiting for a file to appear
        // in this case we never fail here
        if (_input && (_mode & DD_MODE_WAITFILE)) {
            if (fname != _fileName)
                strncpy(_fileName, fname, MAXPATHLEN);

            return true;
        }

        if (!((!_input) && (_delayOpen))) {
            // DD_MODE_WAITFILE or _delayOpen not set, fail
            LOGERROR(1, "Cannot open file '%s'\n", fname);
            return false;
        }
    }

    if (!(_mode & DD_MODE_PACKETFILE)) {
        if (fname != _fileName)
            strncpy(_fileName, fname, MAXPATHLEN);
    }

    if (_input)
        _opened = (_fileStream != NULL);

    LOGDEBUG((_input && ZONE_DISKDEVICE), "Opened input file '%s'\n", _fileName);

    return _opened;
}


bool CDiskDevice::OpenOutputFile(const char *path, bool openNow) {
    char *fname = (char *) path;

    ASSERT(!_input);
    ASSERT(_fileStream == NULL);
    ASSERT(fname != NULL);

    // initialize first file name for packeted files
    if (_mode & DD_MODE_PACKETFILE) {
        if (_baseName[0] == '\0') {
            if (_baseName != fname)
                strncpy(_baseName, fname, MAXPATHLEN);
            fname = NextFileName();
        }
    }

    if (openNow || (!_delayOpen)) {
        // open the file
        _fileStream = fopen(fname, (_mode & DD_MODE_WRITENEW) ? "w" : "a");
        if (_fileStream == NULL) {
            LOGERROR(1, "Cannot open file '%s'\n", fname);
            return false;
        } else {
            if (fname != _fileName)
                strncpy(_fileName, fname, MAXPATHLEN);
            LOGDEBUG(ZONE_DISKDEVICE, "Opened output file '%s'\n", _fileName);
            _opened = true;
            _onstart = true;
            return true;
        }
    } else {
        // delayed opening of output files
        // only check whether the file can be opened
        if (access(fname, W_OK) == -1) {
            if (errno != ENOENT) {
                LOGERROR(1, "File '%s' cannot be open or written to.\n", fname);
                return false;
            }
        }

        if (fname != _fileName)
            strncpy(_fileName, fname, MAXPATHLEN);

        LOGDEBUG(ZONE_DISKDEVICE, "Output file '%s' will be opened on first write\n", _fileName);
        return true;
    }

    ASSERT(false);
    return false;
}


bool CDiskDevice::DoneWithFile(bool allDone) {
    if (_input) {
        if (_mode & DD_MODE_READONCE)
            Close();

        LOGDEBUG(ZONE_DISKDEVICE, "Done reading file '%s'\n", _fileName);
    } else {
        Close();
        LOGDEBUG(ZONE_DISKDEVICE, "Done writing file '%s'\n", _fileName);
    }

    if (_mode & DD_MODE_MARKDONE) {
        // rename the file that has been processed
        if (strlen(_fileName) >= MAXPATHLEN-32) {
            LOGERROR(1, "File path too long to be renamed.\n");
        }
        else {
            char newName[MAXPATHLEN+32+1];
            char sfxFormat[10];
            char suffix[32];
            const char *sfxBase = _input ? ".ci%d_%%y%%m%%d%%H%%M%%S"
                                         : ".co%d_%%y%%m%%d%%H%%M%%S";

            sprintf(sfxFormat, sfxBase, (int) getpid());

            time_t t = time(NULL);
            struct tm *stm = gmtime(&t);
            strftime(suffix, 25, sfxFormat, stm);

            strncpy(newName, _fileName, MAXPATHLEN);
            strcat(newName, suffix);

            if (rename(_fileName, newName)) {
                LOGERROR(1, "Rename of file failed\n");
            } else {
                LOGINFO(ZONE_DISKDEVICE, "File '%s' renamed to '%s'\n", _fileName, newName);
            }
        }
    } else if ((!_input) && (_mode & DD_MODE_PACKETFILE)) {
        // delete output file if empty (the last file in sequence is open regardless of input availability)
        struct stat fs;
        if (stat(_fileName, &fs) == 0) {
            if (fs.st_size == 0) {
                unlink(_fileName);
                LOGDEBUG(ZONE_DISKDEVICE, "Empty output file '%s' deleted\n", _fileName);
            }
        }

    }

    // return true if there are more files to process
    return allDone ? DoneAll() : NextFile();
}


unsigned int CDiskDevice::BytesLeftToRead() {
    if ((!_input) || (!_opened) || (!_fileStream)) {
//        ASSERT(false);
        return 0;
    }

    int pos = ftell(_fileStream);
    if (pos < 0)
        return 0;

    struct stat fs;
    if (fstat(fileno(_fileStream), &fs))
        return 0;

    return fs.st_size - pos;
}


bool CDiskDevice::NextFile() {
    Close();

    // go to the next file, if specified so by _mode
    // for input, currently it will try to re-open the file with the same name
    // for output, multiple file output is supported if DD_MODE_PACKETFILE is set

    if (_mode & DD_MODE_PACKETFILE) {
        return Init(NextFileName());
    } else {
        // there's no next file, so we're done
        return DoneAll();
    }
}


char *CDiskDevice::NextFileName() {
    ASSERT(_mode & DD_MODE_PACKETFILE);
    ASSERT(!_input); // only output is supported
    ASSERT(!(_mode & DD_MODE_MARKDONE)); // not compatible
    ASSERT(strlen(_baseName) > 0); // should have been set in Init()

    static char cnt[32];
    static char newName[MAXPATHLEN+1];

    sprintf(cnt, "_%08d", _seqNo);

    char *posExt = strrchr(_baseName, '.');
    if (posExt) {
        strncpy(newName, _baseName, posExt - _baseName); // base name
        strncpy(newName + (posExt - _baseName), cnt, MAXPATHLEN-(posExt - _baseName)); // sequence counter
        strcat(newName, posExt); // extension
    } else {
        strncpy(newName, _baseName, MAXPATHLEN-32);
        strcat(newName, cnt);
    }

    return newName;
}


void CDiskDevice::Close() {
    if (_fileStream != NULL) {
        ASSERT(_opened);
        fflush(_fileStream);
        fclose(_fileStream);
        _fileStream = NULL;
/*
        if(_mode & DD_MODE_TEMPNAME)
        {
            // check if _tempName already exists before renaming !!!
            // this would erase previous file, so rename old file to keep it
            // the best way to go is to rename (backup) the existing (old) file
            // on Linux, renaming the file won't affect any application that may have it already open

            struct stat fs;
            bool mvrc = (stat(_tempName, &fs) != 0);
            if(!mvrc)
            {
                LOGWARNING(1, "File '%s' already exists.\n", _tempName);

                // rename the existing file to <name>.nnnn, where nnnn is the first available decimal number with leading zeros
                char backupName[MAXPATHLEN];
                strcpy(backupName, _tempName);
                int bnPos=strlen(backupName);

                if(bnPos + 6 <= MAXPATHLEN)
                {
                    for(int b=0; b<=9999; b++)
                    {
                        sprintf(backupName+bnPos, ".%04d", b);
                        mvrc = (stat(backupName, &fs) != 0);
                        if(mvrc)
                            break;
                    }

                    if(mvrc)
                    {
                        if(rename(_tempName, backupName))
                        {
                            LOGERROR(1, "Cannot rename existing file %s to %s.\n", _tempName, backupName);
                        }
                        else
                        {
                            LOGWARNING(1, "Existing file '%s' renamed to '%s'\n", _tempName, backupName);
                        }
                    }
                }
                else
                {
                    LOGERROR(1, "Cannot rename existing file %s, file name too long.\n", _tempName);
                }
            }
            if(mvrc)
            {
                if(rename(_fileName, _tempName))
                {
                    LOGERROR(1, "Cannot rename %s to %s.\n", _fileName, _tempName);
                }
                else
                {
                    LOGINFO(ZONE_DISKDEVICE, "File '%s' renamed to '%s'\n", _fileName, _tempName);
                }
            }

            strcpy(_fileName, _tempName);
            _tempName[0] = '\0';
        }
*/
    }

    _opened = false;
}


bool CDiskDevice::IoCtrl(const unsigned int command, const void *data, size_t len) {
    static bool result = false;

    switch (command) {
        case EReset:
            if (_opened && _fileStream) {
                if (_input) // seek to the beginning of the input file
                {
                    _onstart = result = (fseek(_fileStream, 0, SEEK_SET) == 0);
                }
            }
            ResetAllErrors();
            break;
        case EPacketDone:
            if ((data != NULL) && (len == sizeof(unsigned int)))
                _seqNo = *(unsigned int *) data;

            if ((!_input) && (_mode & DD_MODE_PACKETFILE))
                result = DoneWithFile();
            else
                result = false;
            break;
        case EAllDone:
            result = DoneWithFile(true);
            break;
        case EIsLastPacket:
            if (_input)
                return !_opened;
            else
                    ASSERT(0);
            break;
        default:
            result = false;
            break;
    }

    return result;
}


bool CDiskDevice::DoneAll() {
    if (_input) {
        return !_opened;
    } else {
        Close();

        // reinitialise the device
        // _baseName must be empty for successful initialisation for DD_MODE_PACKETFILE
        // but we must keep the base name
        char tmpName[MAXPATHLEN+1];

        if (_mode & DD_MODE_PACKETFILE)
            strncpy(tmpName, _baseName, MAXPATHLEN);
        else
            strncpy(tmpName, _fileName, MAXPATHLEN);

        memset(_fileName, 0, sizeof(_fileName));
        memset(_baseName, 0, sizeof(_baseName));
        _seqNo = 0;

        if (tmpName[0] != '\0')
            return Init(tmpName);

        return true;
    }
}


bool CDiskDevice::IsOpened() {
    if (_mode & DD_MODE_WAITFILE)
        return true;

    return _opened;
}
