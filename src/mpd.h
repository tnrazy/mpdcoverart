/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef MPD_H
#define MPD_H

#include <mpd/client.h>

int mpd_currentstaus(struct mpd_status **status, struct mpd_entity **entity);

const char *mpd_song_tag(const struct mpd_song *song, enum mpd_tag_type type);

const char *mpd_song_title(const struct mpd_song *song);

const char *mpd_song_artist(const struct mpd_song *song);

const char *mpd_song_album(const struct mpd_song *song);

const char *mpd_song_uri(const struct mpd_song *song);

const unsigned int mpd_song_pos(const struct mpd_song *song);

#endif
