/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "ui.h"
#include "setting.h"

#include <stdlib.h>
#include <string.h>
#include <gtk/gtk.h>

static int e_window_drag(GtkWidget *widget, GdkEventButton *event, void *data);

static int e_menu_popup(GtkWidget *widget, GdkEventButton *event, void *data);

static void s_menu_quit(GtkWidget *widget, GdkEventButton *event);

static void s_menu_skin(GtkWidget *widget, GdkEventButton *event);

static void s_menu_lock(GtkWidget *widget, GdkEventButton *event);

static GtkWidget **skin_menu_items;

void ui_menu_init(GtkWidget *container)
{
	GtkWidget *menu;
	GtkWidget *skin_menu;
	GtkWidget *item_skin;
	GtkWidget *item_quit;
	GtkWidget *item_lock;

	int idx = 0;

	menu = gtk_menu_new();

	/* menu skin */

	skin_menu = gtk_menu_new();

	for(struct ui_skin_entity **skins = ui_skin_load_all(), **shell = skins, *skin = *skins; skin;)
	{
		if(skin_menu_items == NULL)
			skin_menu_items = calloc(sizeof *skin_menu_items, 1 + 1);
		else
			skin_menu_items = realloc(skin_menu_items, (idx + 1 + 1) * sizeof *skin_menu_items);

		*(skin_menu_items + idx) = gtk_check_menu_item_new_with_label(skin->skin_name);
		*(skin_menu_items + idx + 1) = NULL;

		if(strcmp(skin->skin_name, ui_get_current_skin()->name) == 0)
		{
			/* set active */
			GTK_CHECK_MENU_ITEM( *(skin_menu_items + idx) )->active = TRUE;
		}

		gtk_check_menu_item_set_draw_as_radio(GTK_CHECK_MENU_ITEM( *(skin_menu_items + idx) ), TRUE);

		gtk_menu_append(GTK_MENU(skin_menu), *(skin_menu_items + idx));

		g_signal_connect(*(skin_menu_items + idx), "activate", G_CALLBACK(s_menu_skin), NULL);

		ui_skin_entity_free(skin);

		++idx;
		skin = *++skins;

		if(skin == NULL)
			free(shell);
	}

	item_skin = gtk_menu_item_new_with_label("Skin");

	gtk_menu_item_set_submenu(GTK_MENU_ITEM(item_skin), skin_menu);

	gtk_menu_append(GTK_MENU(menu), item_skin);

	/* menu lock */
	item_lock = gtk_check_menu_item_new_with_label("Lock");

	if(cfg_get_pos_lock())
	{
		gtk_menu_item_activate(GTK_MENU_ITEM(item_lock));
	}

	gtk_menu_append(GTK_MENU(menu), item_lock);

	/* menu quit */
	item_quit = gtk_menu_item_new_with_label("Quit");

	gtk_menu_append(GTK_MENU(menu), item_quit);

	gtk_widget_show_all(menu);

	gtk_widget_set_events(container, GDK_BUTTON_PRESS_MASK);

	g_signal_connect(container, "button_press_event", G_CALLBACK(e_menu_popup), menu);	
	g_signal_connect(container, "button_press_event", G_CALLBACK(e_window_drag), gtk_widget_get_toplevel(container));
	g_signal_connect(item_quit, "activate", G_CALLBACK(s_menu_quit), NULL);
	g_signal_connect(item_lock, "activate", G_CALLBACK(s_menu_lock), NULL);
}

static int e_menu_popup(GtkWidget *widget, GdkEventButton *event, void *data)
{
	switch(event->button)
	{
		case 1:
			/* mouse left */
			break;
		case 2:
			/* mouse middle */
			break;
		case 3:
			gtk_menu_popup(GTK_MENU(data), NULL, NULL, NULL, NULL,
			  			event->button, event->time);
	}

	return FALSE;
}

static void s_menu_quit(GtkWidget *widget, GdkEventButton *event)
{
	/* quit */
	gtk_main_quit();
}

static void s_menu_lock(GtkWidget *widget, GdkEventButton *event)
{
	/* lock the postion */
	cfg_set_postion_lock();
}

static int e_window_drag(GtkWidget *widget, GdkEventButton *event, void *data)
{
	char position[32];
	int x, y;

	if(cfg_get_pos_lock() == 1)
		return TRUE;

	if(event->button == 1)
	{
		gtk_window_begin_move_drag(GTK_WINDOW(data), event->button, event->x_root, event->y_root, event->time);
	}

	gdk_window_get_position(GTK_WIDGET(data)->window, &x, &y);

	snprintf(position, 32, "%d,%d", x, y);

	cfg_set_postion(position);

	return TRUE;
}

static void s_menu_skin(GtkWidget *widget, GdkEventButton *event)
{
	const char *skin_name, *new_skin_name;

	/* set active */
	GTK_CHECK_MENU_ITEM(widget)->active = TRUE;

	new_skin_name = gtk_menu_item_get_label(GTK_MENU_ITEM(widget));

	for(GtkWidget **skin_items = skin_menu_items, *ptr = *skin_items; ptr;)
	{
		skin_name = gtk_menu_item_get_label(GTK_MENU_ITEM(ptr));

		if(strcmp(new_skin_name, skin_name))
		{
			/* set deactive */
			GTK_CHECK_MENU_ITEM(ptr)->active = FALSE;
		}

		ptr = *++skin_items;
	}

	/* change skin */
	if(strcmp(new_skin_name, ui_get_current_skin()->name))
	{
		cfg_set_skinname((char *)new_skin_name);

		ui_update(new_skin_name);
	}
}
