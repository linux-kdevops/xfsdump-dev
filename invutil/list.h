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
#ifndef LIST_H
#define LIST_H

struct menu_ops_s;

typedef struct node_s {
    void *data;
    struct node_s *next;
    struct node_s *prev;
} node_t;

typedef struct {
    int		level;		/* indentation level in menu */
    int		hidden;		/* flag: displayed in menu */
    int		expanded;	/* flag: children displayed in menu */
    int		deleted;	/* flag: marked as deleted in menu */
    int		imported;	/* flag: marked as imported in menu */
    int		commited;	/* flag: record already commited */
    int		file_idx;	/* index into file table */
    char	*text;		/* text displayed in menu */
    struct menu_ops_s *ops;	/* table of applicable operations */
    node_t	*parent;	/* parent node */
    node_t	**children;	/* child nodes */
    int		nbr_children;	/* number of children nodes */
    int		data_idx;	/* real data in mmap file */
} data_t;

void node_free(node_t *node);
node_t *node_create(int hidden, int expanded, int level, int deleted, int file_idx, char *text,
		    struct menu_ops_s *ops, node_t *parent, node_t **children, int nbr_children,
		    int data_idx);
node_t *list_add(node_t *prevnode, node_t *newnode);
node_t *list_del(node_t *node);
node_t *free_all_children(node_t *node);
node_t *mark_all_children_commited(node_t *node);

#endif /* LIST_H */

