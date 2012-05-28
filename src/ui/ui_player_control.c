/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "ui.h"
#include "player.h"

#include <gtk/gtk.h>

static GtkWidget *ui_make_img_btn(char *img_btn);

static int e_img_btn_handle(GtkWidget *widget, GdkEventButton *event, void *data);

static int e_player_status(GtkWidget *widget, GdkEventButton *event, void *data);

static int e_player_next(GtkWidget *widget, GdkEventButton *event, void *data);

static int e_player_prev(GtkWidget *widget, GdkEventButton *event, void *data);

/* update this container */
static GtkWidget *container;

void ui_player_control_init(GtkWidget *fixed)
{
	GtkWidget *btn_prev;
	GtkWidget *btn_next;
	GtkWidget *btn_status;

	struct ui_skin *a_skin;
	struct ui_skin_image *status;

	a_skin = ui_get_current_skin();

	if(player_get_status() == PLAYER_PLAY)
	{
		status = a_skin->img_pause;
	}
	else
		status = a_skin->img_play;

	btn_prev = ui_make_img_btn(a_skin->img_prev->src);
	btn_next = ui_make_img_btn(a_skin->img_next->src);
	btn_status = ui_make_img_btn(status->src);

	gtk_fixed_put(GTK_FIXED(fixed), btn_prev, a_skin->img_prev->x, a_skin->img_prev->y);
	gtk_fixed_put(GTK_FIXED(fixed), btn_next, a_skin->img_next->x, a_skin->img_next->y);
	gtk_fixed_put(GTK_FIXED(fixed), btn_status, status->x, status->y);

	/* bind event */
	g_signal_connect(btn_prev, "button_press_event", G_CALLBACK(e_player_prev), NULL);
	g_signal_connect(btn_next, "button_press_event", G_CALLBACK(e_player_next), NULL);
	g_signal_connect(btn_status, "button_press_event", G_CALLBACK(e_player_status), NULL);

	container = btn_status;
}

void ui_player_control_update()
{
	GtkWidget *new_img;
	GtkWidget *old_img;

	struct ui_skin_image *status;
	struct ui_skin *a_skin;

	a_skin = ui_get_current_skin();

	/* get musci player status, is play set pause and change the image */
	if(player_get_status() == PLAYER_PLAY)
		status = a_skin->img_pause;
	else
		status = a_skin->img_play;
	
	old_img = GTK_WIDGET(gtk_container_children(GTK_CONTAINER(container))->data);

	gtk_container_remove(GTK_CONTAINER(container), old_img);

	new_img = gtk_image_new_from_file(status->src);

	gtk_container_add(GTK_CONTAINER(container), new_img);

	gtk_widget_show(new_img);
}

static GtkWidget *ui_make_img_btn(char *img_btn)
{
	GtkWidget *image;
	GtkWidget *eventbox;

	eventbox = gtk_event_box_new();

	gtk_widget_set_events(eventbox, GDK_MOTION_NOTIFY | GDK_BUTTON_RELEASE
	  				| GDK_BUTTON_PRESS
	  				| GDK_ENTER_NOTIFY
	  				| GDK_LEAVE_NOTIFY);

	image = gtk_image_new_from_file(img_btn);

	gtk_container_add(GTK_CONTAINER(eventbox), image);

	/* set eventbox not visible, only accept event */
	gtk_event_box_set_visible_window(GTK_EVENT_BOX(eventbox), FALSE);

	/* just change the mouse style */
	g_signal_connect(eventbox, "button_press_event", G_CALLBACK(e_img_btn_handle), NULL);
	g_signal_connect(eventbox, "button_release_event", G_CALLBACK(e_img_btn_handle), NULL);
	g_signal_connect(eventbox, "enter_notify_event", G_CALLBACK(e_img_btn_handle), NULL);
	g_signal_connect(eventbox, "leave_notify_event", G_CALLBACK(e_img_btn_handle), NULL);

	return eventbox;
}

static int e_img_btn_handle(GtkWidget *widget, GdkEventButton *event, void *data)
{
	/* ... */

	if(event->type == GDK_ENTER_NOTIFY)
	{
		gdk_window_set_cursor(widget->window, gdk_cursor_new(GDK_HAND1));
	}

	else if(event->type == GDK_LEAVE_NOTIFY)
	{
		gdk_window_set_cursor(widget->window, gdk_cursor_new(GDK_LEFT_PTR));
	}

	return FALSE;
}

static int e_player_status(GtkWidget *widget, GdkEventButton *event, void *data)
{
	if(player_toggle() < 0)
	{
		return FALSE;
	}

	ui_update(NULL);

	return TRUE;
}

static int e_player_next(GtkWidget *widget, GdkEventButton *event, void *data)
{
	if(player_next() < 0)
	{
		/* fail */
		return FALSE;
	}

	/* refresh ui info */
	ui_update(NULL);

	return TRUE;
}

static int e_player_prev(GtkWidget *widget, GdkEventButton *event, void *data)
{
	if(player_prev() < 0)
	{
		/* fail */
		return FALSE;
	}

	ui_update(NULL);

	return TRUE;
}

