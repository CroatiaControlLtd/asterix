# Makefile.am
# Routines for All Purpose Structured EUROCONTROL Surveillance Information Exchange (Asterix) Protocol disassembly
# Copyright 2013, Nabil BENDAFI nabil@bendafi.fr
#   
# Ethereal - Network traffic analyzer
# By Gerald Combs <gerald@ethereal.com>
# Copyright 1998 Gerald Combs
# 
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License
# as published by the Free Software Foundation; either version 2
# of the License, or (at your option) any later version.
# 
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# 
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
#

INCLUDES = -I$(top_srcdir)

plugindir = @plugindir@

plugin_LTLIBRARIES = asterix.la
asterix_la_SOURCES = \
	AsterixDefinition.cpp \
	AsterixDefinition.h \
	AsterixData.cpp \
	AsterixData.h \
	Category.cpp \
	Category.h \
	DataBlock.cpp \
	DataBlock.h \
	DataRecord.cpp \
	DataRecord.h \
	DataItem.cpp \
	DataItem.h \
	DataItemBits.cpp \
	DataItemBits.h \
	DataItemDescription.cpp \
	DataItemDescription.h \
	DataItemFormat.cpp \
	DataItemFormat.h \
	DataItemFormatCompound.cpp \
	DataItemFormatCompound.h \
	DataItemFormatExplicit.cpp \
	DataItemFormatExplicit.h \
	DataItemFormatFixed.cpp \
	DataItemFormatFixed.h \
	DataItemFormatRepetitive.cpp \
	DataItemFormatRepetitive.h \
	DataItemFormatVariable.cpp \
	DataItemFormatVariable.h \
	InputParser.cpp \
	InputParser.h \
	Tracer.cpp \
	Tracer.h \
	UAP.cpp \
	UAP.h \
	UAPItem.cpp \
	UAPItem.h \
	Utils.cpp \
	Utils.h \
	XMLParser.cpp \
	XMLParser.h \
	WiresharkWrapper.cpp \
	WiresharkWrapper.h \
	packet-asterix.c \
	packet-asterix.h \
	plugin.c \
	moduleinfo.h

asterix_la_LDFLAGS = -module -avoid-version
asterix_la_LIBADD = @PLUGIN_LIBS@

AM_CPPFLAGS = -DWIRESHARK_WRAPPER -DETHEREAL_WRAPPER -DLINUX

# Libs must be cleared, or else libtool won't create a shared module.
# If your module needs to be linked against any particular libraries,
# add them here.
LIBS =

CLEANFILES = \
        asterix \
        *~

MAINTAINERCLEANFILES = \
        Makefile.in

EXTRA_DIST = \
        Makefile.nmake
