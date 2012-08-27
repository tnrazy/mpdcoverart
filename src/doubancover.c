/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#define ___DEBUG

#include "http.h"
#include "log.h"
#include "utils.h"

#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#include <regex.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/mman.h>

/*
 * not need cookie, result of query always none
 * */
//#define SEARCH_API                              "http://www.douban.com/search?search_text=%s"

/*
 * need cookie
 * */
#define SEARCH_API                              "http://music.douban.com/subject_search?search_text=%s"

/*
 * request of HEAD to host will return 403
 * */
#define COVERPATTERN                            "^img[0-9].douban.com/spic/s[0-9]+.jpg"
#define TIMEOUT                                 10

static int do_search(char *key, char **tmpfile);

static char *do_match(int fd, const char *pattern);

static char *do_fetch(const char *url, char *key);

char *doubancover(char *key, const char *filepath)
{
    	if_null_return_null(key);
	if_null_return_null(filepath);

	char *matched, *tmpfile, *filename, *fullname;
	int fd;

	if(fd = do_search(key, &tmpfile), fd < 0)
		return NULL;

	if(matched = do_match(fd, COVERPATTERN), matched == NULL)
	{
		_ERROR("No cover art has matched.");

		close(fd);

		if(unlink(tmpfile) < 0)
		{
			_ERROR("Remove temp file: %s error: %s", tmpfile, strerror(errno));
		}
		free(tmpfile);
		return NULL;
	}

	/* clean file */
	close(fd);

	_INFO("Matched: %s", matched);

	if(unlink(tmpfile) < 0)
	{
		_ERROR("Remove temp file: %s error: %s", tmpfile, strerror(errno));
	}
	free(tmpfile);

	/* save cover to the /filepath/key.xxx */
	filename = calloc(strlen(filepath) + strlen(key) + 1, 1);
	strcpy(filename, filepath);
	strcat(filename, key);

	/* download cover */
	if(fullname = do_fetch(matched, filename), fullname == NULL)
	{
		_ERROR("Download cover art error.");
	}

	free(matched);
	free(filename);

	return fullname;
}

static int do_search(char *key, char **tmpfile)
{
    	int fd;
	char tmpname[16], *url;

	/* html file name, length: 14'\0' */
	sprintf(tmpname, "/tmp/html_%d", (int)time(NULL) & 0XFFFF);

	key = url_encode(key);
	url = calloc(strlen(SEARCH_API) + strlen(key) + 1, 1);

	/* make url */
	sprintf(url, SEARCH_API, key);
	free(key);

	if(*tmpfile = http_getfile(url, tmpname, HTTP_COOKIE), *tmpfile == NULL)
	{
		_ERROR("Get file error.");

		free(url);
		return -1;
	}

	if(fd = open(*tmpfile, O_RDONLY, PERM), fd < 0)
	{
		_ERROR("Open file(%s) error: %s", *tmpfile, strerror(errno));
	}

	free(url);

	return fd;
}

static char *do_match(int fd, const char *pattern)
{
    	char *start, *walk, *matched = NULL;
	size_t offset;

	regex_t reg;
	regmatch_t match;


	lseek(fd, 0, SEEK_END);

	offset = lseek(fd, 0, SEEK_CUR);

	start = mmap(NULL, offset, PROT_READ | PROT_WRITE, MAP_PRIVATE, fd, 0);

	if(start == NULL)
	{
		_ERROR("Map file error: %s", strerror(errno));

		return NULL;
	}
	*(start + offset) = '\0';

	regcomp(&reg, pattern, REG_EXTENDED);

	walk = start;
	while(walk = strstr_igcase(walk, "<img src=\"http://", 0), walk)
	{
		/* match pattern */
		if(regexec(&reg, walk, 1, &match, 0) == 0)
		{
			/* has matched */
			matched = calloc(match.rm_eo - match.rm_so + 1, 1);
			/* copy string */
			strncpy(matched, walk + match.rm_so, match.rm_eo - match.rm_so);
			_DEBUG("Matched: %s", matched);

			//TODO: cover list
			break;
		}
		/* next */
		offset -= walk - start;
	}

	/* download cover from server img1 */
	if(matched && BEGIN_WITH(matched, "img3"))
	{
		*(matched + 3) = '1';
	}

	regfree(&reg);
	munmap(start, lseek(fd, 0, SEEK_CUR));

	return matched;
}

/*
 * replace 'spic' to 'lpic'
 * */
static char *do_fetch(const char *url, char *key)
{
    	char *lpic, *filename;

	lpic = calloc(strlen(url) + 1, 1);
	strcpy(lpic, url);

	*(strstr(lpic, "/spic") + 1) = 'l';

	filename = http_getfile(lpic, key, HTTP_NOCOOKIE);

	free(lpic);
	return filename;
}

