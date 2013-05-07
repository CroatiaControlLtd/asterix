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
#ifndef ASTERIXRACKSSUBFORMAT_HXX__
#define ASTERIXRACKSSUBFORMAT_HXX__
/**
 *
 * @brief Asterix tracks sub-format processor. This is number of tracks read from Asterix CAT62.
 *
 */

class CBaseDevice;

// Define which category is used for tracks counter
#define ASTERIX_CATEGORY	30
//#define ASTERIX_CATEGORY	62

#if (ASTERIX_CATEGORY==30)
	// Maximal number of track IDs
	#define MAX_NUMBER_OF_TRACKS	4096
#elif (ASTERIX_CATEGORY==62)
// Maximal number of track IDs
#define MAX_NUMBER_OF_TRACKS	32768
#else
#error "Category not defined"
#endif

// Number of tracks after which error is printed
#define TRACKS_OVERLOAD_MARK    747
// test #define TRACKS_OVERLOAD_MARK	82

// Number of track must fall below this line to print new error
#define TRACKS_OVERLOAD_END     700
// test #define TRACKS_OVERLOAD_END	81

// Number of tracks after which error is printed
#define TRACKS_PRE_OVERLOAD_MARK    655

// Number of track must fall below this line to print new error
#define TRACKS_PRE_OVERLOAD_END     600



/**
 * @class CAsterixTracksSubformat
 *
 * @brief Asterix tracks sub-format
 *
 * Specifies format of Asterix message.
 *
 */
class CAsterixTracksSubformat
{
public:

    static bool WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard);
    static bool Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device);
private:

	static unsigned char m_pTracks[MAX_NUMBER_OF_TRACKS/8]; // Table with 1 bit for each track ID
	static time_t m_nTimeOfMaximum; // Timestamp of todays maximum
	static int m_nMaxNumberOfTracks; // Maximal number of tracks today
	static int m_nCurrentDay; // Todays date
	static int m_bOverloadWarningRaised; // 1 if overload Warning is raised
	static int m_bOverloadErrorRaised; // 1 if overload Error is raised
};

#endif
