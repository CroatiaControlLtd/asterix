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
#ifndef ASTERIXTXTSUBFORMAT_HXX__
#define ASTERIXTXTSUBFORMAT_HXX__
/**
 * @file asterixtxtsubformat.hxx
 *
 * @brief Asterix xt file sub-format processor. This is description of Asterix format in plain human-readable text.
 *
 */

class CBaseDevice;
class CAsterixFormatDescriptor;
class InputParser;


/**
 * @class CAsterixTxtSubformat
 *
 * @brief Asterix txt sub-format
 *
 * Specifies format of Asterix message.
 *
 */
class CAsterixTxtSubformat
{
public:

    static bool WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard);

private:

};

#endif
