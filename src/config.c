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
#include <pthread.h>

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

static char *cfg_get(enum cfg_keys cfg_key);

static void cfg_set(enum cfg_keys, const char *value);

static void cfg_check();

static int cfg_init;

#define INIT_CHK()                                                          if(cfg_init) return;
#define INIT_OK()                                                           cfg_init = 1;

static char *cfg_filename;

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

void cfg_load(char *filename)
{
	INIT_CHK();

	FILE *cfgfile;

	if(filename != NULL)
	{
		if(access(filename, F_OK | R_OK) == -1)
		{
			die("Config file '%s' not exists ot can not be read", filename);
		}

		/* save config file name */
		cfg_filename = filename;

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
		die("Failed to load config file '%s' ", cfg_filename);
	}

	/* init config value */
	char buf[512] = { 0 }, *line, *tmp;

	while(fgets(buf, sizeof buf, cfgfile))
	{
		_DEBUG("Line: %s\n", buf);

		line = trim(buf);

		/* skip comment line */
		if(BEGIN_WITH(line, "#"))
		{
			continue;
		}

		/* skip blank line */
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

char *cfg_get_logfile()
{
        char *value = cfg_get(CFG_LOGFILE), *ret;

        ret = path_wildcard(value);

        free(value);
        return ret;
}

char *cfg_get_coverpath()
{
	char *value = cfg_get(CFG_COVERPATH), *ret;

	ret = path_wildcard(value);

	free(value);

	return ret;
}

char *cfg_get_musicpath()
{
    	char *value = cfg_get(CFG_MUSICPATH), *ret;

	ret = path_wildcard(value);

	free(value);
	return ret;
}

char *cfg_get_skinpath()
{
	char *value = cfg_get(CFG_SKINPATH), *ret;

	ret = path_wildcard(value);

	free(value);
	return ret;
}

char *cfg_get_skinname()
{
	return cfg_get(CFG_SKINNAME);
}

unsigned int cfg_get_pos_x()
{
	char *value = cfg_get(CFG_POSITION);
	int x = atoi(value);

	free(value);
	return x;
}

unsigned int cfg_get_pos_y()
{
	char *value = cfg_get(CFG_POSITION);
	int y = atoi(strchr(value, ',') + 1);

	free(value);
	return y;
}

void cfg_set_postion(const char *xy)
{
	cfg_set(CFG_POSITION, xy);
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
	char *value = cfg_get(CFG_POSITION_LOCK);
	int lock = atoi(value);

	free(value);
	return lock;
}

char *cfg_get_rule()
{
    	return cfg_get(CFG_RULE);
}

static char *cfg_get(enum cfg_keys cfg_key)
{
    	cfg_load(cfg_filename);

	if(cfg_key < 0 || cfg_key >= CFG_UNKNOW)
	{
		_ERROR("Unknow cfg_key: %s", cfg_key);
		return NULL;
	}

	return strdup(cfgs[cfg_key].value);
}

static pthread_mutex_t cfg_lock = PTHREAD_MUTEX_INITIALIZER;

static void cfg_set(enum cfg_keys cfg_key, const char *value)
{
	if(cfg_key < 0 || cfg_key >= CFG_UNKNOW)
	{
		_ERROR("Unknow cfg_key: %s", cfg_key);
		return;
	}

	pthread_mutex_lock(&cfg_lock);

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

	pthread_mutex_unlock(&cfg_lock);

	cfg_refresh();
}

static FILE *cfg_resolver()
{
	FILE *cfgfile = NULL;

	if(cfg_filename)
	{
		cfgfile = fopen(cfg_filename, "r");

		return cfgfile;
	}

	cfg_filename = path_real(getenv("HOME"), CFGNAME);

	_DEBUG("Try load default config file: %s", cfg_filename);

	if(access(cfg_filename, F_OK | R_OK) == -1)
	{
		free(cfg_filename);

		cfg_filename = path_real(getenv("XDG_CONFIG_HOME"), CFGHOMENAME);

		_DEBUG("Try load default config file: %s", cfg_filename);

		if(access(cfg_filename, F_OK | R_OK) == -1)
		{
			free(cfg_filename);
			die("Failed to load default config file 'HOME/%s', '~/.config/%s'", CFGNAME, CFGHOMENAME);
		}
	}

	/* save config file name */
	cfgfile = fopen(cfg_filename, "r");

	return cfgfile;
}

static void cfg_check()
{
	char *logfile = cfg_get_logfile();

	if(logfile == NULL)
	{
		die("Log file is null.");
	}

	if(access(logfile, F_OK) == -1)
	{
		die("Log file '%s' not exists.", logfile);
	}

	if(access(logfile, W_OK) == -1)
	{
		die("Log file '%s' can not be write.");
	}

	free(logfile);

	char *coverpath = cfg_get_coverpath();

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

	free(coverpath);

	char *musicpath = cfg_get_musicpath();

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

	free(musicpath);

	char *skinpath = cfg_get_skinpath();

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

	free(skinpath);

	_INFO("Check config is ok");
}

