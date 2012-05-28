/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef UI_H
#define UI_H

#include "player.h"

#include <gtk/gtk.h>

struct ui_skin_albumcover
{
	int x;
	int y;
	int width;
	int height;
	char *nocover;
};

struct ui_skin_image
{
	int x;
	int y;
	char *src;
};

struct ui_skin_label
{
	int x;
	int y;
	int max_char;
	char *font;
	char *color;

	/* 0 right, 1 left, 0.5 middle */
	float align;

	/* 0 disable, 1 enable */
	int enable;
};

struct ui_skin
{
	char *name;
	int width;
	int height;

	struct ui_skin_albumcover *albumcover;
	struct ui_skin_image *img_case;
	struct ui_skin_label *title;
	struct ui_skin_label *artist;
	struct ui_skin_label *album;
	struct ui_skin_image *img_prev;
	struct ui_skin_image *img_pause;
	struct ui_skin_image *img_play;
	struct ui_skin_image *img_next;
};

struct ui_skin_entity
{
	char *skin_name;
	char *skin_conf;
};

#define DEF_SKIN 			"default"

void ui_skin_free(struct ui_skin *ptr);

struct ui_skin *ui_get_current_skin();

struct ui_skin *ui_skin_load();

struct ui_skin_entity **ui_skin_load_all();

void ui_skin_entity_free(struct ui_skin_entity *ptr);

void ui_cover_init(GtkWidget *fixed, GtkWidget *cover_container);

void ui_cover_update(struct player_music_info *info);

void ui_player_control_init(GtkWidget *fixed);

void ui_player_control_update();

void ui_player_info_init(GtkWidget *fixed);

void ui_player_info_update(struct player_music_info *info);

void ui_menu_init(GtkWidget *menu_container);

void ui_load();

void ui_update(const char *skin_name);

int ui_set_transparent(GtkWidget *widget, GdkScreen *old_screen, void *data);

#endif
