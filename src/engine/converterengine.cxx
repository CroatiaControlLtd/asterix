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
#include <stdio.h>
#include <string.h>

#include "asterix.h"
#include "converterengine.hxx"
#include "channelfactory.hxx"
#include "descriptor.hxx"

CSingleton<CConverterEngine> CConverterEngine::_Instance;


bool CConverterEngine::Initialize(const char *inputChannel, const char *outputChannel[], const unsigned int nOutput,
                                  const unsigned int chFailover) {

    ASSERT(inputChannel);

    CDescriptor inputDescriptor(inputChannel, ";");
    const char *inputDevice = inputDescriptor.GetFirst();
    const char *inputDeviceDescriptor = inputDescriptor.GetNext();
    const char *inputFormat = inputDescriptor.GetNext();
    const char *inputFormatDescriptor = inputDescriptor.GetNext();

    LOGDEBUG(1, "inputDevice(%s)", inputDevice);
    LOGDEBUG(1, "inputDeviceDescriptor(%s)", inputDeviceDescriptor);
    LOGDEBUG(1, "inputFormatDescriptor(%s)", inputFormatDescriptor);

    // Check input channel parameters consistency
    if ((inputDevice == NULL) || (inputDeviceDescriptor == NULL) || (inputFormat == NULL)) {
        LOGERROR(1, "Input channel descriptor must be in the following format: \n\""
                    "<device> <device_descriptor> <format> [format_descriptor]\"\n");
        return false;
    }

    LOGDEBUG(1, "Input format: %s\n", inputFormat);

    // Create input channel
    if (!CChannelFactory::Instance()->CreateInputChannel(inputDevice, inputDeviceDescriptor, inputFormat,
                                                         inputFormatDescriptor)) {
        LOGERROR(1, "Input channel initialization failed.\n");
        return false;
    }

    // Process output channels
    for (unsigned int i = 0; i < nOutput; i++) {
        // TODO: special initialization of Failover channels

        ASSERT(outputChannel[i]);
        CDescriptor outputDescriptor(outputChannel[i], " ");
        const char *outputDevice = outputDescriptor.GetFirst();
        const char *outputDeviceDescriptor = outputDescriptor.GetNext();
        const char *outputFormat = outputDescriptor.GetNext();
        const char *outputFormatDescriptor = outputDescriptor.GetNext();
        const char *outputHeartbeat = outputDescriptor.GetNext();

        // Check output channel parameters consistency
        if ((outputDevice == NULL) || (outputDeviceDescriptor == NULL) || (outputFormat == NULL)) {
            LOGERROR(1, "Output channel descriptor must be in the following format: \n\""
                        "<device> <device_descriptor> <format> [format_descriptor] [heartbeat]\"\n");
            return false;
        }

        LOGDEBUG(1, "Output format: %s\n", outputFormat);

        // Create output channel
        if (!CChannelFactory::Instance()->CreateOutputChannel(outputDevice, outputDeviceDescriptor, outputFormat,
                                                              outputFormatDescriptor, i >= chFailover,
                                                              outputHeartbeat)) {
            LOGERROR(1, "Output channel initialization failed.\n");
            return false;
        }
    }

    return true;
}


// TODO:
// - call appropriate method (todo- write method) to analyze if the packet needs to be discarded
// - enable disconnection/reconnection with server (check CreateOutputChannel).
//
void CConverterEngine::Start() {
    unsigned int nChannels = CChannelFactory::Instance()->GetNOutputChannels();
    bool discard = false, packetOk = false, packetReceived = false, noMoreData = false;

    LOGNOTIFY(gVerbose, "Converter Engine Started.\n");

    while (true) {
        // 1. Wait for incomming packet on input channel
        do {
            packetReceived = CChannelFactory::Instance()->WaitForPacket(gHeartbeat);

            // 1.1 HeartbeatProcessing for all output channels is called *AT LEAST* every gHeartBeat
            for (unsigned int i = 0; i < nChannels; i++) {
                if (!CChannelFactory::Instance()->HeartbeatProcessing(i)) {
                    LOGERROR(1, "Heartbeat() failed.\n");
                }
            }
        } while (!packetReceived);

        // 1a Check if there is more data
        int rps = ProcessStatus();
        noMoreData = rps & STS_NO_DATA;
        if (noMoreData) {
            break;
        }

        // 2. Read the incoming packet
        if (!(packetOk = CChannelFactory::Instance()->ReadPacket())) {
            int rps = ProcessStatus();
            noMoreData = rps & STS_NO_DATA;
            if (noMoreData) {
                LOGINFO(1, "No more data available on input channel.\n");
                break;
            } else {
                // LOGERROR(1, "ReadPacket() failed.\n");

                if (rps & (STS_FAIL_INPUT | STS_FAIL_DATA))
                    continue;
            }
        }

        if (packetOk || noMoreData) {
            // 3. Process the packet
            if (packetOk && !noMoreData) {
                if (!CChannelFactory::Instance()->ProcessPacket(discard)) {
                    LOGERROR(1, "ProcessPacket() failed.\n");

                    if (ProcessStatus() & (STS_FAIL_INPUT | STS_FAIL_DATA))
                        continue;
                }
            }

            // Check if currently read packet needs to be discarded
            if ((gForceRouting) || (!discard)) {
                // 4. Dispatch the incoming packet to all normal output channels,
                //    and to a single failover output channel
                for (unsigned int i = 0; i < nChannels; i++) {
                    if (noMoreData) {
                        // notify output channels
                        if (!CChannelFactory::Instance()->IoCtrl((int) i, CBaseDevice::EAllDone)) {
                            LOGERROR(1, "IoCtrl() failed.\n");
                        }
                    } else if (packetOk) {
                        if (CChannelFactory::Instance()->IsFailoverOutputChannel(i))
                            break;

                        if (!CChannelFactory::Instance()->WritePacket(i)) {
                            LOGERROR(1, "WritePacket() failed.\n");
                        }
                    }
                }

                // handle failover output channels

                unsigned int ch = CChannelFactory::Instance()->GetActiveFailoverOutputChannel();
                unsigned int startCh = ch;

                while (ch < nChannels) {
                    // try writing to the active channel
                    // LOGDEBUG(1, "Trying to write to output channel %d...\n", (int)ch);

                    if (CChannelFactory::Instance()->WritePacket(ch))
                        break; // successfull!

                    // failed...
                    LOGERROR(1, "The current packet has been lost for failover output channel %d\n", (int) ch);

                    int sts = ProcessStatus();

                    if (sts & STS_FAIL_OUTPUT) {
                        ch = CChannelFactory::Instance()->GetNextFailoverOutputChannel();

                        LOGNOTIFY(gVerbose, "Switching to failover output channel: %d\n", (int) ch);

                        if (ch == startCh)
                            break; // we've cycled through all failover channels
                    }

                    if (sts & STS_FAIL_INPUT)
                        break; // input channel has been reset, so discard current packet and start over from reading
                }
            }
        }
    }
}


int CConverterEngine::ProcessStatus() {
    int sts = CChannelFactory::Instance()->GetStatus();

    if ((sts & STS_FAIL_INPUT) || (sts & STS_FAIL_DATA)) {
        LOGWARNING(1, "Resetting input channel due to excessive errors or data errors\n");
        if (!CChannelFactory::Instance()->ResetInputChannel()) {
            LOGERROR(1, "Failed to reset input channel.\n");
        }
    }

    unsigned int activeFOC = CChannelFactory::Instance()->GetActiveFailoverOutputChannel();

    if (sts & STS_FAIL_OUTPUT) {
        LOGWARNING(1, "Resetting output channel %d due to excessive errors\n", (int) activeFOC);
        if (!CChannelFactory::Instance()->ResetOutputChannel(activeFOC)) {
            LOGERROR(1, "Failed to reset output channel %d.\n", (int) activeFOC);
        }
    }

    return sts;
}
