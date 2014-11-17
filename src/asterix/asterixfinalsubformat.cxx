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

#include "asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixfinalsubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"

bool CAsterixFinalSubformat::ReadPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard)
{
  struct sFinalRecordHeader finalRecordHeader;
  char padding[4];

  CAsterixFormatDescriptor& Descriptor((CAsterixFormatDescriptor&)formatDescriptor);

  // Read final record header
  if (!device.Read((void*)&finalRecordHeader, sizeof(finalRecordHeader)))
  {
      LOGERROR(1, "Couldn't read packet header.\n");
      return false;
  }

  unsigned long nTimestamp = (unsigned long) finalRecordHeader.m_nTimeMMSB << 16;
  nTimestamp |= (unsigned long) finalRecordHeader.m_nTimeMSB << 8;
  nTimestamp |= (unsigned long) finalRecordHeader.m_nTimeLSB;
  // convert timestamp to milliseconds (resolution is 10ms in final format)
  nTimestamp = nTimestamp * 10;

  unsigned int neededLen = finalRecordHeader.m_nByteCountMSB;
  neededLen <<= 8;
  neededLen |= finalRecordHeader.m_nByteCountLSB;

  neededLen -= sizeof(finalRecordHeader); // minus header
  neededLen -= sizeof(padding); // minus padding

  if (neededLen <= 0)
  {
    LOGERROR(1, "Wrong final record header.\n");
    return false;
  }

  const unsigned char* pBuffer = Descriptor.GetNewBuffer(neededLen);

  // Read packet
  if (!device.Read((void*)pBuffer, neededLen))
  {
      LOGERROR(1, "Couldn't read packet.\n");
      return false;
  }

  // Read padding bytes
  if (!device.Read((void*)padding, sizeof(padding)))
  {
      LOGERROR(1, "Couldn't read packet padding.\n");
      return false;
  }

  // parse packet
  if (Descriptor.m_pAsterixData)
    delete Descriptor.m_pAsterixData;

  Descriptor.m_pAsterixData = Descriptor.m_InputParser.parsePacket(pBuffer, neededLen, nTimestamp);

  return true;
}

bool CAsterixFinalSubformat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard)
{
  return false; //TODO
}

bool CAsterixFinalSubformat::ProcessPacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard)
{
  return true;
}

bool CAsterixFinalSubformat::Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device)
{
  return true;
}

