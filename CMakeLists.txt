#
#  Copyright (c) 2013 Croatia Control Ltd. (www.crocontrol.hr)
#
#  This file is part of Asterix.
#
#  Asterix is free software: you can redistribute it and/or modify
#  it under the terms of the GNU General Public License as published by
#  the Free Software Foundation, either version 3 of the License, or
#  (at your option) any later version.
#
#  Asterix is distributed in the hope that it will be useful,
#  but WITHOUT ANY WARRANTY; without even the implied warranty of
#  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
#  GNU General Public License for more details.
#
#  You should have received a copy of the GNU General Public License
#  along with Asterix.  If not, see <http://www.gnu.org/licenses/>.
#
#
# AUTHOR:  Damir Salantic, Croatia Control Ltd.

cmake_minimum_required(VERSION 2.8.4)

project(asterix)

set(EXECUTABLE_OUTPUT_PATH install)

set(CMAKE_CXX_STANDARD 11)

#set (CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")
#set (CMAKE_LINKER_FLAGS_DEBUG "${CMAKE_LINKER_FLAGS_DEBUG} -fno-omit-frame-pointer -fsanitize=address")

include_directories(src/asterix)
include_directories(src/engine)
include_directories(src/main)

include(FindEXPAT)
find_package(EXPAT REQUIRED)
include_directories(${EXPAT_INCLUDE_DIRS})

add_executable(${PROJECT_NAME}
        src/asterix/AsterixData.cpp
        src/asterix/AsterixData.h
        src/asterix/AsterixDefinition.cpp
        src/asterix/AsterixDefinition.h
        src/asterix/asterixfinalsubformat.cxx
        src/asterix/asterixfinalsubformat.hxx
        src/asterix/asterixformat.cxx
        src/asterix/asterixformat.hxx
        src/asterix/asterixformatdescriptor.hxx
        src/asterix/asterixgpssubformat.cxx
        src/asterix/asterixgpssubformat.hxx
        src/asterix/asterixhdlcparsing.c
        src/asterix/asterixhdlcparsing.h
        src/asterix/asterixhdlcsubformat.cxx
        src/asterix/asterixhdlcsubformat.hxx
        src/asterix/asterixpcapsubformat.cxx
        src/asterix/asterixpcapsubformat.hxx
        src/asterix/asterixrawsubformat.cxx
        src/asterix/asterixrawsubformat.hxx
        src/asterix/Category.cpp
        src/asterix/Category.h
        src/asterix/DataBlock.cpp
        src/asterix/DataBlock.h
        src/asterix/DataItem.cpp
        src/asterix/DataItem.h
        src/asterix/DataItemBits.cpp
        src/asterix/DataItemBits.h
        src/asterix/DataItemDescription.cpp
        src/asterix/DataItemDescription.h
        src/asterix/DataItemFormat.cpp
        src/asterix/DataItemFormat.h
        src/asterix/DataItemFormatBDS.cpp
        src/asterix/DataItemFormatBDS.h
        src/asterix/DataItemFormatCompound.cpp
        src/asterix/DataItemFormatCompound.h
        src/asterix/DataItemFormatExplicit.cpp
        src/asterix/DataItemFormatExplicit.h
        src/asterix/DataItemFormatFixed.cpp
        src/asterix/DataItemFormatFixed.h
        src/asterix/DataItemFormatRepetitive.cpp
        src/asterix/DataItemFormatRepetitive.h
        src/asterix/DataItemFormatVariable.cpp
        src/asterix/DataItemFormatVariable.h
        src/asterix/DataRecord.cpp
        src/asterix/DataRecord.h
        src/asterix/InputParser.cpp
        src/asterix/InputParser.h
        src/asterix/Tracer.cpp
        src/asterix/Tracer.h
        src/asterix/UAP.cpp
        src/asterix/UAP.h
        src/asterix/UAPItem.cpp
        src/asterix/UAPItem.h
        src/asterix/Utils.cpp
        src/asterix/Utils.h
        src/asterix/XMLParser.cpp
        src/asterix/XMLParser.h
        src/engine/basedevice.hxx
        src/engine/baseformat.hxx
        src/engine/baseformatdescriptor.hxx
        src/engine/basepacket.hxx
        src/engine/channel.hxx
        src/engine/channelfactory.cxx
        src/engine/channelfactory.hxx
        src/engine/converterengine.cxx
        src/engine/converterengine.hxx
        src/engine/descriptor.cxx
        src/engine/descriptor.hxx
        src/engine/devicefactory.cxx
        src/engine/devicefactory.hxx
        src/engine/diskdevice.cxx
        src/engine/diskdevice.hxx
        src/engine/serialdevice.cxx
        src/engine/serialdevice.hxx
        src/engine/singleton.hxx
        src/engine/stddevice.cxx
        src/engine/stddevice.hxx
        src/engine/tcpdevice.cxx
        src/engine/tcpdevice.hxx
        src/engine/udpdevice.cxx
        src/engine/udpdevice.hxx
        src/main/asterix.cpp
        src/main/asterix.h
        src/main/version.h)

target_link_libraries(${PROJECT_NAME} ${EXPAT_LIBRARIES})
