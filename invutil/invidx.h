/*
 * Copyright (c) 2002 Silicon Graphics, Inc.  All Rights Reserved.
 *
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of version 2 of the GNU General Public License as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it would be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 *
 * Further, this software is distributed without any warranty that it is
 * free of the rightful claim of any third person regarding infringement
 * or the like.  Any license provided herein, whether implied or
 * otherwise, applies only to this software file.  Patent licenses, if
 * any, provided herein do not apply to combinations of this program with
 * other software, or any other product whatsoever.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, write the Free Software Foundation, Inc., 59
 * Temple Place - Suite 330, Boston MA 02111-1307, USA.
 *
 * Contact information: Silicon Graphics, Inc., 1600 Amphitheatre Pkwy,
 * Mountain View, CA  94043, or:
 *
 * http://www.sgi.com
 *
 * For further information regarding this notice, see:
 *
 * http://oss.sgi.com/projects/GenInfo/SGIGPLNoticeExplan/
 */
#ifndef INVIDX_H
#define INVIDX_H

#include "types.h"
#include "mlog.h"
#include "inv_priv.h"
#include "list.h"
#include "cmenu.h"

/* invidx.c */
node_t *generate_invidx_menu(char *inv_path, node_t *startnode, int level, char *idxFileName);
int open_invidx(char *idxFileName);
int close_invidx(int idx);
int close_all_invidx(void);
void *remmap_invidx(int fidx, int num);
int find_matching_stobjfile(char *filename);
int find_matching_invidxfile(char *filename);
int find_overlapping_invidx(int fidx, invt_entry_t *inv_entry);
int find_invidx_insert_point(int fd, time_t sh_time);
int find_invidx_insert_pos(int fidx, invt_entry_t *inv_entry);
char *get_inventry_stobjfile(data_t *d);
node_t *find_invidx_node(node_t *list, int inv_fileidx);
int read_stobj_info(int fd, int idx, invt_seshdr_t **hdr, invt_session_t **ses,
		invt_stream_t **strms, invt_mediafile_t **mfiles);
int insert_stobj_into_stobjfile(int invidx_fileidx, char *filename, int fd,
 		invt_seshdr_t *hdr, invt_session_t *ses, invt_stream_t *strms,
		invt_mediafile_t *mfiles);
int insert_stobj_into_inventory(int invidx_fileidx, node_t *node);
int invidx_undelete(WINDOW *win, node_t *current, node_t *list);
int invidx_select(WINDOW *win, node_t *current, node_t *list);
int invidx_highlight(WINDOW *win, node_t *current, node_t *list);
int invidx_commit(WINDOW *win, node_t *current, node_t *list);
int invidx_prune(char *mountpt, uuid_t *uuidp, time_t prunetime, node_t *node, node_t *list);

#endif /* INVIDX_H */
