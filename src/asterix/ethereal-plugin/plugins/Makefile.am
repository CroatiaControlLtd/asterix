# Makefile.am
# Automake file for Ethereal
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
# Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.

if HAVE_LIBLUA
lua_dir = \
	lua
else		# HAVE_LIBLUA
lua_dir =
endif		# HAVE_LIBLUA

SUBDIRS = \
	acn \
	agentx \
	artnet \
	asn1 \
	asterix \
	ciscosm \
	docsis \
	enttec \
	giop \
	gryphon \
	h223 \
	irda \
	$(lua_dir) \
	lwres \
	mate \
	megaco \
	mgcp \
	opsi \
	pcli \
	profinet \
	rdm \
	rlm \
	rtnet \
	rudp \
	stats_tree \
	v5ua


plugindir = @plugindir@


EXTRA_DIST = \
	Makefile.nmake

MAINTAINERCLEANFILES = \
	Makefile.in

