/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#define ___DEBUG

#include "coverfetch.h"
#include "setting.h"
#include "msg.h"
#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

static char *getcover_local(const char *uri, const char *artist, const char *title);

static char *getcover_network(const char *uri, const char *artist, const char *title, const char *album, coverfetch fetch);

static char *getcover_network_v(coverfetch fetch, char *path, const char *format, ...);

char *getcover(const char *uri, const char *artist, const char *title, const char *album, coverfetch fetch)
{
    	char *cover = NULL, *cover_path, *music_path, *cmd, newname[1024];

	if(uri == NULL)
	{
		_ERROR("Music URI is null.");
		return NULL;
	}
    
	/* get cover from local */
	if(cover = getcover_local(uri, artist, title), cover)
	{
		_INFO("Find Cover: %s", cover);
		return cover;
	}

	if(fetch == NULL)
	{
		_WARN("Fetcher is null.");

		return NULL;
	}

	/* else get cover from network */
	if(cover = getcover_network(uri, artist, title, album, fetch), cover)
	{
		/* rename as "artist - title" */
		cover_path = cfg_get_coverpath();

		if(artist && title && strrchr(cover, '.'))
			snprintf(newname, sizeof newname, "%s%s - %s%s", cover_path, artist, title, strrchr(cover, '.'));
		else
		{
			/* cfg_get_coverpath always return path end with '/' */
			music_path = cfg_get_musicpath();

			/* move the cover to the music path, name eg: /media/music/xxx/folder.jpg */
			snprintf(newname, sizeof newname - strlen(DEF_COVER_NAME) - 1, "%s%s", music_path, uri);

			/* erase the music file name */
			if(strrchr(newname, '/'))
				*(strrchr(newname, '/') + 1) = '\0';

			strcat(newname, DEF_COVER_NAME);

			free(music_path);

			/* make command without error output */
			cmd = calloc(strlen(cover) + strlen(newname) + 30, 1);
			sprintf(cmd, "mv \"%s\" \"%s\" 2> /dev/null", cover, newname);

			if(system(cmd) == 0)
			{
				_INFO("Move \"%s\" ==> \"%s\"", cover, newname);
			}

			free(cmd);

			return strdup(newname);
		}

		/* renmane the cover name, artist - title */
		if(rename(cover, newname) == 0)
		{
			_INFO("Rename \"%s\" ==> \"%s\".", cover, newname);
		}

		free(cover_path);
	}

	return cover ? strdup(newname) : cover;
}

static char *getcover_network(const char *uri, const char *artist, const char *title, const char *album, coverfetch fetch)
{
    	char *cover_path, *cover, key[1024];

	cover_path = cfg_get_coverpath();

	if(artist && title && album)
	{
		if(cover = getcover_network_v(fetch, cover_path, "%s %s %s", artist, title, album), cover)
		{
			free(cover_path);
			return cover;
		}
	}

	if(artist && title)
	{
		if(cover = getcover_network_v(fetch, cover_path, "%s %s", artist, title), cover)
		{
			free(cover_path);
			return cover;
		}
	}

	if(artist && album)
	{
		if(cover = getcover_network_v(fetch, cover_path, "%s %s", artist, album), cover)
		{
			free(cover_path);
			return cover;
		}
	}

	if(artist)
	{

		if(cover = getcover_network_v(fetch, cover_path, "%s", artist), cover)
		{
			free(cover_path);
			return cover;
		}
	}

	/* try to use the file name search cover */
	{
		/* remove the path name */
		if(strrchr(uri, '/'))
			snprintf(key, sizeof key, "%s", strrchr(uri, '/') + 1);

		/* remove the file extension name */
		if(strrchr(key, '.'))
			*(strrchr(key, '.')) = '\0';

		/* content of search [0-9a-zA-Z\\s] */
		clean_name(key);

		_DEBUG("Key: %s", key);

		if(cover = getcover_network_v(fetch, cover_path, "%s", key), cover)
		{
			free(cover_path);
			return cover;
		}
	}

	free(cover_path);
	return NULL;
}

static char *getcover_network_v(coverfetch fetch, char *path, const char *format, ...)
{
    	char key[1024];

	va_list ap;
	va_start(ap, format);
	vsnprintf(key, sizeof key, format, ap);
	va_end(ap);

	return fetch(key, path);
}

static char *getcover_local(const char *uri, const char *artist, const char *title)
{
    	char *music_path, *cover_path, *tmp, rule[512], pattern[1024];

	/* search result */
    	char **cover_list = NULL;

	/* get image from the .cover */
	if(artist && title)
	{
		cover_path = cfg_get_coverpath();

		snprintf(pattern, sizeof pattern, "%s - %s", artist, title);

		/* replace '(' to '\(' '^' to '\^' ... */
		tmp = reg_clean(pattern);

		snprintf(pattern, sizeof pattern, "^%s.(jpg|png|JPG|PNG)$", tmp);
		free(tmp);

		_DEBUG("Search local path: %s, %s", cover_path, pattern);

		/* search cover in the COVERPATH, directory's depth is 0, not search the child directory, max matched 1 */
		if(cover_list = dir_search(cover_path, pattern, 0, 1), cover_list)
		{
			tmp = *cover_list;

			free(cover_list);
			free(cover_path);
			return tmp;
		}

		free(cover_path);
	}

	/* search cover from music path */
	music_path = cfg_get_musicpath();

	snprintf(pattern, sizeof pattern, "%s%s", music_path, uri);

	/* remove music file name */
	if(strrchr(pattern, '/'))
		*(strrchr(pattern, '/') + 1) = '\0';

	/* get cover from the music path */
	for(char *list = cfg_get_rule(), *buf = list; buf;)
	{
		_DEBUG("Ruelist: %s", list);
		buf = get_ln(buf, strlen(buf), rule, sizeof rule, ";");

		if(buf == NULL)
		{
			free(list);
			break;
		}

		/* remove the separator ";" */
		*(rule + strlen(rule) - 1) = '\0';

		_DEBUG("Search local: %s, %s", pattern, rule);

		if(cover_list = dir_search(pattern, rule, 0, 1), cover_list)
		{
			tmp = *cover_list;

			free(list);
			free(cover_list);
			free(music_path);
			return tmp;
		}
	}

	free(music_path);
	return NULL;
}
