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
 * AUTHORS:  Damir Salantic, Croatia Control Ltd.
 * 			 Jurica Baricevic, Croatia Control Ltd.
 *			 Zeljko Harjac, Croatia Control Ltd.
 *			 Darko Markus, Croatia Control Ltd.
 *
 */

#include <string>
#include <iostream>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "asterix.h"
#include "version.h"
#include "Tracer.h"
#include "../engine/converterengine.hxx"
#include "../engine/channelfactory.hxx"

bool gVerbose = false;
bool gSynchronous = false;
bool gForceRouting = false;
int gHeartbeat = 0;
const char *gAsterixDefinitionsFile = NULL;
bool gFiltering = false;

static void DisplayCopyright() {
    std::cerr << "Asterix " _VERSION_STR " " __DATE__;
#ifdef _DEBUG
    std::cerr << " DEBUG version";
#endif
    std::cerr
            << "\n\nCopyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)\nThis program comes with ABSOLUTELY NO WARRANTY.\nThis is free software, and you are welcome to redistribute it\nunder certain conditions. See COPYING file for details.\n";
}

static void show_usage(std::string name) {
    DisplayCopyright();

    std::cerr
            << "\nReads and parses ASTERIX data from stdin, file or network multicast stream\nand prints it in textual presentation on standard output.\n\n"
            << "Usage:\n"
            << name
            << " [-h] [-V] [-v] [-L] [-o] [-s] [-P|-O|-R|-F|-H] [-l|-x|-j|-jh|-je] [-d filename] [-LF filename] -f filename|-i (mcastaddress:ipaddress:port[:srcaddress]@)+"
            << "\n\nOptions:"
            << "\n\t-h,--help\tShow this help message and exit."
            << "\n\t-V,--version\tShow version information and exit."
            << "\n\t-v,--verbose\tShow more information during program execution."
            << "\n\t-d,--def\tXML protocol definitions filenames are listed in specified filename. By default are listed in config/asterix.ini"
            << "\n\t-L,--list\tList all configured ASTERIX items. Mark which items are filtered."
            << "\n\t-LF,--filter\tPrintout only items listed in configured file."
            << "\n\t-o,--loop\tLoop the input file. Only relevant when file is data source."
            << "\n\t-s,--sync\tOutput will be printed synchronously with input file (with time delays between packets). This parameter is used only if input is from file."
            << "\n\nInput format"
            << "\n------------"
            << "\n\t-P,--pcap\tInput is from PCAP file."
            << "\n\t-R,--oradispcap\tInput is from PCAP file and Asterix packet is encapsulated in ORADIS packet."
            << "\n\t-O,--oradis\tAsterix packet is encapsulated in ORADIS packet."
            << "\n\t-F,--final\tAsterix packet is encapsulated in FINAL packet."
            << "\n\t-H,--hdlc\tAsterix packet is encapsulated in HDLC packet."
            << "\n\t-G,--gps\tAsterix packet is encapsulated in GPS packet."
            << "\n\nOutput format"
            << "\n------------"
            << "\n\t-l,--line\tOutput will be printed as one line per item. This format is suitable for parsing."
            << "\n\t-x,--xml\tOutput will be printed in compact line-delimited XML format (one object per line, suitable for parsing)."
            << "\n\t-xh,--xmlh\tOutput will be printed in human readable XML format (suitable for file storage)."
            << "\n\t-j,--json\tOutput will be printed in compact line-delimited JSON format (one object per line, suitable for parsing)."
            << "\n\t-jh,--jsonh\tOutput will be printed in human readable JSON format (suitable for file storage)."
            << "\n\t-je,--json-extensive\tOutput will be printed in extensive JSON format (with both hex and scaled value and description of each item)."
            << "\n\nData source"
            << "\n------------"
            << "\n\t-f filename\tFile generated from libpcap (tcpdump or Wireshark) or file in FINAL or HDLC format.\n\t\t\tFor example: -f filename.pcap"
            << "\n\t-i m:i:p[:s]\tMulticast UDP/IP address:Interface address:Port[:Source address].\n\t\t\tFor example: 232.1.1.12:10.17.58.37:21112:10.17.22.23\n\t\t\tMore than one multicast group could be defined, use @ as separator.\n\t\t\tFor example: 232.1.1.13:10.17.58.37:21112:10.17.22.23@232.1.1.14:10.17.58.37:21112:10.17.22.23"
            << std::endl;
}

int main(int argc, const char *argv[]) {
    std::string strDefinitions = "config/asterix.ini";
    std::string strFileInput;
    std::string strIPInput;
    std::string strFilterFile;
    std::string strInputFormat = "ASTERIX_RAW";
    std::string strOutputFormat = "ASTERIX_TXT";

    bool bListDefinitions = false;
    bool bLoopFile = false;

    for (int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if ((arg == "-h") || (arg == "--help")) {
            show_usage(argv[0]);
            return 0;
        } else if ((arg == "-V") || (arg == "--version")) {
            DisplayCopyright();
            return 0;
        } else if ((arg == "-v") || (arg == "--verbose")) {
            gVerbose = true;
        } else if ((arg == "-s") || (arg == "--sync")) {
            gSynchronous = true;
        } else if ((arg == "-o") || (arg == "--loop")) {
            bLoopFile = true;
        } else if ((arg == "-L") || (arg == "--list")) {
            bListDefinitions = true;
        } else if ((arg == "-LF") || (arg == "--filter")) {
            if (i >= argc - 1) {
                std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
                return 1;
            }
            strFilterFile = argv[++i];
            gFiltering = true;
        } else if ((arg == "-P") || (arg == "--pcap")) {
            if (strInputFormat != "ASTERIX_RAW") {
                std::cerr << "Error: Option -P not allowed because input format already defined as " + strInputFormat
                          << std::endl;
                return 1;
            }
            strInputFormat = "ASTERIX_PCAP";
        } else if ((arg == "-O") || (arg == "--oradis")) {
            if (strInputFormat != "ASTERIX_RAW") {
                std::cerr << "Error: Option -O not allowed because input format already defined as " + strInputFormat
                          << std::endl;
                return 1;
            }
            strInputFormat = "ASTERIX_ORADIS_RAW";
        } else if ((arg == "-R") || (arg == "--oradispcap")) {
            if (strInputFormat != "ASTERIX_RAW") {
                std::cerr << "Error: Option -R not allowed because input format already defined as " + strInputFormat
                          << std::endl;
                return 1;
            }
            strInputFormat = "ASTERIX_ORADIS_PCAP";
        } else if ((arg == "-F") || (arg == "--final")) {
            if (strInputFormat != "ASTERIX_RAW") {
                std::cerr << "Error: Option -F not allowed because input format already defined as " + strInputFormat
                          << std::endl;
                return 1;
            }
            strInputFormat = "ASTERIX_FINAL";
        } else if ((arg == "-H") || (arg == "--hdlc")) {
            if (strInputFormat != "ASTERIX_RAW") {
                std::cerr << "Error: Option -H not allowed because input format already defined as " + strInputFormat
                          << std::endl;
                return 1;
            }
            strInputFormat = "ASTERIX_HDLC";
        } else if ((arg == "-G") || (arg == "--gps")) {
            if (strInputFormat != "ASTERIX_RAW") {
                std::cerr << "Error: Option -H not allowed because input format already defined as " + strInputFormat
                          << std::endl;
                return 1;
            }
            strInputFormat = "ASTERIX_GPS";
        } else if ((arg == "-l") || (arg == "--line")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -l not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_OUT";
        } else if ((arg == "-x") || (arg == "--xml")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -x not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_XML";
        } else if ((arg == "-xh") || (arg == "--xmlh")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -xh not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_XMLH";
        } else if ((arg == "-j") || (arg == "--json")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -j not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_JSON";
        } else if ((arg == "-jh") || (arg == "--jsonh")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -jh not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_JSONH";
        } else if ((arg == "-je") || (arg == "--json-extensive")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -je not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_JSONE";
        } else if ((arg == "-k") || (arg == "--kml")) {
            if (strOutputFormat != "ASTERIX_TXT") {
                std::cerr << "Error: Option -k not allowed because output format already defined as " + strOutputFormat
                          << std::endl;
                return 1;
            }
            strOutputFormat = "ASTERIX_KML";
        } else if ((arg == "-d") || (arg == "--def") || (arg == "--definitions")) {
            if (i >= argc - 1) {
                std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
                return 1;
            }

            strDefinitions = argv[++i];
        } else if ((arg == "-f")) {
            if (i >= argc - 1) {
                std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
                return 1;
            }
            strFileInput = argv[++i];
        } else if ((arg == "-i")) {
            if (i >= argc - 1) {
                std::cerr << "Error: " + arg + " option requires one argument." << std::endl;
                return 1;
            }
            strIPInput = argv[++i];
        }
    }

    // definitions file
    gAsterixDefinitionsFile = strDefinitions.c_str();

    // check for definitions file
    FILE *tmp = fopen(gAsterixDefinitionsFile, "r");
    if (tmp == NULL) {
        std::cerr << "Error: Asterix definitions file " + strDefinitions + " not found." << std::endl;
        exit(2);
    }
    fclose(tmp);

    // Create input string
    std::string strInput = "std;0;";
    std::string strInputFixed = "";
    if (!strFileInput.empty() && !strIPInput.empty()) {
        strInput = "std;0;ASTERIX_RAW";
    }
    if (!strFileInput.empty()) {
        strInput = "disk;" + strFileInput + "|0|";

        if (bLoopFile) {
            strInput += "65;";
        } else {
            strInput += "1;";
        }
    } else if (!strIPInput.empty()) {
/*
		// count ':'
		int cntr=0;
		int indx=0;
		while((indx=strIPInput.find(':', indx+1)) >= 0)
		{
			cntr++;
		}

		if (cntr == 2)
			strInput = "udp " + strIPInput + "::S ";
		else if (cntr == 3)
			strInput = "udp " + strIPInput + ":S ";
		else {
			std::cerr << "Error: Wrong input address format  (shall be: mcastaddress:ipaddress:port[:srcaddress]" << std::endl;
			exit (3);
		}
*/
        strInput = "udp;" + strIPInput + ";"; // + ":S ";
    }

    strInput += strInputFormat;

    // Create output string
    std::string strOutput = "std 0 " + strOutputFormat;

    const char *inputChannel = NULL;
    const char *outputChannel[CChannelFactory::MAX_OUTPUT_CHANNELS];
    unsigned int chFailover = 0;
    unsigned int nOutput = 1; // Total number of output channels

    inputChannel = strInput.c_str();
    outputChannel[0] = strOutput.c_str();

    // Print out options
    LOGDEBUG(inputChannel, "Input channel description: %s\n", inputChannel);

    for (unsigned int i = 0; i < nOutput; i++) {
        LOGDEBUG(outputChannel[i], "Output channel %d description: %s\n", i + 1, outputChannel[i]);
    }

    gHeartbeat = abs(gHeartbeat); // ignore negative values
    //    LOGDEBUG(1, "Heart-beat: %d\n", gHeartbeat);

    // Finally execute converter engine
    if (CConverterEngine::Instance()->Initialize(inputChannel, outputChannel, nOutput, chFailover)) {
        if (strFilterFile.empty() == false) { // read filter file and configure items
            CBaseFormatDescriptor *desc = CChannelFactory::Instance()->GetInputChannel()->GetFormatDescriptor();
            if (desc == NULL) {
                std::cerr << "Error: Format description not found." << std::endl;
                exit(2);
            }

            FILE *ff = fopen(strFilterFile.c_str(), "r");
            if (ff == NULL) {
                std::cerr << "Error: Filter file " + strFilterFile + " not found." << std::endl;
                exit(2);
            }
            char line[1025];
            while (fgets(line, 1024, ff)) {

                int cat = 0;
                char item[129] = "";
                char name[129] = "";

                // skip commented lines
                if (line[0] == '#' || strlen(line) <= 0)
                    continue;

                char *p = strtok(line, ":");
                int ret = 0;
                while (p) {
                    if (sscanf(p, "CAT%03d", &cat) != 1)
                        break;
                    p = strtok(NULL, ":");
                    if (NULL == p) {
                        std::cerr <<
                                  "Warning: Wrong Filter format. Shall be: \"CATxxx:Ixxx:NAME  DESCRIPTION\" or start with \"#\" for comment . It is: " +
                                  std::string(line) << std::endl;
                        exit(3);
                    }
                    if (sscanf(p, "I%128s", item) != 1)
                        break;
                    p = strtok(NULL, "");
                    if (NULL == p) {
                        std::cerr <<
                                  "Warning: Wrong Filter format. Shall be: \"CATxxx:Ixxx:NAME  DESCRIPTION\" or start with \"#\" for comment . It is: " +
                                  std::string(line) << std::endl;
                        exit(3);
                    }
                    if (sscanf(p, "%128s", name) != 1)
                        break;
                    ret = 1;
                    break;
                }

                if (ret == 0) {
                    std::cerr <<
                              "Warning: Wrong Filter format. Shall be: \"CATxxx:Ixxx:NAME  DESCRIPTION\" or start with \"#\" for comment . It is: " +
                              std::string(line) << std::endl;
                    exit(3);
                }

                if (!desc->filterOutItem(cat, std::string(item), name)) {
                    std::cerr << "Warning: Filtering item not found: " + std::string(line) + ":" + std::string(item) +
                                 ":" + std::string(name) << std::endl;
                }
            }
            fclose(ff);
        }

        if (bListDefinitions) { // Parse definitions file and print all items
            CBaseFormatDescriptor *desc = CChannelFactory::Instance()->GetInputChannel()->GetFormatDescriptor();
            if (desc == NULL) {
                std::cerr << "Error: Format description not found." << std::endl;
                exit(2);
            }
            std::cout << desc->printDescriptor();
        } else {
            CConverterEngine::Instance()->Start();
        }
    } else {
        LOGERROR(1, "Couldn't initialize asterix engine.\n");
        exit(1);
    }

    CChannelFactory::DeleteInstance();
    CConverterEngine::DeleteInstance();
    CDeviceFactory::DeleteInstance();
    Tracer::Delete();
}


