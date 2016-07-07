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

include makefile.include

DIRS =  engine \
        asterix \
        main

all: makefile.include makefile.rules
	@for dir in $(DIRS); do\
		echo "=== making $$dir ===";\
                if $(MAKE) -r -C $$dir $@; then true; else exit 1; fi;\
	done

install: makefile.include makefile.rules
	@for dir in $(DIRS); do\
		echo "=== installing $$dir ===";\
                if $(MAKE) -r -C $$dir $@; then true; else exit 1; fi;\
	done

debug: makefile.include makefile.rules
	@for dir in $(DIRS); do\
                echo "=== making $$dir ===";\
                if $(MAKE) MODE=DEBUG -r -C $$dir $@; then true; else exit 1; fi;\
        done

#depend: $(DEPEND_FILES)
#        find .. -name "*.d" | xargs cat > .depend

clean:
	-@ rm -f ../lib/$(OBJ_DIR_SHORT)/*.a ../install/*.bin ../install/*.sre .depend
	@if [ -d ../lib/$(OBJ_DIR_SHORT) ]; then rmdir ../lib/$(OBJ_DIR_SHORT); fi;
	@for dir in $(DIRS); do\
		echo "=== cleaning $$dir ===";\
                if $(MAKE) -r -C $$dir $@; then true; else exit 1; fi;\
	done

test:
	cd ../install/test;./test.sh

