/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef LIST_H
#define LIST_H

#include <sys/types.h>

/* 
 * destructor for free the value 
 * */
typedef void (*value_free)(void *);

typedef struct Dlist_entity
{
    void *data;
    value_free des;
} Dlist_entity;

typedef struct Dlist
{
    /* unique in the list */
    char *key;
    Dlist_entity *entity;

    struct Dlist *next;
    struct Dlist *prev;
} Dlist;



/* 
 * create a list
 * */
Dlist *Dlist_new();

Dlist_entity *Dlist_entity_new(void *value, size_t size, value_free des);

/*
 * wrapper of Dlist_find
 * */
Dlist_entity *Dlist_entity_get(Dlist *list, char *key);

/* 
 * destroy the list
 * */
void Dlist_free(Dlist *list);

/*
 * insert the key to the list after the key_pos
 * if key_pos not exist and key is null, add the key_pos to the list
 * */
Dlist *Dlist_insert(Dlist *list, char *key_pos, char *key, Dlist_entity *entity);

/* 
 * delete the key from the list
 * */
Dlist *Dlist_del(Dlist *list, char *key);

/*
 * find the key from the list, not found return null
 * */
Dlist *Dlist_find(Dlist *list, char *key);

/* 
 * add the key to the list, if key is exist free the value
 * */
Dlist *Dlist_add(Dlist *list, char *key, Dlist_entity *entity);

/* 
 * get list size
 * */
int Dlist_get_size(Dlist *list);

#endif
