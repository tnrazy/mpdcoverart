/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef CONFIG_H
#define CONFIG_H

#define CFGNAME                                                     	".mpdcoverart"
#define CFGHOMENAME 							"mpdcoverart/mpdcoverart.conf"

#define CFG_MAP(XX) 							\
XX(LOGFILE,    		"log_file") 					\
XX(COVERPATH,  		"cover_path")                                   \
XX(MUSICPATH,  		"music_path")                                   \
XX(SKINPATH, 		"skin_path")  				    	\
XX(SKINNAME, 		"skin_name") 					\
XX(POSITION, 		"pos_xy") 					\
XX(POSITION_LOCK, 	"pos_lock") 					\
XX(RULE, 		"rule")

enum cfg_keys
{
    	#define XX(CFG_KEY, CFG_NAME)                          		CFG_##CFG_KEY,
	CFG_MAP(XX)
	#undef XX

    	CFG_UNKNOW
};

struct position 
{
	int x;
	int y;
};

void cfg_load(const char * filename);

void cfg_refresh();

const char *cfg_get_logfile();

const char *cfg_get_coverpath();

const char *cfg_get_musicpath();

const char *cfg_get_skinpath();

const char *cfg_get_skinname();

const char *cfg_get_rule();

struct position const *cfg_get_pos(struct position *pos);

unsigned int cfg_get_pos_lock();

void cfg_set_postion(const struct position *pos);

void cfg_set_postion_lock();

int cfg_get_debug();

void cfg_set_debug();

/* change skin */
void cfg_set_skinname(const char *skin_name);

#endif
