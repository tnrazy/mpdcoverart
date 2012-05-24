/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "setting.h"

#include <gtk/gtk.h>

static int e_window_drag(GtkWidget *widget, GdkEventButton *event, void *data);

static int e_menu_popup(GtkWidget *widget, GdkEventButton *event, void *data);

static void s_menu_quit(GtkWidget *widget, GdkEventButton *event);

static void s_menu_lock(GtkWidget *widget, GdkEventButton *event);

void ui_menu_init(GtkWidget *container)
{
	GtkWidget *menu;
	GtkWidget *item_quit;
	GtkWidget *item_lock;

	menu = gtk_menu_new();

	item_lock = gtk_check_menu_item_new_with_label("Lock");

	if(cfg_get_pos_lock() == 1)
	{
		gtk_menu_item_activate(GTK_MENU_ITEM(item_lock));
	}

	gtk_menu_append(GTK_MENU(menu), item_lock);

	gtk_widget_show(item_lock);

	item_quit = gtk_menu_item_new_with_label("Quit");

	gtk_menu_append(GTK_MENU(menu), item_quit);

	gtk_widget_show(item_quit);

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
