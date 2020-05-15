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
#ifndef BASEFORMATDESCRIPTOR_HXX__
#define BASEFORMATDESCRIPTOR_HXX__

#include <string>

/**
 * @class CBaseFormatDescriptor
 * 
 * @brief Virtual class that stores fomat parameters for each channel set at 
 *  the initialization stage. 
 *  Each formatter needs to define its own descriptor storing data specific
 *  to the format.
 *  Descriptor is kept within channel and forwarded to formatter whenever 
 *  method requires specific parameters.
 *        
 */
class CBaseFormatDescriptor {
private:

public:
    /**
     * Empty constructor
     */
    CBaseFormatDescriptor() {}

    /**
     * Pure virtual destructor.
     */
    virtual ~CBaseFormatDescriptor() {}

    /**
     * Print format description
     */
    virtual std::string printDescriptor() { return "Not available"; }

    virtual bool filterOutItem(int cat, std::string item, const char *name) { return false; }

};

#endif
