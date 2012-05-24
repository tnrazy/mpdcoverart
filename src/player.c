/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */


#include "player.h"

#include <stdlib.h>

void player_music_info_free(struct player_music_info *ptr)
{
	if(ptr == NULL)
		return;

	if(ptr->artist)
		free(ptr->artist);

	if(ptr->title)
		free(ptr->title);

	if(ptr->album)
		free(ptr->album);

	if(ptr->uri)
		free(ptr->uri);

	free(ptr);
}
