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

#include "../main/asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixtrackssubformat.hxx"

#include "AsterixDefinition.h"

unsigned char CAsterixTracksSubformat::m_pTracks[MAX_NUMBER_OF_TRACKS/8] = { 0 };
time_t CAsterixTracksSubformat::m_nTimeOfMaximum = 0;
int CAsterixTracksSubformat::m_nMaxNumberOfTracks = 0;
int CAsterixTracksSubformat::m_nCurrentDay = 0;
int CAsterixTracksSubformat::m_bOverloadWarningRaised = 0;
int CAsterixTracksSubformat::m_bOverloadErrorRaised = 0;

/*
 * Extracts track ID from Asterix CAT62 and populates table.
 * Table consists of one bit for each track ID with following meaning:
 * 1 - track ID occurred since last table reset
 * 0 - track ID did not occur since last table reset
 *
 */
bool CAsterixTracksSubformat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard)
{
  CAsterixFormatDescriptor& Descriptor((CAsterixFormatDescriptor&)formatDescriptor);

  if (Descriptor.m_pAsterixData == NULL)
  {
    LOGERROR(1, "Asterix data packet not present\n");
    return false;
  }

  // go through all data
  std::list<DataBlock*>::iterator itr;
  for ( itr=Descriptor.m_pAsterixData->m_lDataBlocks.begin() ; itr != Descriptor.m_pAsterixData->m_lDataBlocks.end(); itr++ )
  {
    DataBlock* db = (DataBlock*)(*itr);
    if (db != NULL)
    {
      std::list<DataRecord*>::iterator it;
      for ( it=db->m_lDataRecords.begin() ; it != db->m_lDataRecords.end(); it++ )
      {
        DataRecord* dr = (DataRecord*)(*it);
        if (dr != NULL)
        { // parse only defined category
          if (dr->getCategory() == ASTERIX_CATEGORY)
          {
            unsigned long lTrackNumber = 0;

            if (dr->getValue(40, lTrackNumber, "track_number"))
            {
            	if (lTrackNumber < 0 || lTrackNumber >= MAX_NUMBER_OF_TRACKS)
            	{
            		Tracer::Error("Invalid track %d", lTrackNumber);
            	}
            	else
            	{
            	  unsigned char mask = 0x01;
            	  mask <<= (7-(lTrackNumber%8));
				  m_pTracks[lTrackNumber/8] |= mask;
            	}
            }
          }
        }
      }
    }
  }
  return true;
}

/*
 * Calculate number of tracks that occurred in table.
 * Clean the table.
 * Print time and number of tracks.
 */
bool CAsterixTracksSubformat::Heartbeat(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device)
{
	// calculate number of tracks
	int numberOfTracks = 0;
	for (int i=0; i<MAX_NUMBER_OF_TRACKS/8; i++)
	{
	  unsigned char mask = 0x01;
	  do
	  {
		  if (m_pTracks[i] & mask)
		  {
			  numberOfTracks++;
		  }
		  mask <<= 1;
	  }
	  while(mask);
	}

	// clean table
	memset(m_pTracks, 0, sizeof(m_pTracks));

	// get current time
	char strDescription[128];
	time_t now = time(NULL);
	tm * ptm = gmtime ( &now );
	int mday = ptm->tm_mday;
	int mon = ptm->tm_mon+1;
	int year = ptm->tm_year+1900;
	int hour =ptm->tm_hour;
	int min = ptm->tm_min;
	int sec = ptm->tm_sec;

	// if this is first packet, set the file name
	static int firstPacket = 1;
	if (firstPacket)
	{
		firstPacket = 0;
		int dateCount = year*10000+mon*100+mday;
                device.IoCtrl(CBaseDevice::EPacketDone, &dateCount, sizeof(dateCount));
	}

	if (mday != m_nCurrentDay)
	{ // day changed
		if (m_nCurrentDay != 0)
		{
			// print daily statistics
			tm * yesterday = gmtime ( &m_nTimeOfMaximum );
			int yestmday = yesterday->tm_mday;
			int yestmon = yesterday->tm_mon+1;
			int yestyear = yesterday->tm_year+1900;
			int yesthour =yesterday->tm_hour;
			int yestmin = yesterday->tm_min;
			int yestsec = yesterday->tm_sec;
			LOGNOTIFY(1, "Daily tracks maximum was %d on %02d.%02d.%02d.%02d:%02d:%02d\n", m_nMaxNumberOfTracks, yestmday, yestmon, yestyear, yesthour, yestmin, yestsec);
		}
		// reset counter
		m_nCurrentDay = ptm->tm_mday;
		m_nMaxNumberOfTracks = numberOfTracks;
		m_nTimeOfMaximum = now;

		// open new output file
		int dateCount = year*10000+mon*100+mday;
                device.IoCtrl(CBaseDevice::EPacketDone, &dateCount, sizeof(dateCount));
	}

        // check if Error must be raised
        if (m_bOverloadErrorRaised == 0 && numberOfTracks >= TRACKS_OVERLOAD_MARK)
        {
                LOGERROR(1, "Track overload !!! Number of tracks larger than %d\n", TRACKS_OVERLOAD_MARK);
                m_bOverloadErrorRaised = 1;
        }
        // check if Warning must be raised
        else if (m_bOverloadWarningRaised == 0 && numberOfTracks >= TRACKS_PRE_OVERLOAD_MARK)
        {
                LOGWARNING(1, "Track pre-overload ! Number of tracks larger than %d\n", TRACKS_PRE_OVERLOAD_MARK);
                m_bOverloadWarningRaised = 1;
        }
        // check if Error must be lowered
        if (m_bOverloadErrorRaised == 1 && numberOfTracks <= TRACKS_OVERLOAD_END)
        {
                LOGNOTIFY(1, "Track overload ended. Number of tracks less than %d. Maximum was %d\n", TRACKS_OVERLOAD_END, m_nMaxNumberOfTracks);
                m_bOverloadErrorRaised = 0;
        }
        // check if Warning must be lowered
	if (m_bOverloadWarningRaised == 1 && numberOfTracks <= TRACKS_PRE_OVERLOAD_END)
	{
                LOGNOTIFY(1, "Track pre-overload ended. Number of tracks less than %d. Maximum was %d\n", TRACKS_PRE_OVERLOAD_END, m_nMaxNumberOfTracks);
		m_bOverloadWarningRaised = 0;
	}

	// remember daily maximum
	if (numberOfTracks > m_nMaxNumberOfTracks)
	{
		m_nMaxNumberOfTracks = numberOfTracks;
		m_nTimeOfMaximum = now;
	}

	// print out result
	sprintf(strDescription, "%d\t%d\t%02d.%02d.%02d.\t%02d:%02d:%02d\n", numberOfTracks, m_nMaxNumberOfTracks, mday, mon, year, hour, min, sec);
	device.Write(strDescription, strlen(strDescription));

	return true;
}

