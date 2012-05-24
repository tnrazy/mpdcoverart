/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "ui.h"

#include <gtk/gtk.h>
#include <gdk/gdkscreen.h>
#include <cairo.h>
#include <assert.h>

static int ui_do_transparent(GtkWidget *widget, GdkEventExpose *event, void *data);

int ui_set_transparent(GtkWidget *widget, GdkScreen *old_screen, void *data)
{
	assert(widget != NULL);

	GdkScreen *screen = gtk_widget_get_screen(widget);
	GdkColormap *colormap = gdk_screen_get_rgba_colormap(screen);

	if(!colormap)
	{
		return FALSE;
	}

	gtk_widget_set_colormap(widget, colormap);

	g_signal_connect(widget, "screen_changed", G_CALLBACK(ui_set_transparent), NULL);
	g_signal_connect(widget, "expose_event", G_CALLBACK(ui_do_transparent), NULL);

	return FALSE;
}

/* 
 * draw the transparent before show widget 
 * */
static int ui_do_transparent(GtkWidget *widget, GdkEventExpose *event, void *data)
{
   	cairo_t *cr = gdk_cairo_create(widget->window);

	/* transparent */
	cairo_set_source_rgba(cr, 1.0, 1.0, 1.0, 0.0);

	/* draw the background */
	cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
	cairo_paint(cr);

	cairo_destroy(cr);

	return FALSE;
}
