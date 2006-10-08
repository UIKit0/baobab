/*
 * callbacks.c
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

#ifdef HAVE_CONFIG_H
#  include <config.h>
#endif

#include <string.h>
#include <gtk/gtk.h>
#include <glib/gi18n.h>
#include <libgnomevfs/gnome-vfs.h>
#include <libgnomevfs/gnome-vfs-mime.h>
#include <libgnomevfs/gnome-vfs-mime-handlers.h>
#include <gconf/gconf-client.h>

#include "baobab.h"
#include "baobab-graphwin.h"
#include "baobab-treeview.h"
#include "baobab-utils.h"
#include "callbacks.h"
#include "baobab-prefs.h"
#include "baobab-remote-connect-dialog.h"

void
on_menuscanhome_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	start_proc_on_dir (g_get_home_dir ());
}

void
on_menuallfs_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	start_proc_on_dir ("file:///");
}

void
on_menuscandir_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	dir_select (FALSE, baobab.window);
}

void
on_esci1_activate (GtkObject *menuitem, gpointer user_data)
{
	baobab.STOP_SCANNING = TRUE;
	gtk_main_quit ();
}

void
on_about_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	const gchar * const authors[] = {
		"Fabio Marzocca <thesaltydog@gmail.com>",
		"Paolo Borelli <pborelli@katamail.com>",
		"Benoît Dejean <benoit@placenet.org>",
		"Igalia (rings-chart widget) <www.igalia.com>",
		NULL
	};

	static const gchar copyright[] = "Fabio Marzocca <thesaltydog@gmail.com> \xc2\xa9 2005-2006";

	gtk_show_about_dialog (NULL,
			       "name", _("Baobab"),
			       "comments", _("A graphical tool to analyze "
					     "disk usage."),
			       "version", VERSION,
			       "copyright", copyright,
			       "logo-icon-name", "baobab",
			       "license", "GPL",
			       "authors", authors, 
			       "translator-credits",
			       _("translator-credits"), NULL);
}

void
on_menu_expand_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_tree_view_expand_all (GTK_TREE_VIEW (baobab.tree_view));
}

void
on_menu_collapse_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	gtk_tree_view_collapse_all (GTK_TREE_VIEW (baobab.tree_view));
}

void
on_menu_stop_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	baobab.STOP_SCANNING = TRUE;
	stop_scan ();
}

void
on_tbscandir_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
	dir_select (FALSE, baobab.window);
}

void
on_tbstop_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
	baobab.STOP_SCANNING = TRUE;
	stop_scan ();
}

void
on_tbscanhome_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
	start_proc_on_dir (g_get_home_dir ());
}

void
on_tbscanall_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
	start_proc_on_dir ("file:///");
}

void
on_tb_scan_remote_clicked (GtkToolButton *toolbutton, gpointer user_data)
{
	gint response;
	GtkWidget *dlg;

	dlg =
	    baobab_remote_connect_dialog_new (GTK_WINDOW (baobab.window),
					      NULL);
	response = gtk_dialog_run (GTK_DIALOG (dlg));

	gtk_widget_destroy (dlg);

	while (gtk_events_pending ())
		gtk_main_iteration ();

	if (response == GTK_RESPONSE_OK)
		start_proc_on_dir (baobab.last_scan_command->str);
}

void
on_menu_scan_rem_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	on_tb_scan_remote_clicked (NULL, NULL);
}

gboolean
on_delete_activate (GtkWidget *widget,
		    GdkEvent *event, gpointer user_data)
{
	on_esci1_activate (NULL, NULL);
	return TRUE;
}

void
open_file_cb (GtkMenuItem *pmenu, gpointer dummy)
{
	GnomeVFSURI *vfs_uri;

	g_assert (!dummy);
	g_assert (baobab.selected_path);

	vfs_uri = gnome_vfs_uri_new (baobab.selected_path);

	if (!gnome_vfs_uri_exists (vfs_uri)) {
		message (_("The document does not exist."), "",
		GTK_MESSAGE_INFO, baobab.window);
		gnome_vfs_uri_unref (vfs_uri);
		return;
	}

	gnome_vfs_uri_unref (vfs_uri);
	open_file_with_application (baobab.selected_path);
}

void
graph_map_cb (GtkMenuItem *pmenu, gchar *path_to_string)
{
	baobab_graphwin_create (GTK_TREE_MODEL (baobab.model),
				path_to_string,
				BAOBAB_GLADE_FILE,
				COL_H_FULLPATH,
				baobab.is_local ? COL_H_ALLOCSIZE : COL_H_SIZE,
				-1);
	g_free (path_to_string);
}

void
trash_dir_cb (GtkMenuItem *pmenu, gpointer dummy)
{
	g_assert (!dummy);
	g_assert (baobab.selected_path);

	if (trash_file (baobab.selected_path)) {
		GtkTreeIter iter;
		guint64 filesize;
		GtkTreeSelection *selection;

		selection =
		    gtk_tree_view_get_selection ((GtkTreeView *) baobab.
						 tree_view);
		gtk_tree_selection_get_selected (selection, NULL, &iter);
		gtk_tree_model_get ((GtkTreeModel *) baobab.model, &iter,
				    5, &filesize, -1);
		gtk_tree_store_remove (GTK_TREE_STORE (baobab.model),
				       &iter);
		if (baobab.bbEnableHomeMonitor)
			contents_changed ();
	}
}

void
volume_changed (GnomeVFSVolumeMonitor *volume_monitor,
		GnomeVFSVolume *volume)
{
	/* filesystem has changed (mounted or unmounted device) */
	baobab_get_filesystem (&g_fs);
	set_label_scan (&g_fs);
	show_label ();
}

void
contents_changed_cb (GnomeVFSMonitorHandle *handle,
		     const gchar *monitor_uri,
		     const gchar *info_uri,
		     GnomeVFSMonitorEventType event_type,
		     gpointer user_data)
{
	gchar *excluding;

	if (!baobab.bbEnableHomeMonitor)
		return;

	if (baobab.CONTENTS_CHANGED_DELAYED)
		return;

	excluding = g_path_get_basename (info_uri);
	if (strcmp (excluding, ".recently-used") == 0   ||
	    strcmp (excluding, ".gnome2_private") == 0  ||
	    strcmp (excluding, ".xsession-errors") == 0 ||
	    strcmp (excluding, ".bash_history") == 0    ||
	    strcmp (excluding, ".gconfd") == 0) {
		g_free (excluding);
		return;
	}
	g_free (excluding);

	baobab.CONTENTS_CHANGED_DELAYED = TRUE;
}

void
on_pref_menu (GtkMenuItem *menuitem, gpointer user_data)
{
	create_props ();
}

void
on_ck_allocated_activate (GtkCheckMenuItem *checkmenuitem,
			  gpointer user_data)
{
	GdkCursor *cursor = NULL;

	if (!baobab.is_local)
		return;

	baobab.show_allocated = gtk_check_menu_item_get_active (checkmenuitem);

	/* change the cursor */
	if (baobab.window->window) {
		cursor = gdk_cursor_new (GDK_WATCH);
		gdk_window_set_cursor (baobab.window->window, cursor);
	}

	set_busy (TRUE);
	set_statusbar (_("Calculating percentage bars..."));
	gtk_tree_model_foreach (GTK_TREE_MODEL (baobab.model),
				show_bars, NULL);
	set_busy (FALSE);
	set_statusbar (_("Ready"));

	/* cursor clean up */
	if (baobab.window->window) {
		gdk_window_set_cursor (baobab.window->window, NULL);
		if (cursor)
			gdk_cursor_unref (cursor);
	}
}

void
on_view_tb_activate (GtkCheckMenuItem *checkmenuitem,
                     gpointer          user_data) 
{
	gboolean visible;

	visible = gtk_check_menu_item_get_active (checkmenuitem);
	set_toolbar_visible (visible);

	gconf_client_set_bool (baobab.gconf_client,
			       BAOBAB_TOOLBAR_VISIBLE_KEY,
			       visible,
			       NULL);
}

void
on_view_sb_activate (GtkCheckMenuItem *checkmenuitem,
                     gpointer          user_data) 
{
	gboolean visible;

	visible = gtk_check_menu_item_get_active (checkmenuitem);
	set_statusbar_visible (visible);

	gconf_client_set_bool (baobab.gconf_client,
			       BAOBAB_STATUSBAR_VISIBLE_KEY,
			       visible,
			       NULL);
}

void
on_menu_treemap_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	GtkTreeIter iter;
	GtkTreePath *path;
	gchar *path_to_string;
	
	if (!gtk_tree_selection_get_selected (gtk_tree_view_get_selection 
							(GTK_TREE_VIEW(baobab.tree_view)), 
					      NULL, 
					      &iter))		 
		return;

	path = gtk_tree_model_get_path(GTK_TREE_MODEL(baobab.model), &iter);
	
	/* path_to_string is freed in graph_map_cb function */
	path_to_string = gtk_tree_path_to_string (path);

	gtk_tree_path_free(path);
	graph_map_cb(NULL, path_to_string);
}

void
on_helpcontents_activate (GtkMenuItem *menuitem, gpointer user_data)
{
	baobab_help_display (GTK_WINDOW (baobab.window), "baobab.xml", NULL);
}

void 
scan_folder_cb (GtkMenuItem *pmenu, gpointer dummy)
{
	GnomeVFSURI *vfs_uri;

	g_assert (!dummy);
	g_assert (baobab.selected_path);

	vfs_uri = gnome_vfs_uri_new (baobab.selected_path);

	if (!gnome_vfs_uri_exists (vfs_uri)) {
		message (_("The folder does not exist."), "", GTK_MESSAGE_INFO, baobab.window);
	}

	gnome_vfs_uri_unref (vfs_uri);
	
	start_proc_on_dir (baobab.selected_path);
}

void
on_tv_selection_changed (GtkTreeSelection *selection, gpointer user_data)
{
	GtkTreeIter iter;
        
	if (gtk_tree_selection_get_selected (selection, NULL, &iter)) {
		GtkTreePath *path;
                
		path = gtk_tree_model_get_path (GTK_TREE_MODEL (baobab.model), &iter);
                
		baobab_ringschart_set_root (baobab.ringschart, path);
		
		gtk_tree_path_free (path);
	}
}

void
on_rchart_sector_activated (BaobabRingschart *rchart, GtkTreeIter *iter)
{
	GtkTreePath *path;

	path = gtk_tree_model_get_path (GTK_TREE_MODEL (baobab.model), iter);

	if (!gtk_tree_view_row_expanded (GTK_TREE_VIEW (baobab.tree_view), path))
                gtk_tree_view_expand_to_path (GTK_TREE_VIEW (baobab.tree_view), path);

	gtk_tree_view_set_cursor (GTK_TREE_VIEW (baobab.tree_view),
				  path, NULL, FALSE);
	gtk_tree_path_free (path);
}