/* packet-asterix.c
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

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <time.h>
#include <string.h>
#include <glib.h>

#include <epan/packet.h>
#include <epan/prefs.h>
#include <epan/expert.h>
#include <epan/range.h>

#include "WiresharkWrapper.h"

#include <stdlib.h>

#define ASTERIX_DEFAULT_UDP_RANGE "24,1024-1080"
#define ASTERIX_DEFAULT_TCP_RANGE "24,1024-1080"

/* The handle for the dynamic dissector */
static dissector_handle_t asterix_handle = NULL;

/* Forward declaration we need below */
static void dissect_asterix_pdu(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree[]);
static void dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree);

/* Global preferences */
static gboolean asterix_disabled = FALSE;
static gboolean oradis = FALSE;
static gboolean global_asterix_enable_log = FALSE;
static range_t *global_asterix_udp_port_range;
static range_t *global_asterix_tcp_port_range;
static range_t *asterix_udp_port_range;
static range_t *asterix_tcp_port_range;

#ifdef LINUX
static const char* asterix_description_path = "/usr/share/ethereal/asterix";
#else
static const char* asterix_description_path = "C:\\Program Files\\Wireshark\\plugins";
#endif

void proto_reg_handoff_asterix(void);

/* Initialize the protocol and registered fields */
static hf_register_info* hf = NULL;
static int maxpid = 0;
static int* asterix_description_pid = NULL;

/* Initialize the protocol and registered fields */
static int proto_asterix = -1;

/* Initialize the subtree pointers */
#define MAX_TREE_DEPTH 5
static gint ett_asterix[MAX_TREE_DEPTH] = { -1, -1, -1, -1, -1 };

static void logTraceFunc(char const* format, ...) {
	FILE* fp = fopen("asterix.log", "at");
	if (fp) {
		va_list args;
		va_start(args, format);
		vfprintf(fp, format, args);
		va_end(args);
		fclose(fp);
		g_print("Opened asterix.log file\n");
	} else {
		g_print("Unable to open asterix.log file\n");
	}
	g_print(format);
} /* logTraceFunc */

static void dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree) {
	/* Set up structures needed to add the protocol subtree and manage it */
	proto_tree* asterix_tree[5];
	proto_item* t_head_item = NULL;
	int tree_depth = 0;

	guint32 offset = 0;
	guint32 ocp_frame_size = 0;
	guint8 category;
	guint32 payload_len;
	char tmpstr[256];
	char tmp2[64];

	/* Asterix dissector not needed */
	if (asterix_disabled)
		return;

	/* Make entries in Protocol column and Info column on summary display */
	if (oradis) {
		if (check_col(pinfo->cinfo, COL_PROTOCOL))
			col_set_str(pinfo->cinfo, COL_PROTOCOL, "Asterix (OCP)");
	} else {
		if (check_col(pinfo->cinfo, COL_PROTOCOL))
			col_set_str(pinfo->cinfo, COL_PROTOCOL, "Asterix");
	}

	tmpstr[0] = 0;
	while (offset < tvb_length(tvb) && strlen(tmpstr)<200) {
		if (oradis)
			offset += 6;

		category = tvb_get_guint8(tvb, offset);
		offset += 1;

		payload_len = tvb_get_ntohs(tvb,offset);
		offset += 2;
		offset += payload_len-3;
		sprintf(tmp2, "CAT%03d (%d bytes), ", category, payload_len);
		strcat(tmpstr, tmp2);
	}

	if (offset != tvb_length(tvb))
		sprintf(tmp2, "Total= %d bytes - Wrong length !!!! ", offset);
	else
		sprintf(tmp2, "Total = %d bytes", offset);
	strcat(tmpstr, tmp2);

	if (check_col(pinfo->cinfo, COL_INFO))
		col_add_fstr(pinfo->cinfo, COL_INFO, tmpstr);

	if(!tree)
		return;

	t_head_item = proto_tree_add_protocol_format(tree, proto_asterix, tvb, 0, -1, "All Purpose Structured EUROCONTROL Surveillance Information Exchange (Asterix) Protocol");

	asterix_tree[tree_depth] = proto_item_add_subtree(t_head_item, ett_asterix[tree_depth]);

	offset = 0;
	if (oradis) {
		ocp_frame_size = tvb_length(tvb);	
		int asterix_bytes = 0;

		while (offset <= ocp_frame_size-1) {
			tvbuff_t *pdu_tvb;

			asterix_bytes = tvb_get_ntohs(tvb, offset)-6; // Asterix+Header length

			pdu_tvb = tvb_new_subset(tvb, offset+6, asterix_bytes, -1);

			// Decode PDU (without OCP header)
			dissect_asterix_pdu(pdu_tvb, pinfo, asterix_tree);
			offset += asterix_bytes+6;
		}

	} else {
		dissect_asterix_pdu(tvb, pinfo, asterix_tree);
	}
} /* dissect_asterix */

static void dissect_asterix_pdu(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree[]) {

	static fulliautomatix_data* pDataList = NULL;
	fulliautomatix_data* pData = NULL;
	int tree_depth = 0;
	int error = 0;

	proto_item* t_head_item = NULL;
	proto_item* t_item = NULL;

	pDataList = fulliautomatix_parse(tvb_get_ptr(tvb, 0, tvb_length(tvb)), tvb_length(tvb));

	pData = pDataList;
	while(pData) {
		if (pData->tree == 1) {
			t_item = proto_tree_add_text(tree[tree_depth], tvb, pData->bytenr, pData->length, pData->description);
			if (tree_depth < MAX_TREE_DEPTH-1)
				tree_depth++;
			tree[tree_depth] = proto_item_add_subtree(t_item, ett_asterix[tree_depth]);
		} else if (pData->tree == -1) {
			if (tree_depth > 0)
				  tree_depth--;
		} else {
			if (pData->type == FT_STRINGZ)
				t_item = proto_tree_add_string(tree[tree_depth], asterix_description_pid[pData->pid], tvb, pData->bytenr, pData->length, pData->val.str);
			else if (pData->type == FT_UINT32)
				t_item = proto_tree_add_uint(tree[tree_depth], asterix_description_pid[pData->pid], tvb, pData->bytenr, pData->length, pData->val.ul);
			else if (pData->type == FT_UINT32)
				t_item = proto_tree_add_uint(tree[tree_depth], asterix_description_pid[pData->pid], tvb, pData->bytenr, pData->length, pData->val.ul);
			else if (pData->type == FT_INT32)
				t_item = proto_tree_add_int(tree[tree_depth], asterix_description_pid[pData->pid], tvb, pData->bytenr, pData->length, pData->val.sl);
			else if (pData->type == FT_BYTES)
				t_item = proto_tree_add_bytes(tree[tree_depth], asterix_description_pid[pData->pid], tvb, pData->bytenr, pData->length, pData->val.str);
			else if (pData->type == FT_NONE)
				t_item = proto_tree_add_text(tree[tree_depth], tvb, pData->bytenr, pData->length, pData->val.str);
		}

		if (pData->value_description)
			proto_item_append_text(t_item, pData->value_description);

		if (pData->err == 1) {
			proto_item_set_expert_flags(t_item, PI_GROUP_MASK, PI_WARN);
			expert_add_info_format(pinfo, t_item, PI_UNDECODED, PI_WARN, "Warning in Asterix message");
			if (error == 0)
				error = 1;
		} else if (pData->err == 2) {
			proto_item_set_expert_flags(t_item, PI_REASSEMBLE, PI_ERROR);
			expert_add_info_format(pinfo, t_item, PI_MALFORMED, PI_ERROR, "Error in Asterix message");
			if (error < 2)
				error = 2;
		}
		pData = pData->next;
 	 }

	if (error == 1)
		proto_item_set_expert_flags(t_head_item, PI_GROUP_MASK, PI_WARN);
	else if (error == 2)
		proto_item_set_expert_flags(t_head_item, PI_REASSEMBLE, PI_ERROR);

	fulliautomatix_data_destroy(pDataList);

} /* dissect_asterix_pdu */

/* Register the protocol with Ethereal */
void proto_register_asterix(void) {
	module_t *asterix_prefs;

	proto_asterix = proto_register_protocol("Asterix", "Asterix", "asterix");

	asterix_prefs = prefs_register_protocol(proto_asterix, proto_reg_handoff_asterix);

	range_convert_str(&global_asterix_udp_port_range, ASTERIX_DEFAULT_UDP_RANGE, 65535);
	range_convert_str(&global_asterix_tcp_port_range, ASTERIX_DEFAULT_TCP_RANGE, 65535);

	/* Register preferences */
	prefs_register_bool_preference(asterix_prefs, "disable_asterix", "Disable ASTERIX",
		"Select this parameter to disable ASTERIX dissector: all packets are discarded.",
		&asterix_disabled);
	prefs_register_bool_preference(asterix_prefs, "oradis", "ORADIS Encapsulation",
		"Select this parameter to remove OCP header",
		&oradis);
	prefs_register_range_preference(asterix_prefs, "udp.asterix_ports", "Asterix listener UDP Ports",
		"Set the UDP ports for Asterix", 
		&global_asterix_udp_port_range, MAX_UDP_PORT);
	prefs_register_range_preference(asterix_prefs, "tcp.asterix_ports", "Asterix listener TCP Ports",
		"Set the TCP ports for Asterix",
		&global_asterix_tcp_port_range, MAX_TCP_PORT);
	prefs_register_bool_preference(asterix_prefs, "udp.log_to_file", "Log to file",
		"Enable logs from Asterix plugin",
		 &global_asterix_enable_log);
	prefs_register_string_preference(asterix_prefs, "description", "ASTERIX XML description path",
		"Name of the path for XML files which describe Asterix categories",
		&asterix_description_path);
} /* proto_register_asterix */

static void range_add_callback(guint32 port) {
	dissector_add("udp.port", port, asterix_handle);
	dissector_add("tcp.port", port, asterix_handle);
}

static void range_delete_callback(guint32 port) {
	dissector_delete("udp.port", port, asterix_handle);
	dissector_delete("tcp.port", port, asterix_handle);
}

/* The registration hand-off routine */
void proto_reg_handoff_asterix(void) {

	int len = 0;
	int ret;
	int i;

	/* Setup protocol subtree array */
	static gint *ett[] = {
		&ett_asterix[0],
		&ett_asterix[1],
		&ett_asterix[2],
		&ett_asterix[3],
		&ett_asterix[4]
		};

	static fulliautomatix_definitions* deflist = NULL;
	fulliautomatix_definitions* asterix_description = NULL;

	static gboolean asterix_prefs_initialized = FALSE;

	if (!asterix_prefs_initialized) {
		asterix_handle = create_dissector_handle(dissect_asterix, proto_asterix);
		asterix_prefs_initialized = TRUE;

	if (!global_asterix_enable_log)
		ret = fulliautomatix_start(g_print, asterix_description_path);
	else
		ret = fulliautomatix_start(logTraceFunc, asterix_description_path);

	if (ret != 0)
		logTraceFunc("Failed to initialize Asterix plugin (%d)\n", ret);

	if (deflist)
		fulliautomatix_destroy_definitions(deflist);

	/* Get XML definited category */
	deflist = asterix_description = fulliautomatix_get_definitions();

	len = 0;
	while (asterix_description) {
		if (asterix_description->pid > maxpid)
			maxpid = asterix_description->pid;
		len++;
		asterix_description = asterix_description->next;
	}

	asterix_description_pid = (int*)malloc(sizeof(int)*(maxpid+1));
	for(i=0; i<=maxpid; i++)
		asterix_description_pid[i] = -1;

	/* Setup list of header fields */
	hf = (hf_register_info*)malloc(sizeof(hf_register_info)*len);
	memset(hf, 0, sizeof(hf_register_info)*len);

	asterix_description = deflist;

	for (i=0; i<len && asterix_description; i++) {
		hf[i].p_id = &asterix_description_pid[asterix_description->pid];
		hf[i].hfinfo.name = asterix_description->name;
		hf[i].hfinfo.abbrev = asterix_description->abbrev;
		hf[i].hfinfo.type = asterix_description->type;
		hf[i].hfinfo.display = asterix_description->display;
		hf[i].hfinfo.strings = asterix_description->strings;
		hf[i].hfinfo.bitmask = asterix_description->bitmask;
		hf[i].hfinfo.blurb = asterix_description->blurb;
		asterix_description = asterix_description->next;
	}

	proto_register_field_array(proto_asterix, hf, len);

	proto_register_subtree_array(ett, array_length(ett));
	} else {
		range_foreach(asterix_udp_port_range, range_delete_callback);
		range_foreach(asterix_tcp_port_range, range_delete_callback);
	}

	g_free(asterix_udp_port_range);
	asterix_udp_port_range = range_copy(global_asterix_udp_port_range);
	range_foreach(asterix_udp_port_range, range_add_callback);

	g_free(asterix_tcp_port_range);
	asterix_tcp_port_range = range_copy(global_asterix_tcp_port_range);
	range_foreach(asterix_tcp_port_range, range_add_callback);
} /* proto_reg_handoff_asterix */
