/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

//#define ___DEBUG

#include "setting.h"
#include "utils.h"
#include "log.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <glib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/stat.h>

struct cfg
{
        int idx;
        char *cfggrp;
        char *cfgkey;
        void *value;
};

static struct cfg cfgs[] = 
{
        #define XX(VALUE, GRP_NAME, KEY_NAME)                               {.idx = VALUE, .cfggrp = STR(GRP_NAME), .cfgkey = KEY_NAME, .value = NULL},
        CFG_MAP(XX)
        #undef XX

        {.idx = -1, .cfggrp = NULL, .cfgkey = NULL, .value = NULL}
};

static GKeyFile *cfg_resolver();

static char *cfg_get(enum cfg_keys cfg_key);

static void cfg_set(enum cfg_keys, const char *value);

static int cfg_init;

static char *cfg_filename;

#define INIT_CHK()                                                          if(cfg_init) return;
#define INIT_OK()                                                           cfg_init = 1;

/* 
 * force refresh the conf file
 * */
void cfg_refresh()
{
	cfg_init = 0;
	cfg_load();
}

void cfg_load()
{
	INIT_CHK();

	char **rulekeys;
	char *tmp, *rulevalue = NULL;
	GKeyFile *cfgfile;

	/* load config file */
	if(cfgfile = cfg_resolver(), cfgfile == NULL)
	{
		die( "Load config file error.");
	}

	/* init config value */
	for(struct cfg *list = cfgs, *entity = list; entity;)
	{
		if(entity->idx == -1)
			break;

		/* get the rule list */
		if(strncmp(entity->cfgkey, "all", 3) == 0)
		{
			rulekeys = g_key_file_get_keys(cfgfile, entity->cfggrp, 0, 0);
			if(rulekeys == NULL)
			{
				g_key_file_free(cfgfile);
				die("Parse rule is error.");
			}

			for(char **keylist = rulekeys, *key = *keylist; key;)
			{
				tmp = g_key_file_get_string(cfgfile, entity->cfggrp, key, 0);

				tmp = tmp ? tmp : "";

				if(rulevalue == NULL)
				{
					rulevalue = calloc(strlen(tmp) + 2, 1);
				}
				else
					rulevalue = realloc(rulevalue, strlen(rulevalue) + strlen(tmp) + 2);

				strcat(rulevalue, tmp);
				strcat(rulevalue, ";");

				key = *++keylist;
			}

			entity->value = rulevalue;

			g_strfreev(rulekeys);
		}
		else
		{
			tmp = g_key_file_get_string(cfgfile, entity->cfggrp, entity->cfgkey, 0);

			if(tmp == NULL)
			{
				g_key_file_free(cfgfile);
				die("Parse confgile is error, GROUP: %s, KEY: %s", entity->cfggrp, entity->cfgkey);
			}

			_DEBUG("%s: %s", entity->cfggrp, tmp);
			entity->value = strdup(tmp);
		}

		entity = ++list;
	}

	g_key_file_free(cfgfile);

	_INFO("Load config file is ok.");

	INIT_OK();
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
    	cfg_load();

	if(cfg_key < 0 || cfg_key >= CFG_UNKNOW)
	{
		_ERROR("Unknow cfg_key: %s", cfg_key);
		return NULL;
	}

	return strdup(cfgs[cfg_key].value);
}

static void cfg_set(enum cfg_keys cfg_key, const char *value)
{
	if(cfg_key < 0 || cfg_key >= CFG_UNKNOW)
	{
		_ERROR("Unknow cfg_key: %s", cfg_key);
		return;
	}

	GKeyFile *cfgfile;
	unsigned length;
	char *data;

	/* load config file */
	if(cfgfile = cfg_resolver(), cfgfile == NULL)
	{
		die( "Load config file error.");
	}

	g_key_file_set_value(cfgfile, cfgs[cfg_key].cfggrp, cfgs[cfg_key].cfgkey, value);

	data = g_key_file_to_data(cfgfile, (gsize *)&length, NULL);

	if(data)
	{
		g_file_set_contents(cfg_filename, data, length, NULL);
		free(data);
	}

	g_key_file_free(cfgfile);
	cfg_refresh();
}

static GKeyFile *cfg_resolver()
{
	struct stat st_buf;
	GKeyFile *cfgfile = NULL;

	if(cfg_filename == NULL)
	{
		cfg_filename = path_real(getenv("HOME"), CFGNAME);
		if(stat(cfg_filename, &st_buf) == -1)
		{
			free(cfg_filename);

			cfg_filename = path_real(getenv("XDG_CONFIG_HOME"), CFGHOMENAME);
			if(stat(cfg_filename, &st_buf) == -1)
			{
				_ERROR("Config file is not exists.");

				free(cfg_filename);

				cfg_filename = NULL;
				return NULL;
			}
		}
	}

	cfgfile = g_key_file_new();

	if(g_key_file_load_from_file(cfgfile, cfg_filename, G_KEY_FILE_KEEP_TRANSLATIONS | G_KEY_FILE_KEEP_COMMENTS, NULL) == TRUE)
		return cfgfile;

	return cfgfile;
}

