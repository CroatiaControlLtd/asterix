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
CUR_DIR         := main
include $(TOP)/src/makefile.include

UNAME := $(shell uname)

ifeq ($(UNAME), Linux)
TARGET_EXE = $(OBJ_DIR)/asterix
TARGET_INSTALL = asterix
else
TARGET_EXE = $(OBJ_DIR)/asterix.exe
TARGET_INSTALL = asterix.exe
endif

# Libraries

LIBS = -lengine -lasterix -lexpat

OBJS = asterix.o

OBJS_WITH_PATH:=$(patsubst %.o,$(OBJ_DIR)/%.o,$(OBJS))

DEPEND_LIBS = $(TOP)/lib/$(OBJ_DIR_SHORT)/libengine.a
DEPEND_LIBS += $(TOP)/lib/$(OBJ_DIR_SHORT)/libasterix.a

all: $(TARGET_EXE)
	$(shell	if [ ! -d $(OBJ_DIR) ];	then mkdir -p $(OBJ_DIR); fi;)

debug: $(TARGET_EXE)
        $(shell if [ ! -d $(OBJ_DIR) ]; then mkdir -p $(OBJ_DIR); fi;)

install: all $(TARGET_INSTALL)
	$(shell	if [ ! -d $(OBJ_DIR) ];	then mkdir -p $(OBJ_DIR); fi;)

clean:
	rm -f $(TOP)/install/$(TARGET_INSTALL) \
	$(TARGET_EXE) \
	$(DEPEND_FILES) $(OBJS_WITH_PATH)
	@if [ -d $(OBJ_DIR) ]; then rmdir $(OBJ_DIR); fi;

$(TARGET_INSTALL): $(TARGET_EXE)
	$(shell \
	if [ ! -d $(TOP)/install ]; \
	then mkdir $(TOP)/install; fi;)
	cp $(TARGET_EXE) $(TOP)/install/$(TARGET_INSTALL)


include $(TOP)/src/makefile.rules
