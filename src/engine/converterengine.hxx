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
#ifndef CONVERTERENGINE_HXX__
#define CONVERTERENGINE_HXX__

#include "singleton.hxx"

/**
 * @class CConverterEngine
 * 
 * @brief The main class responsible for both initialization and
 *        execution of all other modules.
 *
 * @see   <CSingleton>
 */
class CConverterEngine {
private:

    // Singleton pattern
    static CSingleton<CConverterEngine> _Instance;

    // To access private Ctor...
    friend class CSingleton<CConverterEngine>;

public:

    /**
     * Returns the pointer to the only instance of this engine
     * (Singleton pattern). If the instance does not yet
     * exist (on a first call), the instance is created 
     *
     * @return Pointer to the only instance of the engine.
     *
     * @see <CSingleton>::<instance>
     */
    static CConverterEngine *Instance() { return _Instance.Instance(); }

    /**
     * Deletes the one and only instance of the engine
     * (Singleton pattern).
     *
     * @see <CSingleton>::<deleteInstance>
     */
    static void DeleteInstance() { _Instance.DeleteInstance(); }

    /**
    * Initializes the engine.
    *
    * @param inputChannel
    * String description of input channel in the format
    * <device_type> <device_descriptor> <data_format>
    *
    * @param outputChannel
    * Array of string descriptions of output channels.
    *
    * @return <true> on success, <false> otherwise
    *
    * @see <CConverterEngine>::<Start>
    */
    bool Initialize(const char *inputChannel, const char *outputChannel[], const unsigned int nOutput,
                    const unsigned int chFailover);

    /**
     * Starts the engine. Never returns. Must not be called if initialization
     * is not properly finished.
     *
     * @see <CConverterEngine>::<Initialize>
     */
    void Start();

    int ProcessStatus();
};

#endif
