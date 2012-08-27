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
#include "config.h"
#include "log.h"
#include "utils.h"

#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <Imlib2.h>

/*
 * ID3V2 tag
 * */
struct id3_header
{
	char header[3];
	char major_version;
	char minor_version;
	char flags;

	/* tag size, not contain tag header(10 bits) */
	char size[4];
};

/***

struct id3_frame
{
	char id[4];
	char size[4];
	char flags[2];
};

***/

static char *getcover_id3v2(const char *uri);

static char *getcover_local(const char *uri, const char *artist, const char *title);

static char *getcover_network(const char *uri, const char *artist, const char *title, const char *album, coverfetch fetch);

static char *getcover_network_v(coverfetch fetch, char *path, const char *format, ...);

char *getcover(const char *uri, const char *artist, const char *title, const char *album, coverfetch fetch)
{
    	char *cover = NULL, *cover_path, *music_path, *cmd, newname[FILENAME_MAX];

	if(cover = getcover_id3v2(uri), cover)
	{
		_INFO("ID3V2 cover: %s", cover);
		return cover;
	}

	/* get cover from local */
	if(cover = getcover_local(uri, artist, title), cover)
	{
		_INFO("Find Cover: %s", cover);
		return cover;
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
			/* cfg_get_coverpath return path end with '/' */
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
		{
			snprintf(key, sizeof key, "%s", strrchr(uri, '/') + 1);
		}
		else
		{
			strncpy(key, uri, sizeof key);
		}

		/* remove the file extension name */
		if(strrchr(key, '.'))
		{
			*(strrchr(key, '.')) = '\0';
		}

		/* content of search [0-9a-zA-Z\\s] */
		clean_name(key);

		_DEBUG("URI: %s, Key: %s", uri, key);

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
		tmp = clean_reg(pattern);

		snprintf(pattern, sizeof pattern, "^%s.(jpg|png|JPG|PNG)$", tmp);
		free(tmp);

		_DEBUG("Search cover from cover path: %s, %s", cover_path, pattern);

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
	{
		*(strrchr(pattern, '/') + 1) = '\0';
	}

	/* get cover from the music path */
	for(char *list = cfg_get_rule(), *buf = list; buf;)
	{
		_DEBUG("Ruelist: %s", list);
		buf = strstr_ln(buf, rule, sizeof rule, ";");

		if(buf == NULL)
		{
			free(list);
			break;
		}

		/* remove the separator ";" */
		*(rule + strlen(rule) - 1) = '\0';

		_DEBUG("Search cover from music path: %s, %s", pattern, rule);

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

/***

static unsigned long id3_realsize(const char *size)
{
	return size[0] << 21 | size[1] << 14 | size[2] << 7 | size[3];
}

***/

static char *getcover_id3v2(const char *uri)
{
	char *music_path, *cover_path, abspath[FILENAME_MAX], cover[FILENAME_MAX];
	FILE *fp = NULL;
	struct id3_header header;

	music_path = cfg_get_musicpath();
	cover_path = cfg_get_coverpath();

	snprintf(abspath, FILENAME_MAX, "%s%s", music_path, uri);
	snprintf(cover, FILENAME_MAX, "%s.id3v2/%s.cover", cover_path, strrchr(uri, '/') + 1);

	_INFO("try to get ID3V2 cover art from '%s'", abspath);

	if(access(cover, F_OK) == 0)
	{
		return strdup(cover);
	}

	fp = fopen(abspath, "rb");

	if(fp == NULL)
	{
		_ERROR("failed to open file '%s': %s", abspath, strerror(errno));

		goto out;
	}

	if(fread(&header, sizeof header, 1, fp) != 1)
	{
		_ERROR("failed to read ID3V2 header from '%s': %s", abspath, strerror(errno));

		goto out;
	}

	if(memcmp(header.header, "ID3", 3) != 0)
	{
		_WARN("failed to read ID3V2 header from '%s', header '%s'", abspath, header.header);

		goto out;
	}

	Imlib_Image image;
	
	if(image = imlib_load_image(abspath), image == NULL)
	{
		_WARN("failed to loaded image from '%s'", abspath);

		goto out;
	}

	imlib_context_set_image(image);
	imlib_save_image(cover);
	imlib_free_image();

	fclose(fp);
	free(music_path);
	free(cover_path);

	return strdup(cover);

	/***
	 
	void *data = NULL, *ptr = data;
	unsigned long data_size;

	data_size = id3_realsize(header.size);
	data = calloc(1, data_size);

	fseek(fp, sizeof header, SEEK_SET);
	
	if(fread(data, data_size, 1, fp) != 1)
	{
		_ERROR("failed to frame data: %s", strerror(errno));

		free(data);
		goto out;
	}

	while(1)
	{
		const struct id3_frame *frame = (const struct id3_frame *)data;
		unsigned long frame_size = id3_realsize(frame->size);

		if(*frame->id == '\0')
		{
			break;
		}

		if(frame_size >= data_size)
		{
			_ERROR("frame overflow");

			break;
		}

		if(memcmp((char *)frame->id, "APIC", 4) != 0)
		{
			data += 10 + frame_size;
			continue;
		}

		FILE *coverart = fopen(cover, "wb+");

		if(fwrite(data + 24, frame_size - 14, 1, coverart) != 1)
		{
			fclose(coverart);

			_ERROR("failed to write cover art: %s", strerror(errno));

			break;
		}

		fclose(coverart);

		free(ptr);
		fclose(fp);
		free(music_path);
		free(cover_path);

		return strdup(cover);
	}
	free(ptr);

	***/
out:
	fclose(fp);
	free(music_path);
	free(cover_path);

	return NULL;
}
