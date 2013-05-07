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
#include <string>

#include "../main/asterix.h"
#include "asterixformat.hxx"
#include "asterixformatdescriptor.hxx"
#include "asterixkmlsubformat.hxx"

#include "AsterixDefinition.h"
#include "InputParser.h"
#include "Utils.h"

bool CAsterixKmlSubformat::WritePacket(CBaseFormatDescriptor &formatDescriptor, CBaseDevice &device, bool &discard)
{
  CAsterixFormatDescriptor& Descriptor((CAsterixFormatDescriptor&)formatDescriptor);

  if (Descriptor.m_pAsterixData == NULL)
  {
    LOGERROR(1, "Asterix data packet not present\n");
    return false;
  }

  std::string strPacketDescription;

  std::map<long,std::string> m_mapTracks;
  std::map<long,std::string> m_mapPlots;
  std::map<long,long> m_mapTrackFirstTimes;
  std::map<long,long> m_mapTrackLastTimes;
  int nYear=0, nMonth=0, nDay=0;

  // KML header
  strPacketDescription = ("<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<kml xmlns=\"http://www.opengis.net/kml/2.2\">\n");
  strPacketDescription += ("<Document><name>Asterix radar tracks</name>");
  strPacketDescription += ("<description>Radar tracks extracted from Asterix data.</description>\n");
  strPacketDescription += ("<Style id=\"yellowLineGreenPoly\">\n");
  strPacketDescription += ("  <LineStyle>\n");
  strPacketDescription += ("    <color>7f00ffff</color>\n");
  strPacketDescription += ("    <width>4</width>\n");
  strPacketDescription += ("  </LineStyle>\n");
  strPacketDescription += ("  <PolyStyle>\n");
  strPacketDescription += ("    <color>7f00ff00</color>\n");
  strPacketDescription += ("  </PolyStyle>\n");
  strPacketDescription += (" </Style>\n");
  strPacketDescription += ("<Style id=\"check-hide-children\"><ListStyle><listItemType>checkHideChildren</listItemType></ListStyle></Style><styleUrl>#check-hide-children</styleUrl>\n");

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
        {
          if (dr->getCategory() == 62)
          {
            unsigned long lTrackNumber = 0;
            long nLatitude;
            long nLongitude;
            std::string strAircraftIdentification;
            long nTimestamp = 0;

            if (dr->getValue(40, lTrackNumber, "track_number") &&
                dr->getValue(105, nLatitude, "Latitude") &&
                dr->getValue(105, nLongitude, "Longitude") &&
                dr->getValue(70, nTimestamp, "time_of_track"))
            {
              double dLongitude = nLongitude * 0.000021457672119140625;
              double dLatitude = nLatitude * 0.000021457672119140625;

              if (nYear == 0)
              {
                const time_t tt = (time_t)dr->m_nTimestamp;
                tm* ptm = gmtime ( &tt );
                nYear = 1900+ptm->tm_year;
                nMonth = ptm->tm_mon;
                nDay = ptm->tm_mday;
                // TODO - this will not work when day changes
              }

              // calculate timestamp
              nTimestamp /= 128; //seconds since midnight

              long nFlightLevel = 0;
              dr->getValue(136, nFlightLevel, "FL");

              // coordinates
              std::string strTmp(format("%.6lf,%.6lf,%.0f\n",dLongitude, dLatitude, (nFlightLevel*25)/3.28));
              m_mapTracks[lTrackNumber] += strTmp;

              // get heading
              long nHeading = 0;
              dr->getValue(180, nHeading, "Heading");
              // get speed
              long nSpeed = 0;
              dr->getValue(180, nSpeed, "Speed");

              std::string strCallSign;
              dr->getValue(390, strCallSign, "Callsign");

              std::string strDeparture;
              dr->getValue(390, strDeparture, "DEP");

              std::string strDestination;
              dr->getValue(390, strDestination, "DES");

              std::string strFlightInfo;

              strFlightInfo = format("<Style><IconStyle id=\"plane\"><heading>%.0f</heading><Icon><href>root://icons/palette-2.png</href><x>0</x><y>0</y><w>32</w><h>32</h></Icon></IconStyle></Style>",(float)nHeading*0.0055);

              strFlightInfo += "<name>"+strCallSign+"</name>";
              strFlightInfo += format("<description>Track %ld\nCallsign %s\nHeading: %.0f\nSpeed: %.0f kt\nAltitude: %.0f m\nFrom: %s\nTo: %s</description>",
                  lTrackNumber, strCallSign.c_str(), (float)nHeading*0.0055, (float)nSpeed*0.22,(nFlightLevel*25)/3.28, strDeparture.c_str(), strDestination.c_str());

              strFlightInfo += "<Point><altitudeMode>absolute</altitudeMode>";
              strFlightInfo += "<coordinates>";
              strFlightInfo += strTmp;
              strFlightInfo += "</coordinates>";
              strFlightInfo += "</Point></Placemark>\n";

              // get last timestamp for this track
              long nBeginTimeStamp = m_mapTrackLastTimes[lTrackNumber];

              // save timestamp as last
              m_mapTrackLastTimes[lTrackNumber] = nTimestamp+1;

              // if this is first timestamp for this track
              if (m_mapTrackFirstTimes[lTrackNumber] == 0)
              { // save as first timestamp
                m_mapTrackFirstTimes[lTrackNumber] = nTimestamp+1;
              }

              if (nBeginTimeStamp != 0)
              {
                long nBeginSeconds = nBeginTimeStamp%60;
                nBeginTimeStamp /= 60; // minutes since midnight
                long nBeginMinutes = nBeginTimeStamp%60;
                nBeginTimeStamp /= 60; // hours since midnight
                long nBeginHours = nBeginTimeStamp;

                long nEndSeconds = nTimestamp%60;
                nTimestamp /= 60; // minutes since midnight
                long nEndMinutes = nTimestamp%60;
                nTimestamp /= 60; // hours since midnight
                long nEndHours = nTimestamp;

                std::string strTimeStamp = format("<TimeSpan><begin>%04ld-%02ld-%02ldT%02ld:%02ld:%02ldZ</begin><end>%04ld-%02ld-%02ldT%02ld:%02ld:%02ldZ</end></TimeSpan>", nYear, nMonth, nDay, nBeginHours, nBeginMinutes, nBeginSeconds, nYear, nMonth, nDay, nEndHours, nEndMinutes, nEndSeconds);

                strFlightInfo = "<Placemark>" + strTimeStamp + strFlightInfo;
                strPacketDescription += (strFlightInfo.c_str());
              }
            }
          }
        }
      }
    }
  }

  // print tracks
  std::map<long,std::string>::iterator mapit;

  // show content:
  for ( mapit=m_mapTracks.begin() ; mapit != m_mapTracks.end(); mapit++ )
  {
    long lTrackNumber = (*mapit).first;

    strPacketDescription += ("<Placemark>\n");

    int nBeginTimeStamp = m_mapTrackFirstTimes[lTrackNumber];
    int nBeginSeconds = nBeginTimeStamp%60;
    nBeginTimeStamp /= 60; // minutes since midnight
    int nBeginMinutes = nBeginTimeStamp%60;
    nBeginTimeStamp /= 60; // hours since midnight
    int nBeginHours = nBeginTimeStamp;

    int nEndTimeStamp = m_mapTrackLastTimes[lTrackNumber];
    int nEndSeconds = nEndTimeStamp%60;
    nEndTimeStamp /= 60; // minutes since midnight
    int nEndMinutes = nEndTimeStamp%60;
    nEndTimeStamp /= 60; // hours since midnight
    int nEndHours = nEndTimeStamp;

    strPacketDescription += format("<TimeSpan><begin>%04d-%02d-%02dT%02d:%02d:%02dZ</begin><end>%04d-%02d-%02dT%02d:%02d:%02dZ</end></TimeSpan>\n", nYear, nMonth, nDay, nBeginHours, nBeginMinutes, nBeginSeconds, nYear, nMonth, nDay, nEndHours, nEndMinutes, nEndSeconds);
    strPacketDescription += format("<name>Track %ld</name>\n", lTrackNumber);
    strPacketDescription += ("<description></description>\n");
    strPacketDescription += ("<styleUrl>#yellowLineGreenPoly</styleUrl>\n");
    strPacketDescription += ("<LineString>\n");
    strPacketDescription += ("<extrude>1</extrude>\n");
    strPacketDescription += ("<tessellate>1</tessellate>\n");
    strPacketDescription += ("<altitudeMode>absolute</altitudeMode>\n");
    strPacketDescription += ("<coordinates>\n");
    strPacketDescription += (((*mapit).second).c_str());
    strPacketDescription += ("</coordinates>\n");
    strPacketDescription += ("</LineString>\n");
    strPacketDescription += ("</Placemark>\n");
  }

  // KML footer
  strPacketDescription += ("</Document></kml>\n");

  device.Write(strPacketDescription.c_str(), strPacketDescription.length());

  return true;
}


