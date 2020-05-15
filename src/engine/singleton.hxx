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
#ifndef SINGLETON_HXX__
#define SINGLETON_HXX__

/**
 * A generic singleton pattern.
 */
template<class T>
class CSingleton {
public:

    /**
     * Class constructor.
     */
    CSingleton() : _Instance(0) {}


    /**
     * Gets the single and only instance of the class.
     * If the instance is never created it is instantiated
     * with this call.
     *
     * @return The only instance of this class
     */
    T *Instance() {
        if (_Instance == 0) {
            _Instance = new T();
        }
        return _Instance;
    }

    /**
     * Deletes the single and only instance of the class.
     */
    void DeleteInstance() {
        if (_Instance) {
            delete _Instance;
            _Instance = 0;
        }
    }

private:

    T *_Instance;

};

#endif // SINGLETON_HXX__

