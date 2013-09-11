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

#ifndef ASTERIXDATA_H_
#define ASTERIXDATA_H_

#include <map>

#include "AsterixDefinition.h"
#include "DataBlock.h"

class AsterixData
{
public:
  AsterixData();
  virtual
  ~AsterixData();

  std::list<DataBlock*> m_lDataBlocks;

#if defined(WIRESHARK_WRAPPER) || defined(ETHEREAL_WRAPPER)
  fulliautomatix_data* getData();
#endif

  bool getText(std::string& strDescription, std::string& header); // appends text to strDescription
  bool getDescription(std::string& strDescription); // appends description to strDescription
  bool getXIDEF(std::string& strXIDEF); // appends XIDEF descriptin to strXIDEF
  bool printOutKMLTimePlots(); // Print out data KML file with plots with timestamp
  bool printOutFiltered(); // Print out filtered data
};

#endif /* ASTERIXDATA_H_ */
