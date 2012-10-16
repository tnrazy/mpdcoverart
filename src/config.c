/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#define ___DEBUG

#include "config.h"
#include "utils.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/stat.h>

struct cfg
{
        enum cfg_keys cfgkey;
        char *name;
        char *value;
};

static struct cfg cfgs[] = 
{
        #define XX(CFG_KEY, CFG_NAME)                               {.cfgkey = CFG_##CFG_KEY, .name = CFG_NAME, .value = NULL},
        CFG_MAP(XX)
        #undef XX

        {.cfgkey = CFG_UNKNOW, .name = NULL, .value = NULL}
};

static FILE *cfg_resolver();

static const char *cfg_get(enum cfg_keys cfg_key);

static void cfg_set(enum cfg_keys, const char *value);

static void cfg_check();

static int cfg_init;

#define INIT_CHK()                                                          if(cfg_init) return;
#define INIT_OK()                                                           cfg_init = 1;

static char cfg_filename[FILENAME_MAX] = { 0 };

static int debug;

int cfg_get_debug()
{
	return debug;
}

void cfg_set_debug()
{
	debug = (~debug & 1);
}

/* 
 * force refresh the conf file
 * */
void cfg_refresh()
{
	cfg_init = 0;
	cfg_load(cfg_filename);
}

void cfg_load(const char *filename)
{
	INIT_CHK();

	FILE *cfgfile = NULL;

	if(filename != NULL)
	{
		_DEBUG("%s", filename);

		if(access(filename, F_OK | R_OK) == -1)
		{
			die("Config file '%s' not exists or can not be read", filename);
		}

		/* save config file name */
		strncpy(cfg_filename, filename, FILENAME_MAX);

		cfgfile = fopen(filename, "r");
	}
	else
	{
		/* load default config file */
		cfgfile = cfg_resolver();
	}

	if(cfgfile == NULL)
	{
		/* fopen() error */
		die("Failed to load config file '%s'", filename);
	}

	/* init config value */
	char buf[512] = { 0 }, *line, *tmp;

	while(fgets(buf, sizeof buf, cfgfile))
	{
		_DEBUG("Line: %s\n", buf);

		line = trim(buf);

		/* skip comment */
		if(BEGIN_WITH(line, "#"))
		{
			continue;
		}

		/* skip blank */
		if(strcmp(line, "") == 0)
		{
			continue;
		}

		for(struct cfg *list = cfgs, *entity = list; entity;)
		{
			if(entity->cfgkey == CFG_UNKNOW)
			{
				fclose(cfgfile);
				die("Unknow config line: %s", line);
			}

			if(BEGIN_WITH(line, entity->name))
			{
				tmp = strstr(line, "=") + 1;

				entity->value = trim(strdup(tmp));

				_INFO("%s=%s", entity->name, entity->value);

				break;
			}

			entity = ++list;
		}
	}

	fclose(cfgfile);

	INIT_OK();

	cfg_check();

	_INFO("Load config file '%s' is ok.", cfg_filename);
}

const char *cfg_get_logfile()
{
        return cfg_get(CFG_LOGFILE);
}

const char *cfg_get_coverpath()
{
	const char *value = cfg_get(CFG_COVERPATH);

	return value;
}

const char *cfg_get_musicpath()
{
    	return cfg_get(CFG_MUSICPATH);
}

const char *cfg_get_skinpath()
{
	return cfg_get(CFG_SKINPATH);
}

const char *cfg_get_skinname()
{
	return cfg_get(CFG_SKINNAME);
}

struct position const *cfg_get_pos(struct position *pos)
{
	const char *value = cfg_get(CFG_POSITION);

	pos->x = atoi(value);
	pos->y = atoi(strchr(value, ',') + 1);

	return pos;
}

void cfg_set_postion(const struct position *pos)
{
	char str[32];
	snprintf(str, sizeof str, "%d,%d", pos->x, pos->y);
	cfg_set(CFG_POSITION, str);
}

void cfg_set_postion_lock()
{
	char format[2];
	snprintf( format, 2, "%d", (~cfg_get_pos_lock() & 1) );

	cfg_set(CFG_POSITION_LOCK, format);
}

void cfg_set_skinname(const char *skin_name)
{
	cfg_set(CFG_SKINNAME, skin_name);
}

unsigned int cfg_get_pos_lock()
{
	const char *value = cfg_get(CFG_POSITION_LOCK);

	int lock = atoi(value);

	return lock;
}

const char *cfg_get_rule()
{
    	return cfg_get(CFG_RULE);
}

static const char *cfg_get(enum cfg_keys cfg_key)
{
    	cfg_load(cfg_filename);

	if(cfg_key < 0 || cfg_key >= CFG_UNKNOW)
	{
		_ERROR("Unknow cfg_key: %s", cfg_key);
		return NULL;
	}

	return cfgs[cfg_key].value;
}

static void cfg_set(enum cfg_keys cfg_key, const char *value)
{
	FILE *cfgfile = fopen(cfg_filename, "w");

	_DEBUG("Config file: %s, %d", cfg_filename, cfgfile == NULL);

	char line[512] = { 0 };

	for(struct cfg *list = cfgs,*entity = list; ;)
	{
		if(entity->cfgkey == CFG_UNKNOW)
		{
			break;
		}

		if(entity->cfgkey == cfg_key)
		{
			entity = ++list;

			continue;
		}

		snprintf(line, sizeof line, "%s = %s\n\n", entity->name, entity->value);

		_DEBUG("Write config line: %s", line);

		if(fwrite(line, strlen(line), 1, cfgfile) < 0)
		{
			_ERROR("Failed to write config: %s", strerror(errno));
		}

		entity = ++list;
	}

	snprintf(line, sizeof line, "%s = %s\n\n", cfgs[cfg_key].name, value);

	_DEBUG("Write config line: %s", line);

	fwrite(line, strlen(line), 1, cfgfile);

	fclose(cfgfile);

	cfg_refresh();
}

static FILE *cfg_resolver()
{
	snprintf(cfg_filename, FILENAME_MAX, "%s/%s", getenv("HOME"), CFGNAME);

	_DEBUG("Try load default config file: %s", cfg_filename);

	if(access(cfg_filename, F_OK | R_OK) == -1)
	{
		snprintf(cfg_filename, FILENAME_MAX, "%s/%s", getenv("XDG_CONFIG_HOME"), CFGHOMENAME);

		_DEBUG("Try load default config file: %s", cfg_filename);

		if(access(cfg_filename, F_OK | R_OK) == -1)
		{
			die("Failed to load default config file 'HOME/%s', '~/.config/%s'", CFGNAME, CFGHOMENAME);
		}
	}

	return fopen(cfg_filename, "r");
}

static void cfg_check()
{
	const char *logfile = cfg_get_logfile();
	char id3v2_dir[FILENAME_MAX];

	if(logfile == NULL)
	{
		die("Log file is null.");
	}

	const char *coverpath = cfg_get_coverpath();

	if(coverpath == NULL)
	{
		die("Cover path is null.");
	}

	if(access(coverpath, F_OK) == -1)
	{
		die("Cover path '%s' is not exists.", coverpath);
	}

	if(access(coverpath, X_OK | W_OK) == -1)
	{
		die("Cover path '%s' can not be write.", coverpath);
	}

	snprintf(id3v2_dir, FILENAME_MAX, "%s.id3v2", coverpath);
	if(access(id3v2_dir, F_OK) == -1)
	{
		mkdir(id3v2_dir, 0766);
	}

	const char *musicpath = cfg_get_musicpath();

	if(musicpath == NULL)
	{
		die("Music path is null.");
	}

	if(access(musicpath, F_OK) == -1)
	{
		die("Music path '%s' not exists.", musicpath);
	}

	if(access(musicpath, X_OK | R_OK) == -1)
	{
		die("Music path '%s' can't be search.");
	}

	const char *skinpath = cfg_get_skinpath();

	if(skinpath == NULL)
	{
		die("Skin path is null.");
	}

	if(access(skinpath, F_OK) == -1)
	{
		die("Skin path '%s' not exists.", skinpath);
	}

	if(access(skinpath, X_OK | R_OK) == -1)
	{
		die("Skin path '%s' can not be search.");
	}

	_INFO("Check config is ok");
}

