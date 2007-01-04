/*
 * baobab-treeview.h
 * This file is part of baobab
 *
 * Copyright (C) 2005-2006 Fabio Marzocca  <thesaltydog@gmail.com>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, 
 * Boston, MA  02110-1301  USA
 */

#ifndef __BAOBAB_TV_H__
#define __BAOBAB_TV_H__

#include <gtk/gtk.h>

/* tree model columns (_H_ are hidden) */
enum
{
	COL_DIR_NAME,
	COL_H_FULLPATH,
	COL_H_PERC,
	COL_DIR_SIZE,
	COL_H_SIZE,
	COL_H_ALLOCSIZE,
	COL_ELEMENTS,
	COL_H_ELEMENTS,
	COL_HARDLINK,
	COL_H_HARDLINK,
	NUM_TREE_COLUMNS
};

/* search model columns */
enum
{
	COL0_ICON,
	COL1_STRING,
	COL2_STRING,
	COL_FULLPATH,
	COL_LASTACCESS,
	COL_SIZE,
	COL_FILETYPE,
	COL_OWNER,	
	NUM_COLUMNS
};

GtkWidget *create_directory_treeview (void);
GtkWidget *create_filesearch_treeview (void);

#endif /* __BAOBAB_TV_H__ */
