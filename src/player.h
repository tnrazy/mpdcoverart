/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef PLAYER_H
#define PLAYER_H

enum player_status
{
	PLAYER_PLAY,
	PLAYER_PAUSE,
	PLAYER_STOP,
	PLAYER_UNKNOW
};

struct player_music_info
{
	char *title;
	char *artist;
	char *album;

	char *uri;
	unsigned int id;
};


void player_music_info_free(struct player_music_info *info);

enum player_status player_get_status();

/* 
 * return a static variable
 * */
struct player_music_info *player_get_current_song();

struct player_music_info *player_get_music_info();

int player_next();

int player_prev();

int player_toggle();

#endif
