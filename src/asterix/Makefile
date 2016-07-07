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
# AUTHORS:  Damir Salantic, Croatia Control Ltd.
# 			 Jurica Baricevic, Croatia Control Ltd.
#			 Zeljko Harjac, Croatia Control Ltd.
#			 Darko Markus, Croatia Control Ltd.
#
#

# TOP and CUR_DIR must be defined for each makefile
TOP             := ../..
TOP_SED 		:= ..\/..
CUR_DIR         := asterix
include $(TOP)/src/makefile.include

# Define the library to be made
TARGET_LIB = $(OBJ_DIR)/libasterix.a

OBJS = AsterixData.o \
	AsterixDefinition.o \
	Category.o \
	DataBlock.o \
	DataItem.o \
	DataItemBits.o \
	DataItemDescription.o \
	DataItemFormat.o \
	DataItemFormatCompound.o \
	DataItemFormatExplicit.o \
	DataItemFormatFixed.o \
	DataItemFormatRepetitive.o \
	DataItemFormatVariable.o \
	DataItemFormatBDS.o \
	DataRecord.o \
	InputParser.o \
	Tracer.o \
	UAP.o \
	UAPItem.o \
	Utils.o \
	XMLParser.o \
	asterixformat.o \
	asterixrawsubformat.o \
	asterixpcapsubformat.o \
	asterixfinalsubformat.o \
	asterixhdlcsubformat.o \
	asterixhdlcparsing.o
	
OBJS_WITH_PATH:=$(patsubst %.o,$(OBJ_DIR)/%.o,$(OBJS))


# Standard targets
all: $(TARGET_LIB)
	$(shell	if [ ! -d $(OBJ_DIR) ];	then mkdir -p $(OBJ_DIR); fi;)

debug: $(TARGET_LIB)
        $(shell if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;)

install: all
	$(shell	if [ ! -d $(OBJ_DIR) ];	then mkdir -p $(OBJ_DIR); fi;)

clean:
	rm -f $(TARGET_LIB) $(OBJS_WITH_PATH) $(DEPEND_FILES)
	@if [ -d $(OBJ_DIR) ]; then rmdir $(OBJ_DIR); fi;


include $(TOP)/src/makefile.rules
