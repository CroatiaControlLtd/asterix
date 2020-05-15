/* packet-asterix.c
 * Routines for All Purpose Structured Eurocontrol Surveillance Information Exchange (Asterix) Protocol disassembly
 * RFC 2257
 *
 * $Id: packet-asterix.c,v 1.2 2012-01-13 14:20:10 cvs Exp $
 *
 * Copyright (c) 2005 by HKZP d.o.o. <damir.salantic@crocontrol.hr>
 *
 * Wireshark - Network traffic analyzer
 * By Gerald Combs <gerald@wireshark.org>
 * Copyright 1999 Gerald Combs
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
 * Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
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

#include "WiresharkWrapper.h"

#include <epan/dissectors/packet-tcp.h>

#include <stdlib.h>

#define ASTERIX_DEFAULT_UDP_RANGE "24,1024-1080"
#define ASTERIX_DEFAULT_TCP_RANGE "24,1024-1080"

static dissector_handle_t asterix_handle;
static gboolean global_asterix_enable_log = 0;
static range_t *global_asterix_udp_port_range;
static range_t *asterix_udp_port_range;
static range_t *global_asterix_tcp_port_range;
static range_t *asterix_tcp_port_range;
#ifdef LINUX
static const char* global_asterix_ini_file_path = "/usr/local/lib/wireshark/plugins/asterix";
#else
static const char *global_asterix_ini_file_path = "C:\\Program Files\\Wireshark\\plugins";
#endif

void proto_reg_handoff_asterix(void);

static hf_register_info *hfd = NULL;
static int maxpid = 0;
static int *def_pid = NULL;

/* Define the asterix proto */
static int proto_asterix = -1;

#define MAX_TREE_DEPTH 5
static gint ett_asterix[MAX_TREE_DEPTH] = {-1, -1, -1, -1, -1};

static void logTraceFunc(char const *format, ...) {
    FILE *fp = fopen("asterix.log", "at");
    if (fp) {
        va_list args;
        va_start(args, format);
        vfprintf(fp, format, args);
        va_end(args);
        fclose(fp);
        g_print("Opened asterix.log file");
    } else {
        g_print("Unable to open asterix.log file");
    }
    g_print(format);
}

static void dissect_asterix_pdu(tvbuff_t *tvb, packet_info *pinfo,
                                proto_tree *tree) {
    proto_tree *asterix_tree[5];
    int tree_depth = 0;
    proto_item *t_head_item;
    proto_item *t_item = NULL;
    static fulliautomatix_data *pDataList = NULL;
    fulliautomatix_data *pData = NULL;
    guint32 offset = 0;
    guint8 category;
    guint32 payload_len;
    char tmpstr[256];
    char tmp2[64];
    int error = 0;

    tmpstr[0] = 0;
    while (offset < tvb_length(tvb) && strlen(tmpstr) < 200) {
        category = tvb_get_guint8(tvb, offset);
        offset += 1;
        payload_len = tvb_get_ntohs(tvb, offset);
        offset += 2;
        offset += payload_len - 3;
        sprintf(tmp2, "CAT%03d (%-3d bytes),", category, payload_len);
        strcat(tmpstr, tmp2);
    }

    if (offset != tvb_length(tvb)) {
        sprintf(tmp2, "Total= %d bytes - Wrong length !!!! ", offset);
    } else {
        sprintf(tmp2, "Total = %d bytes", offset);
    }
    strcat(tmpstr, tmp2);

    if (check_col(pinfo->cinfo, COL_PROTOCOL))
        col_set_str(pinfo->cinfo, COL_PROTOCOL, "Asterix");

    if (check_col(pinfo->cinfo, COL_INFO))
        col_add_fstr(pinfo->cinfo, COL_INFO, tmpstr);
    if (!tree)
        return;

    t_head_item = proto_tree_add_protocol_format(tree, proto_asterix, tvb, 0, -1,
                                                 "All Purpose Structured EUROCONTROL Surveillance Information Exchange (Asterix) Protocol");

    asterix_tree[tree_depth] = proto_item_add_subtree(t_head_item, ett_asterix[tree_depth]);

    pDataList = fulliautomatix_parse(tvb_get_ptr(tvb, 0, tvb_length(tvb)), tvb_length(tvb));

    pData = pDataList;
    while (pData) {
        if (pData->tree == 1) {
            t_item = proto_tree_add_text(asterix_tree[tree_depth], tvb, pData->bytenr, pData->length,
                                         pData->description);
            if (tree_depth < MAX_TREE_DEPTH - 1) {
                tree_depth++;
            }
            asterix_tree[tree_depth] = proto_item_add_subtree(t_item, ett_asterix[tree_depth]);
        } else if (pData->tree == -1) {
            if (tree_depth > 0) {
                tree_depth--;
            }
        } else {
            if (pData->pid < 0 || pData->pid > maxpid) {
                logTraceFunc("Wrong PID.\n");
            } else if (pData->type == FT_STRINGZ) {
                t_item = proto_tree_add_string(asterix_tree[tree_depth], def_pid[pData->pid], tvb, pData->bytenr,
                                               pData->length, pData->val.str);
            } else if (pData->type == FT_UINT32) {
                t_item = proto_tree_add_uint(asterix_tree[tree_depth], def_pid[pData->pid], tvb, pData->bytenr,
                                             pData->length, pData->val.ul);
            } else if (pData->type == FT_INT32) {
                t_item = proto_tree_add_int(asterix_tree[tree_depth], def_pid[pData->pid], tvb, pData->bytenr,
                                            pData->length, pData->val.sl);
            } else if (pData->type == FT_BYTES) {
                t_item = proto_tree_add_bytes(asterix_tree[tree_depth], def_pid[pData->pid], tvb, pData->bytenr,
                                              pData->length, pData->val.str);
            } else if (pData->type == FT_NONE) {
                t_item = proto_tree_add_text(asterix_tree[tree_depth], tvb, pData->bytenr, pData->length,
                                             pData->val.str);
            }
            //TODO add other types
        }
        if (pData->value_description) {
            proto_item_append_text(t_item, pData->value_description);
        }
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

    if (error == 1) {
        proto_item_set_expert_flags(t_head_item, PI_GROUP_MASK, PI_WARN);
    } else if (error == 2) {
        proto_item_set_expert_flags(t_head_item, PI_REASSEMBLE, PI_ERROR);
    }

    fulliautomatix_data_destroy(pDataList);
}

static guint
get_asterix_pdu_len(packet_info *pinfo _U_, tvbuff_t *tvb, int offset) {
    return tvb_get_ntohs(tvb, offset + 1);
}

static void dissect_asterix(tvbuff_t *tvb, packet_info *pinfo, proto_tree *tree) {
    tcp_dissect_pdus(tvb, pinfo, tree, TRUE, 3, get_asterix_pdu_len, dissect_asterix_pdu);
}

void
proto_register_asterix(void) {
    module_t *asterix_module;

    proto_asterix = proto_register_protocol("Asterix", "Asterix", "asterix");

    asterix_module = prefs_register_protocol(proto_asterix, proto_reg_handoff_asterix);

    /*
    uint global_asterix_tcp_port = 111;
    prefs_register_uint_preference(asterix_module, "tcp.asterix_port",
                                   "Asterix listener TCP Port",
                                   "Set the TCP port for Asterix"
                                   "(if other than the default of 705)",
                                   10, &global_asterix_tcp_port);
    */
    /*
    prefs_register_uint_preference(asterix_module, "udp.asterix_port",
                                   "Asterix listener UDP Port",
                                   "Set the UDP port for Asterix"
                                   "(if other than the default of 32773)",
                                   10, &global_asterix_udp_port);
    */

    range_convert_str(&global_asterix_udp_port_range, ASTERIX_DEFAULT_UDP_RANGE, 65535);
    range_convert_str(&global_asterix_tcp_port_range, ASTERIX_DEFAULT_TCP_RANGE, 65535);

    prefs_register_range_preference(asterix_module, "udp.asterix_ports", "Asterix listener UDP Ports",
                                    "Set the UDP ports for Asterix",
                                    &global_asterix_udp_port_range, MAX_UDP_PORT);

    prefs_register_range_preference(asterix_module, "tcp.asterix_ports", "Asterix listener TCP Ports",
                                    "Set the TCP ports for Asterix",
                                    &global_asterix_tcp_port_range, MAX_TCP_PORT);

    prefs_register_bool_preference(asterix_module, "udp.log_to_file",
                                   "Log to file",
                                   "Enable logs from Asterix plugin",
                                   &global_asterix_enable_log);

    prefs_register_string_preference(asterix_module, "udp.asterix_ini",
                                     "Asterix ini file path", "Asterix ini file path", &global_asterix_ini_file_path);
}

static void
range_add_callback(guint32 port) {
    dissector_add("udp.port", port, asterix_handle);
    dissector_add("tcp.port", port, asterix_handle);
}

static void
range_delete_callback(guint32 port) {
    dissector_delete("udp.port", port, asterix_handle);
    dissector_delete("tcp.port", port, asterix_handle);
}


/* The registration hand-off routine */
void
proto_reg_handoff_asterix(void) {
    int len = 0;
    int ret;
    int i;

    static gint *ett[] = {
            &ett_asterix[0],
            &ett_asterix[1],
            &ett_asterix[2],
            &ett_asterix[3],
            &ett_asterix[4]
    };

    static fulliautomatix_definitions *deflist = NULL;
    fulliautomatix_definitions *def = NULL;
    static gboolean asterix_prefs_initialized = FALSE;

    if (!asterix_prefs_initialized) {
        asterix_handle = create_dissector_handle(dissect_asterix, proto_asterix);
        asterix_prefs_initialized = TRUE;

        if (global_asterix_enable_log == 0) {
            ret = fulliautomatix_start(g_print, global_asterix_ini_file_path);
        } else {
            ret = fulliautomatix_start(logTraceFunc, global_asterix_ini_file_path);
        }
        if (ret != 0) {
            logTraceFunc("Failed to initialize Asterix plugin (%d)\n", ret);
        }

        if (deflist) {
            fulliautomatix_destroy_definitions(deflist);
        }

        deflist = def = fulliautomatix_get_definitions();

        // get len
        len = 0;
        while (def) {
            if (def->pid > maxpid)
                maxpid = def->pid;
            len++;
            def = def->next;
        }

        def_pid = (int *) malloc(sizeof(int) * (maxpid + 1));
        for (i = 0; i <= maxpid; i++) {
            def_pid[i] = -1;
        }

        hfd = (hf_register_info *) malloc(sizeof(hf_register_info) * len);
        memset(hfd, 0, sizeof(hf_register_info) * len);

        def = deflist;

        for (i = 0; i < len && def; i++) {
            hfd[i].p_id = &def_pid[def->pid];
            hfd[i].hfinfo.name = def->name;
            hfd[i].hfinfo.abbrev = def->abbrev;
            hfd[i].hfinfo.type = def->type;
            hfd[i].hfinfo.display = def->display;
            hfd[i].hfinfo.strings = def->strings;
            hfd[i].hfinfo.bitmask = def->bitmask;
            hfd[i].hfinfo.blurb = def->blurb;
            def = def->next;
        }

        proto_register_field_array(proto_asterix, hfd, len);

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
}


