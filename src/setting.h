/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef SETTING_H
#define SETTING_H

#define CFGNAME                                                     	".mpdcoverart"
#define CFGHOMENAME 							"mpdcoverart/conf"

#define CFG_MAP(XX) 							\
XX(0, LOGFILE,    	"log_file") 					\
XX(1, COVERPATH,  	"cover_path")                                   \
XX(2, MUSICPATH,  	"music_path")                                   \
XX(3, SKINPATH, 	"skin_path")  				    	\
XX(4, SKINNAME, 	"skin_name") 					\
XX(5, POSITION, 	"pos_xy") 					\
XX(6, POSITION_LOCK, 	"pos_lock") 					\
XX(7, RULE, 		"all")

enum cfg_keys
{
    	#define XX(VALUE, GRP_NAME, KEY_NAME)                          	CFG_##GRP_NAME,
	CFG_MAP(XX)
	#undef XX

    	CFG_UNKNOW
};

void cfg_load();

void cfg_refresh();

char *cfg_get_logfile();

char *cfg_get_coverpath();

char *cfg_get_musicpath();

char *cfg_get_skinpath();

char *cfg_get_skinname();

char *cfg_get_rule();

unsigned int cfg_get_pos_x();

unsigned int cfg_get_pos_y();

unsigned int cfg_get_pos_lock();

void cfg_set_postion(const char *xy);

void cfg_set_postion_lock();

/* change skin */
void cfg_set_skinname(const char *skin_name);

#endif
