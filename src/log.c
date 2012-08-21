/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "log.h"
#include "config.h"

#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <stdio.h>

int msg_out[] =
{
    	/* 
     	* message out just stdout and stderror 
     	* 1: stdout
     	* 2: stderr
     	* */
    	#define XX(NAME, VALUE, FP, PRE_MSG, COLOR)             (FP & 1) + 1,
    	MSG_MAP(XX)
    	#undef XX

    	/* end */
    	0
};

char *msg_prefix[] =
{
    	#define XX(NAME, VALUE, FP, PRE_MSG, COLOR)             PRE_MSG,
    	MSG_MAP(XX)
    	#undef XX

    	NULL
};

int msg_color[] = 
{
    	#define XX(NAME, VALUE, FP, PRE_MSG, COLOR)             COLOR,
    	MSG_MAP(XX)
    	#undef XX

    	/* end */
    	0
};

void prmsg(enum msg_types msg_type, const char *format, ...)
{
	int daemon = !cfg_get_debug();

	/* in daemon skip debug message */
	if(daemon && msg_type == MSG_DEBUG)
	{
		return;
	}

	if(msg_type < 0)
	{
		msg_type = MSG_INFO;
	}
	
	if(msg_type > MSG_UNKNOW)
	{
		msg_type = MSG_ERROR;
	}

	char msg_buf[BUFSIZ];
	memset(&msg_buf, 0, BUFSIZ);

	va_list ap;
	va_start(ap, format);
	vsnprintf(msg_buf, sizeof msg_buf, format, ap);
	va_end(ap);

	___MSG(     msg_out[msg_type] == 2 ? stderr : stdout, 
	  		msg_prefix[msg_type],
	  		msg_buf, 
	  		msg_color[msg_type], 
	  		"");

	return;
}

void die(const char *format, ...)
{
	char buf[512] = {0};

	va_list ap;
	va_start(ap, format);
	vsnprintf(buf, sizeof buf, format, ap);
	va_end(ap);

	___MSG(     		stderr, 
	  			msg_prefix[MSG_ERROR],
	  			buf, 
	  			msg_color[MSG_ERROR], 
	  			"");

	exit(EXIT_FAILURE);
}
