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

/**

 * @file asterixhdlcsubformat.cxx
 *
 * @brief Asterix HDLC sub-format processor.
 *        HDLC data is in following format:
 *        1 byte - 0x01 (addr)
 *        1 byte - 0x03 (ctrl)
 *        N bytes - Asterix packet
 *        2 bytes - FCS
 *
 */


#include <stdio.h>
#include <string.h>

#include "asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixhdlcsubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"

#include "asterixhdlcparsing.h"


/*
 * 1. Read all data from input (but not more than buffer size)
 * 2. Start from beginning of input buffer and find potential HDLC frame start (0x01,0x03)
 * 3. From potential frame start to MAX_FRAME, find potential frame end (calculating FCS)
 * 4. Extract Asterix packet and store it to end of parser buffer
 * 5. Move pointer to input buffer beginning to next position
 * 6. Repeat from point 2, until end of input buffer is found
 * 7. Move the rest of unparsed data to beginning of input buffer
 */

bool CAsterixHDLCSubformat::ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard) {
    CAsterixFormatDescriptor &Descriptor((CAsterixFormatDescriptor &) formatDescriptor);

    // read available data to read buffer
    size_t readSize = MAX_RXBUF;

    if (!device.Read((void *) RxBuf, &readSize)) {
        LOGERROR(1, "Couldn't read HDLC packet.\n");
        return false;
    }

    // get output buffer
    const unsigned char *pBuffer = Descriptor.GetNewBuffer(MAX_CBUF);
    int nOutPos = 0;

    // copy read buffer to circular buffer
    copy_to_cbuf(RxBuf, readSize);

    bool bufferDone = false;

    while (!bufferDone && nOutPos < MAX_CBUF - MAX_FRM) {
        int len = 0;
        unsigned char *ptrFrame = get_next_hdlc_frame(&len);

        if (ptrFrame) { // store packet in internal buffer
            memcpy((void *) &pBuffer[nOutPos], ptrFrame, len);
            nOutPos += len;
            Descriptor.SetDataLen(Descriptor.GetDataLen() + len);
        } else {
            bufferDone = true;
        }

        // print number of failed bytes to standard error output
        int failedBytes = GetAndResetFailedBytes();
        LOGERROR(failedBytes, "HDLC frame error. Ignored bytes = %d\n", failedBytes);
    }

    return true;
}

bool CAsterixHDLCSubformat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard) {
    return false; //TODO
}

/*
 * Parse packet read from input channel and stored to Descriptor.m_pBuffer
 */
bool CAsterixHDLCSubformat::ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard) {
    CAsterixFormatDescriptor &Descriptor((CAsterixFormatDescriptor &) formatDescriptor);

    if (Descriptor.GetDataLen() < 3) {
        return true;
    }

    // delete old data
    if (Descriptor.m_pAsterixData) {
        delete Descriptor.m_pAsterixData;
    }

    // parse packet
    Descriptor.m_pAsterixData = Descriptor.m_InputParser.parsePacket(Descriptor.GetBuffer(), Descriptor.GetDataLen());

    return true;
}

bool CAsterixHDLCSubformat::Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device) {
    // nothing to do
    return true;
}

/* convert textual hexadecimal file data to binary
 * example:
 *
 * "01 03 01 00 31"  -> {0x01,0x03,0x03,0x031}
 */
/*
int CAsterixHDLCSubformat::ConvertTxtHexToBin(unsigned char* array, int len)
{
  // read byte by byte and convert to binary
  int inPos = 0;
  int outPos = 0;
  int lastChar = array[inPos++];
  int binByte = 0;
  int digits = 0;

  while(inPos <= len)
  {
    int val = -1;

    if ((lastChar >= '0' && lastChar <= '9'))
    {
      val = lastChar - '0';
    }
    else if ((lastChar >= 'A' && lastChar <= 'F'))
    {
      val = lastChar - 'A' + 10;
    }
    else if ((lastChar >= 'a' && lastChar <= 'f'))
    {
      val = lastChar - 'a' + 10;
    }

    if (val >= 0)
    {
      if (digits == 0)
      {
        binByte = val * 16;
      }
      else if (digits == 1)
      {
        binByte += val;
      }
      digits++;

      if (digits == 2)
      {
        array[outPos++] = (unsigned char)binByte;
        digits = 0;
      }
    }
    else
    {
      digits = 0;
    }

    lastChar = array[inPos++];
  }

  return outPos;
}
*/
