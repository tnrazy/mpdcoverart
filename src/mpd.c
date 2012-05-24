/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "mpd.h"
#include "player.h"
#include "msg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpd/client.h>

struct mpd_connection *mpd_get_connection()
{
    	struct mpd_connection *conn;

	conn = mpd_connection_new(NULL, 0, 3000);
	if(conn == NULL)
	{
		_ERROR("Mpd connection error.");
		return NULL;
	}

	if(mpd_connection_get_error(conn) != MPD_ERROR_SUCCESS)
	{
		_ERROR("%s", mpd_connection_get_error_message(conn));

		mpd_connection_free(conn);
		return NULL;
	}

	return conn;
}

struct mpd_status *mpd_get_status()
{
	struct mpd_connection *conn;
	struct mpd_status *status;

	conn = mpd_get_connection();

	status = mpd_run_status(conn);

	if(status == NULL)
	{
		_ERROR("%s", mpd_connection_get_error_message(conn));
		return NULL;
	}

	mpd_response_finish(conn);
	mpd_connection_free(conn);

	return status;
}

struct mpd_song *mpd_get_song()
{
	struct mpd_connection *conn;
	struct mpd_song *song;

	conn = mpd_get_connection();

	song = mpd_run_current_song(conn);
	if(song == NULL)
	{
		_ERROR("%s", mpd_connection_get_error_message(conn));

		return NULL;
	}

	mpd_response_finish(conn);
	mpd_connection_free(conn);

	return song;
}

const char *mpd_song_tag(const struct mpd_song *song, enum mpd_tag_type type)
{
    	int i = 0;
	const char *value = NULL;

	while(value = mpd_song_get_tag(song, type, i++), value != NULL)
		break;

	return value;
}

enum player_status player_get_status()
{
	struct mpd_status *status;

	if(status = mpd_get_status(), status == NULL)
		return PLAYER_UNKNOW;

	if(mpd_status_get_state(status) == MPD_STATE_PLAY)
		return PLAYER_PLAY;

	if(mpd_status_get_state(status) == MPD_STATE_PAUSE)
		return PLAYER_PAUSE;

	if(mpd_status_get_state(status) == MPD_STATE_STOP)
		return PLAYER_STOP;

	mpd_status_free(status);

	return PLAYER_UNKNOW;
}

struct player_music_info *player_get_music_info()
{
	struct mpd_song *song;
	struct player_music_info *info;
	const char *value;

	if(song = mpd_get_song(), song == NULL)
	{
		printf("song is null.\n");
		return NULL;
	}
	
	info = calloc(sizeof *info, 1);

	if(value = mpd_song_tag((const struct mpd_song *)song, MPD_TAG_ARTIST), value)
	{
		info->artist = strdup(value);
	}

	if(value = mpd_song_tag((const struct mpd_song *)song, MPD_TAG_TITLE), value)
	{
		info->title = strdup(value);
	}

	if(value = mpd_song_tag((const struct mpd_song *)song, MPD_TAG_ALBUM), value)
	{
		info->album = strdup(value);
	}

	if(value = mpd_song_get_uri((const struct mpd_song *)song), value)
	{
		info->uri = strdup(value);
	}

	info->id = mpd_song_get_id((const struct mpd_song *)song);

	mpd_song_free(song);

	return info;
}

int player_next()
{
	struct mpd_connection *conn;
	int res;

	conn = mpd_get_connection();

	res = mpd_run_next(conn);
	
	mpd_connection_free(conn);

	return res;
}

int player_prev()
{
	struct mpd_connection *conn;
	int res;

	conn = mpd_get_connection();

	res = mpd_run_previous(conn);

	mpd_connection_free(conn);

	return res;
}

int player_toggle()
{
	struct mpd_connection *conn;
	int res;

	conn = mpd_get_connection();

	res = mpd_run_toggle_pause(conn);

	mpd_connection_free(conn);

	return res;
}
