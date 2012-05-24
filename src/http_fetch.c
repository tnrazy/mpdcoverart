/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

//#define ___DEBUG

#include "http.h"
#include "msg.h"
#include "utils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>

static int http_do_fetch(int connfd, char *fullname, struct http_res *res);

char *http_fetch(int connfd, struct http_res *res, char *filename)
{
    	if_null_return_null(res);
	if_null_return_null(res->type);
	if_null_return_null(res->type->ext);
	if_null_return_null(filename);

	/* file name + extension name */
	char *fullname;

	if(connfd <= 0)
	{
		_ERROR("Connection error.");
		return NULL;
	}

	if(res->status_code != 200)
	{
		_ERROR("Error response status: %d", res->status_code);
		return NULL;
	}

	if(res->length <= 0)
	{
		_WARN("No data.");
		return NULL;
	}

	fullname = calloc(strlen(filename) + strlen(res->type->ext) + 1, 1);
	sprintf(fullname, "%s%s", filename, res->type->ext);

	//if(res->minor < 1)
	if(http_do_fetch(connfd, fullname, res) != 0 
	  			&& res->type->idx != HTTP_CXT_HTML)
	{
		_ERROR("Fetch data error.");


		if(unlink(fullname) == 0)
			_INFO("Remove the file: %s", fullname);
		else
			_ERROR("Remove file: %s, error.", fullname);

		free(fullname);
		return NULL;
	}

	return fullname;
}

static int http_do_fetch(int connfd, char *fullname, struct http_res *res)
{
    	char buf[1024];
	int fd, num_bytes, remain = res->length;

	/* remove the file mask */
	fd = open(fullname, O_CREAT | O_WRONLY, PERM);

	if((int)res->tail_len > 0)
	{
		if(num_bytes = write(fd, res->tail, res->tail_len), num_bytes != res->tail_len)
		{
			_ERROR("Write the tail to the file is error: %s", strerror(errno));

			return remain;
		}

		_DEBUG("Tail: %d, Write: %d", res->tail_len, num_bytes);
	}

	remain -= res->tail_len;

	while(1)
	{
		while(num_bytes = recv(connfd, buf, sizeof buf, 0), num_bytes < 0)
		{
			if(errno == EINTR)
			{
				_ERROR("EINTR");
				continue;
			}

			close(fd);
			_ERROR("Receive data error: %s", strerror(errno));
			return remain;
		}

		remain -= num_bytes;

		if(num_bytes == 0)
		{
			close(fd);
			break;
		}

		while(write(fd, buf, num_bytes) != num_bytes)
		{
			_ERROR("Write data error: %s", strerror(errno));
			close(fd);
			if(unlink(fullname) == 0)
				_INFO("Remove the file: %s", fullname);
			else
				_ERROR("Remove file: %s, error.", fullname);

			return remain;
		}

		//_INFO("%s, PROGRESS: %d%%", fullname, (int)(((double)total - remain) / total * 100));

		if(remain == 0)
			break;
	}

	return remain;
}
