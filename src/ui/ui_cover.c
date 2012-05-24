/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "coverfetch.h"
#include "coverfetch.h"
#include "player.h"
#include "ui.h"

#include <string.h>
#include <stdlib.h>
#include <gtk/gtk.h>

static GtkWidget *container;

void ui_cover_init(GtkWidget *fixed, GtkWidget *cover_container)
{
	GtkWidget *cover;
	GtkWidget *cover_case;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scalebuf;

	struct ui_skin *a_skin = ui_get_current_skin();

	pixbuf = gdk_pixbuf_new_from_file(a_skin->albumcover->nocover, NULL);

	scalebuf = gdk_pixbuf_scale_simple(pixbuf, a_skin->albumcover->width, a_skin->albumcover->height, GDK_INTERP_BILINEAR);

	cover = gtk_image_new_from_pixbuf(scalebuf);

	cover_case = gtk_image_new_from_file(a_skin->img_case->src);

	gtk_container_add(GTK_CONTAINER(cover_container), cover);

	gtk_fixed_put(GTK_FIXED(fixed), cover_container, a_skin->albumcover->x, a_skin->albumcover->y);

	gtk_fixed_put(GTK_FIXED(fixed), cover_case, a_skin->img_case->x, a_skin->img_case->y);

	container = cover_container;

	g_object_unref(pixbuf);
	g_object_unref(scalebuf);
}

void ui_cover_update(struct player_music_info *info)
{
	GtkWidget *new_cover;
	GtkWidget *old_cover;
	GdkPixbuf *pixbuf;
	GdkPixbuf *scalebuf;

	struct ui_skin *a_skin;

	char *cover_src;

	a_skin = ui_get_current_skin();

	cover_src = getcover(info->uri, info->artist, info->title, info->album, doubancover);

	if(cover_src == NULL)
	{
		cover_src = strdup(a_skin->albumcover->nocover);
	}

	pixbuf = gdk_pixbuf_new_from_file(cover_src, NULL);

	scalebuf = gdk_pixbuf_scale_simple(pixbuf, a_skin->albumcover->width, a_skin->albumcover->height, GDK_INTERP_BILINEAR);

	new_cover = gtk_image_new_from_pixbuf(scalebuf);

	old_cover = gtk_container_get_children(GTK_CONTAINER(container))->data;

	gtk_container_remove( GTK_CONTAINER(container), old_cover);

	gtk_container_add(GTK_CONTAINER(container), new_cover);

	gtk_widget_show(new_cover);

	free(cover_src);
	g_object_unref(pixbuf);
	g_object_unref(scalebuf);
}
