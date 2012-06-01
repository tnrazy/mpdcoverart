/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#define ___DEBUG

#include "list.h"
#include "msg.h"

#include <stdlib.h>
#include <string.h>

/* --- LINK LIST --- */

Dlist *Dlist_insert(Dlist *list, char *key_pos, char *key, Dlist_entity *entity)
{
    	if_null_return_null(list);
	if_null_return_null(key_pos);

	Dlist *pos = Dlist_find(list, key_pos);

	if(pos == NULL && key == NULL)
		return Dlist_add(list, key_pos, entity);

	else if(pos && key)
	{
		/* append this entity after the pos */
		return Dlist_add(pos, key, entity);
	}

	else if(pos)
	{
		/* update the entity of key_pos */
		if(entity)
		{
			pos->entity->des(pos->entity->data);
			free(pos->entity);
		}

		pos->entity = entity;
	}

	return list = pos;
}

Dlist *Dlist_new()
{
    	Dlist *list = calloc(sizeof(Dlist), 1);

	list->next = list;
	list->prev = list;

	return list;
}

Dlist_entity *Dlist_entity_new(void *data, size_t size, value_free des)
{
    	Dlist_entity *e = calloc(sizeof(Dlist_entity), 1);

	/* copy data */
	if(data && size <= 0)
		e->data = NULL;
	else
	{
		e->data = calloc(size + 1, 1);
		memcpy(e->data, data, size);
	}

	e->des = des ? des : free;

	return e;
}

Dlist_entity *Dlist_entity_get(Dlist *list, char *key)
{
	if_null_return_null(key);

	Dlist *res = Dlist_find(list, key);

	if(res && res->entity)
		return res->entity;

	return NULL;
}

void Dlist_free(Dlist *list)
{
    	if_null_return(list);

	Dlist *current = list, *tmp;
	int i = Dlist_get_size(list);

	if(i == 0)
	{
		/* not and element */
		free(list);
		return;
	}

	while(i-- > 0)
	{
		tmp = current->next;

		_DEBUG("free key: %s", current->key);

		free(current->key);

		if(current->entity)
		{
			current->entity->des(current->entity->data);
		}

		free(current->entity);
		free(current);

		/* point to the next */
		current = tmp;
	}
}

Dlist *Dlist_del(Dlist *list, char *key)
{
    	if_null_return_null(list);
	if_null_return_null(key);

	Dlist *del = Dlist_find(list, key), *next, *prev;

	/* key not exist, return the origin list */
	if(del == NULL)
		return list;

	/* not enough element to build a list, destroy the list, return null */
	if(Dlist_get_size(list) <= 1)
	{
		Dlist_free(list);
		return NULL;
	}

	/* retain the scene */
	next = del->next;
	prev = del->prev;

	/* clean this target and relink the list */
	free(del->key);
	del->entity->des(del->entity->data);
	free(del->entity);
	free(del);

	prev->next = next;
	next->prev = prev;

	return prev;
}

Dlist *Dlist_find(Dlist *list, char *key)
{
    	Dlist *current = list, *end = list->prev, *tmp;

	while(list && key)
	{
		tmp = current;

		/* list has 0 element */
		if(current->key == NULL)
			break;

		if(strcmp(key, current->key) == 0)
			return current;

		/* reach end of list */
		if(strcmp(current->key, end->key) == 0)
			break;

		current = tmp->next;
	}

	return NULL;
}

Dlist *Dlist_add(Dlist *list, char *key, Dlist_entity *entity)
{
    	if_null_return_null(list);
	if_null_return_null(key);

	/* if first been called */
	if(list->key == NULL)
	{
		list->key = strdup(key);
		list->entity = entity;
		return list;
	}

	/* the key is exists */
	if(Dlist_find(list, key))
	{
		if(entity)
		{
			_DEBUG("");
			/*
			 * should ensure that this entity was careated correctly
			 * */
			entity->des(entity->data);
			free(entity);
		}
		return list;
	}

	/* make a new node */
	Dlist *new = calloc(sizeof(Dlist), 1), *next = list->next;

	new->key = strdup(key);
	new->entity = entity;
	new->prev = list;
	new->next = next;

	/* relink */
	list->next = new;
	next->prev = new;

	return new;
}

int Dlist_get_size(Dlist *list)
{
    	if_null_return_null(list);

	int size = 0;
	Dlist *current = list, *end = list->prev;

	while(list->key)
	{
		++size;

		if(strcmp(current->key, end->key) == 0)
			break;

		current = current->next;
	}

	return size;
}

