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
 * @file asterixxidefsubformat.cxx
 *
 * @brief XML based Internal Data Exchange Format processor.
 */

#include <stdio.h>
#include <string.h>
#include <string>

#include "../main/asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixxidefsubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"
#include "Utils.h"

bool CAsterixXIDEFSubformat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard)
{
  CAsterixFormatDescriptor& Descriptor((CAsterixFormatDescriptor&)formatDescriptor);

  if (Descriptor.m_pAsterixData == NULL)
  {
    LOGERROR(1, "Asterix data packet not present\n");
    return true; //ds check this
//    return false;
  }

  std::string strPacketDescription;

  static bool firstTime = true;

  if (firstTime)
  {
    strPacketDescription = "<XIDEFSTART>";
    firstTime = false;
  }

  if (!Descriptor.m_pAsterixData->getXIDEF(strPacketDescription))
  {
    LOGERROR(1, "Failed to get XIDEF data packet description\n");
    return false;
  }

  device.Write(strPacketDescription.c_str(), strPacketDescription.length());

  return true;
}


