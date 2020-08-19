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
#include <unistd.h>
#include <stdarg.h>

#include "asterix.h"
#include "asterixformat.hxx"
#include "asterixrawsubformat.hxx"
#include "asterixpcapsubformat.hxx"
#include "asterixfinalsubformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixhdlcsubformat.hxx"
#include "asterixgpssubformat.hxx"

#include "Tracer.h"
#include "XMLParser.h"

// Supported Asterix format names
const char *CAsterixFormat::_FormatName[CAsterixFormat::ETotalFormats] =
        {
                "ASTERIX_RAW",
                "ASTERIX_PCAP",
                "ASTERIX_TXT",
                "ASTERIX_FINAL",
                "ASTERIX_XML",
                "ASTERIX_XMLH",
                "ASTERIX_JSON",
                "ASTERIX_JSONH",
                "ASTERIX_JSONE",
                "ASTERIX_HDLC",
                "ASTERIX_ORADIS_RAW",
                "ASTERIX_ORADIS_PCAP",
                "ASTERIX_OUT",
                "ASTERIX_GPS"
        };

//CBaseFormatDescriptor* CAsterixFormat::m_pFormatDescriptor = NULL;


bool
CAsterixFormat::ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, const unsigned int formatType,
                           bool &discard) {
    switch (formatType) {
        case ERaw:
            return CAsterixRawSubformat::ReadPacket(formatDescriptor, device, discard);
        case EPcap:
            return CAsterixPcapSubformat::ReadPacket(formatDescriptor, device, discard);
        case EOradisRaw:
            return CAsterixRawSubformat::ReadPacket(formatDescriptor, device, discard, true);
        case EOradisPcap:
            return CAsterixPcapSubformat::ReadPacket(formatDescriptor, device, discard, true);
        case EFinal:
            return CAsterixFinalSubformat::ReadPacket(formatDescriptor, device, discard);
        case EHDLC:
            return CAsterixHDLCSubformat::ReadPacket(formatDescriptor, device, discard);
        case EGPS:
            return CAsterixGPSSubformat::ReadPacket(formatDescriptor, device, discard);
        case EXML:
        case EXMLH:
        case EJSON:
        case EJSONH:
        case EJSONE:
            //todo not supported
            return false;
        default:
            ASSERT(0);
    }
    LOGERROR(1, "Unsupported format type %d.\n", formatType);
    return false;
}

bool
CAsterixFormat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, const unsigned int formatType,
                            bool &discard) {
    std::string strPacketDescription;

    switch (formatType) {
        case ERaw:
            return CAsterixRawSubformat::WritePacket(formatDescriptor, device, discard); //TODO
        case EPcap:
            return CAsterixPcapSubformat::WritePacket(formatDescriptor, device, discard);//TODO
        case EOradisRaw:
            return CAsterixRawSubformat::WritePacket(formatDescriptor, device, discard, true); //TODO
        case EOradisPcap:
            return CAsterixPcapSubformat::WritePacket(formatDescriptor, device, discard, true);//TODO
        case EFinal:
            return CAsterixFinalSubformat::WritePacket(formatDescriptor, device, discard); // TODO
        case EHDLC:
            return CAsterixHDLCSubformat::WritePacket(formatDescriptor, device, discard);//TODO
        case EGPS:
            return CAsterixGPSSubformat::WritePacket(formatDescriptor, device, discard);//TODO
        case EXML:
        case EXMLH:
        case EJSON:
        case EJSONH:
        case EJSONE:
        case ETxt:
        case EOut: {
            CAsterixFormatDescriptor &Descriptor((CAsterixFormatDescriptor &) formatDescriptor);

            if (Descriptor.m_pAsterixData == NULL) {
                LOGERROR(1, "Asterix data packet not present\n");
                return true;
            }

            if (!Descriptor.m_pAsterixData->getText(strPacketDescription, formatType)) {
                LOGERROR(1, "Failed to get data packet description\n");
                return false;
            }

            device.Write(strPacketDescription.c_str(), strPacketDescription.length());

            return true;
        }
        default:
            ASSERT(0);
    }
    LOGERROR(1, "Unsupported format type %d.\n", formatType);
    return false;
}


bool CAsterixFormat::ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device,
                                   const unsigned int formatType, bool &discard) {
    discard = false;

    switch (formatType) {
        case ERaw:
            return CAsterixRawSubformat::ProcessPacket(formatDescriptor, device, discard);
        case EPcap:
            return CAsterixPcapSubformat::ProcessPacket(formatDescriptor, device, discard);
        case EOradisRaw:
            return CAsterixRawSubformat::ProcessPacket(formatDescriptor, device, discard, true);
        case EOradisPcap:
            return CAsterixPcapSubformat::ProcessPacket(formatDescriptor, device, discard, true);
        case EFinal:
            return CAsterixFinalSubformat::ProcessPacket(formatDescriptor, device, discard);
        case EHDLC:
            return CAsterixHDLCSubformat::ProcessPacket(formatDescriptor, device, discard);
        case EGPS:
            return CAsterixGPSSubformat::ProcessPacket(formatDescriptor, device, discard);
        case ETxt:
        case EXML:
        case EXMLH:
        case EJSON:
        case EJSONH:
        case EJSONE:
        case EOut:
            return false;
        default:
            ASSERT(0);
    }
    LOGERROR(1, "Unsupported format type %d.\n", formatType);
    return false;
}

bool CAsterixFormat::HeartbeatProcessing(
        CBaseFormatDescriptor &formatDescriptor,
        CBaseDevice &device,
        const unsigned int formatType) {
    switch (formatType) {
        case ERaw:
            return CAsterixRawSubformat::Heartbeat(formatDescriptor, device);
        case EPcap:
            return CAsterixPcapSubformat::Heartbeat(formatDescriptor, device);
        case EOradisRaw:
            return CAsterixRawSubformat::Heartbeat(formatDescriptor, device, true);
        case EOradisPcap:
            return CAsterixPcapSubformat::Heartbeat(formatDescriptor, device, true);
        case EFinal:
            return CAsterixFinalSubformat::Heartbeat(formatDescriptor, device);
        case EHDLC:
            return CAsterixHDLCSubformat::Heartbeat(formatDescriptor, device);
        case EGPS:
            return CAsterixGPSSubformat::Heartbeat(formatDescriptor, device);
        case ETxt:
        case EXML:
        case EXMLH:
        case EJSON:
        case EJSONH:
        case EJSONE:
        case EOut:
            return false;
        default:
            ASSERT(0);
    }
    LOGERROR(1, "Unsupported format type %d.\n", formatType);
    return false;
}

static void debug_trace(char const *format, ...) {
    char buffer[1025];
    va_list args;
    va_start (args, format);
    vsnprintf(buffer, 1024, format, args);
    va_end (args);
    strcat(buffer, "\n");
    LOGERROR(1, "%s", buffer);
}

CBaseFormatDescriptor *CAsterixFormat::CreateFormatDescriptor
        (const unsigned int formatType, const char *sFormatDescriptor) {
    if (!m_pFormatDescriptor) {
        char inputFile[256];

        Tracer::Configure(debug_trace);

        // initialize Fulliautomatix engine
        AsterixDefinition *pDefinition = new AsterixDefinition();

        // open asterix.ini file
        FILE *fpini = fopen(gAsterixDefinitionsFile, "rt");
        if (!fpini) {
            LOGERROR(1, "Failed to open definitions file");
            return NULL;
        }

        // extract ini file path
        std::string strInifile = gAsterixDefinitionsFile;
        std::string strInifilePath;
        int index = strInifile.find_last_of('\\');
        if (index < 0)
            index = strInifile.find_last_of('/');
        if (index > 0) {
            strInifilePath = strInifile.substr(0, index + 1);
        }

        while (fgets(inputFile, sizeof(inputFile), fpini)) {
            std::string strInputFile;

            // remove trailing /n from filename
            int lastChar = strlen(inputFile) - 1;
            while (lastChar >= 0 && (inputFile[lastChar] == '\r' || inputFile[lastChar] == '\n')) {
                inputFile[lastChar] = 0;
                lastChar--;
            }
            if (lastChar <= 0)
                continue;

            strInputFile = inputFile;

            FILE *fp = fopen(strInputFile.c_str(), "rt");
            if (!fp) {
                // try in folder where is ini file
                if (!strInifilePath.empty()) {
                    std::string strfilepath = strInifilePath + strInputFile;
                    fp = fopen(strfilepath.c_str(), "rt");
                }

                if (!fp) {
                    LOGERROR(1, "Failed to open definitions file: %s\n", inputFile);
                    continue;
                }
            }

            // parse format file
            XMLParser Parser;
            if (!Parser.Parse(fp, pDefinition, inputFile)) {
                LOGERROR(1, "Failed to parse definitions file: %s\n", strInputFile.c_str());
            }

            fclose(fp);
        };

        fclose(fpini);

        m_pFormatDescriptor = new CAsterixFormatDescriptor(pDefinition);
    }
    return m_pFormatDescriptor;
}


bool CAsterixFormat::GetFormatNo(const char *formatName, unsigned int &formatType) {
    bool found = false;
    unsigned int i;

    for (i = 0; (i < ETotalFormats) && (!found); i++) {
        ASSERT(formatName);
        if (strcasecmp(formatName, CAsterixFormat::_FormatName[i]) == 0) {
            found = true;
            formatType = i;
        }
    }

    return found;
}

int CAsterixFormat::GetStatus(CBaseDevice &device, const unsigned int formatType, int query) {
    if (device.IsOpened() == false)
        return STS_NO_DATA;

    return 0;
}


bool CAsterixFormat::OnResetInputChannel(CBaseFormatDescriptor &formatDescriptor) {
    return false;
}


bool CAsterixFormat::OnResetOutputChannel(unsigned int channel, CBaseFormatDescriptor &formatDescriptor) {
    return true;
}




