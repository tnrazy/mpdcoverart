/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef COVERFETCH_H
#define COVERFETCH_H

#define DEF_COVER_NAME 						"folder.jpg"

typedef char *(*coverfetch)(char *, const char *);

char *doubancover(char *key, const char *filepath);

char *getcover(const char *uri, const char *artist, const char *title, const char *album, coverfetch fetch);

#endif
