/* packet-asterix.h
 * Routines for All Purpose Structured EUROCONTROL Surveillance Information Exchange (Asterix) Protocol disassembly
 * Copyright 2013, Nabil BENDAFI nabil@bendafi.fr
 *
 * Ethereal - Network traffic analyzer
 * By Gerald Combs <gerald@ethereal.com>
 * Copyright 1998 Gerald Combs
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __PACKET_ASTERIX_H__
# define __PACKET_ASTERIX_H__

#include <glib.h>

#ifdef __cplusplus
extern "C" {
#endif

extern void proto_register_asterix(void);
extern void proto_reg_handoff_asterix(void);

#ifdef __cplusplus
}
#endif

#endif /* __PACKET_ASTERIX_H__ */
