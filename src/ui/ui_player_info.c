/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "player.h"
#include "ui.h"

#include <stdio.h>
#include <gtk/gtk.h>

#define LAB_FORMAT 					"<span foreground='%s' font_desc='%s'>%s</span>"
#define INFO_NA 					"---"

static GtkWidget *lab_artist;
static GtkWidget *lab_title;
static GtkWidget *lab_album;

static GtkWidget *ui_make_label(struct ui_skin_label *label, char *str);

void ui_player_info_init(GtkWidget *fixed)
{
	struct ui_skin *a_skin;

	a_skin = ui_get_current_skin();

	lab_artist = ui_make_label(a_skin->artist, NULL);
	lab_title = ui_make_label(a_skin->title, NULL);
	lab_album = ui_make_label(a_skin->album, NULL);

	if(lab_artist)
		gtk_fixed_put(GTK_FIXED(fixed), lab_artist, a_skin->artist->x, a_skin->artist->y);

	if(lab_title)
		gtk_fixed_put(GTK_FIXED(fixed), lab_title, a_skin->title->x, a_skin->title->y);

	if(lab_album)
		gtk_fixed_put(GTK_FIXED(fixed), lab_album, a_skin->album->x, a_skin->album->y);
}

void ui_player_info_update(struct player_music_info *info)
{
	char *markup = NULL;
	struct ui_skin *a_skin;

	a_skin = ui_get_current_skin();

	if(a_skin->artist->enable == 1)
	{
		markup = g_markup_printf_escaped(LAB_FORMAT, a_skin->artist->color, a_skin->artist->font, info->artist ? info->artist : INFO_NA);

		gtk_label_set_markup(GTK_LABEL(lab_artist), markup);

		g_free(markup);
	}

	else if(lab_artist)
		gtk_label_set_label(GTK_LABEL(lab_artist), NULL);

	if(a_skin->title->enable == 1)
	{
		markup = g_markup_printf_escaped(LAB_FORMAT, a_skin->title->color, a_skin->title->font, info->title ? info->title : INFO_NA);

		gtk_label_set_markup(GTK_LABEL(lab_title), markup);

		g_free(markup);
	}

	else if(lab_title)
		gtk_label_set_label(GTK_LABEL(lab_title), NULL);

	if(a_skin->album->enable == 1)
	{
		markup = g_markup_printf_escaped(LAB_FORMAT, a_skin->album->color, a_skin->album->font, info->album ? info->album : INFO_NA);

		gtk_label_set_markup(GTK_LABEL(lab_album), markup);

		g_free(markup);
	}

	else if(lab_album)
		gtk_label_set_text(GTK_LABEL(lab_album), NULL);
}

static GtkWidget *ui_make_label(struct ui_skin_label *label, char *str)
{
	GtkWidget *widget;
	char format[512];

	if(label->enable != 1)
	{
		return NULL;
	}

	str = str ? str : INFO_NA;

	widget = gtk_label_new(NULL);

	gtk_label_set_line_wrap(GTK_LABEL(widget), FALSE);

	gtk_label_set_max_width_chars(GTK_LABEL(widget), label->max_char);

	gtk_misc_set_alignment(GTK_MISC(widget), label->align, 0);

	snprintf(format, sizeof format, LAB_FORMAT, label->color, label->font, str);

	gtk_label_set_markup(GTK_LABEL(widget), format);

	return widget;
}
