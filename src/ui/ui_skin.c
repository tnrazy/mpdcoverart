/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "ui.h"
#include "log.h"
#include "utils.h"
#include "config.h"

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <libxml/parser.h>

static struct ui_skin *current_skin;

void ui_skin_entity_free(struct ui_skin_entity *ptr)
{
	if(ptr == NULL)
		return;

	if(ptr->skin_name)
		free(ptr->skin_name);

	if(ptr->skin_conf)
		free(ptr->skin_conf);

	free(ptr);
}

static char *ui_skin_read_string(xmlNode *root, char *node_name, char *attr_name, char **s);

static float ui_skin_read_float(xmlNode *root, char *node_name, char *attr_name, float *d);

static int ui_skin_read_integer(xmlNode *root, char *node_name, char *attr_name, int *i);

static struct ui_skin_label *ui_skin_label_read(xmlNode *root, char *node_name);

static struct ui_skin_image *ui_skin_image_read(xmlNode *root, char *node_name);

static void ui_skin_label_free(struct ui_skin_label *ptr);

static void ui_skin_image_free(struct ui_skin_image *ptr);

void ui_skin_free(struct ui_skin *ptr)
{
	if(ptr == NULL)
		return;

	if(ptr->name)
	{
		free(ptr->name);
	}

	if(ptr->albumcover)
	{
		if(ptr->albumcover->nocover)
		{
			free(ptr->albumcover->nocover);
		}

		free(ptr->albumcover);
	}

	ui_skin_image_free(ptr->img_case);
	ui_skin_image_free(ptr->img_next);
	ui_skin_image_free(ptr->img_pause);
	ui_skin_image_free(ptr->img_prev);

	ui_skin_label_free(ptr->title);
	ui_skin_label_free(ptr->artist);
	ui_skin_label_free(ptr->album);

	free(ptr);
}

static char *ui_skin_getattr(xmlNode *root, char *node_name, char *attr_name)
{
	char *value = NULL;

	for(xmlNode *cur_node = root; cur_node; cur_node = cur_node->next)
	{
		if(cur_node->type == XML_ELEMENT_NODE && !xmlStrcmp(cur_node->name, (const xmlChar *)node_name))
		{
			for(xmlAttr *cur_attr = cur_node->properties; cur_attr;)
			{
				if(!xmlStrcmp(cur_attr->name, (const xmlChar *)attr_name))
				{
					return (char *)xmlGetProp(cur_node, (const xmlChar *)attr_name);
				}

				cur_attr = cur_attr->next;
			}
		}

		/* whether has child node */
		value = ui_skin_getattr(cur_node->children, node_name, attr_name);

		if(value)
			return value;
	}

	return NULL;
}

struct ui_skin *ui_skin_load(const char *skin_name)
{
	xmlDoc *doc;
	xmlNode *root;

	char *work_path;
	struct ui_skin *a_skin;
	struct ui_skin_entity **skin_list, *entity;

	skin_list = ui_skin_load_all();

	for(struct ui_skin_entity **list = skin_list, *ptr = *list; ptr;)
	{
		if(strcmp(ptr->skin_name, skin_name) == 0)
		{
			entity = ptr;
		}
		else
			ui_skin_entity_free(ptr);

		ptr = *++list;
	}
	free(skin_list);

	doc = xmlReadFile(entity->skin_conf, 0, 0);

	if(doc == NULL)
	{
		die("Read skin.xml error.");
	}

	root = xmlDocGetRootElement(doc);

	if(root == NULL)
	{
		xmlFreeDoc(doc);

		die("Empty document.\n");
	}

	if(xmlStrcmp(root->name, (const xmlChar *)"skin"))
	{
		xmlFreeDoc(doc);

		die("Document wrong type.\n");
	}

	a_skin = calloc(sizeof *a_skin, 1);

	/* 
	 * node skin 
	 * */
	a_skin->name = ui_skin_read_string(root, "skin", "name", NULL);
	a_skin->width = ui_skin_read_integer(root, "skin", "width", NULL);
	a_skin->height = ui_skin_read_integer(root, "skin", "height", NULL);

	/*
	 * node albumcover
	 * */
	a_skin->albumcover = calloc(sizeof *a_skin->albumcover, 1);
	a_skin->albumcover->x = ui_skin_read_integer(root, "albumcover", "x", NULL);
	a_skin->albumcover->y = ui_skin_read_integer(root, "albumcover", "y", NULL);
	a_skin->albumcover->width = ui_skin_read_integer(root, "albumcover", "width", NULL);
	a_skin->albumcover->height = ui_skin_read_integer(root, "albumcover", "height", NULL);
	a_skin->albumcover->nocover = ui_skin_read_string(root, "albumcover", "nocover", NULL);

	/*
	 * node image
	 * */
	a_skin->img_case = ui_skin_image_read(root, "image");

	/*
	 * node titlename
	 * */
	a_skin->title = ui_skin_label_read(root, "titlename");

	/*
	 * node artistname
	 * */
	a_skin->artist = ui_skin_label_read(root, "artistname");

	/*
	 * node albumname
	 * */
	a_skin->album = ui_skin_label_read(root, "albumname");

	/* 
	 * node next
	 * */
	a_skin->img_next = ui_skin_image_read(root, "next");

	/*
	 * node prev
	 * */
	a_skin->img_prev = ui_skin_image_read(root, "prev");

	/*
	 * node pause
	 * */
	a_skin->img_pause = ui_skin_image_read(root, "pause");

	/*
	 * node play
	 * */
	a_skin->img_play = ui_skin_image_read(root, "play");

	xmlFreeDoc(doc);

	/* change the work path */
	work_path = calloc(strlen(entity->skin_conf) + 1, 1);
	sprintf(work_path, "%s", entity->skin_conf);
	*(strrchr(work_path, '/') + 1) = '\0';

	/* change process work path to skin path */
	chdir(work_path);
	
	free(work_path);
	
	/* load config is ok */
	ui_skin_entity_free(entity);

	ui_skin_free(current_skin);

	current_skin = a_skin;

	return a_skin;
}

struct ui_skin *ui_get_current_skin()
{
	if(current_skin)
		return current_skin;

	return ui_skin_load(DEF_SKIN);
}

struct ui_skin_entity **ui_skin_load_all()
{
	char *skin_path, *skin_name, **skins;
	struct ui_skin_entity **list = NULL;
	int idx = 0;

	xmlDoc *doc;
	xmlNode *root;

	skin_path = (char *)cfg_get_skinpath();

	skins = dir_search(skin_path, "^skin.xml$", 1, 0);

	if(skins == NULL)
	{
		die("Not exist skin file.\n");
	}
	
	for(char **res = skins, *ptr = *res; ptr;)
	{
		if(doc = xmlReadFile(ptr, 0, 0), doc)
		{
			root = xmlDocGetRootElement(doc);

			if(root == NULL)
			{
				xmlFreeDoc(doc);
				goto next;
			}

			skin_name = ui_skin_getattr(root, "skin", "name");

			if(skin_name == NULL)
			{
				xmlFreeDoc(doc);
				goto next;
			}

			if(list)
			{
				for(struct ui_skin_entity **skin_list = list, *skin = *skin_list; skin;)
				{
					if(strcmp(skin->skin_name, skin_name) == 0)
					{
						_WARN("Skin name has exist");
						xmlFreeDoc(doc);
						goto next;
					}

					skin = *++skin_list;
				}
			}

			if(list == NULL)
			{
				list = calloc(sizeof *list, 1 + 1);
			}
			else
				list = realloc(list, (idx + 1 + 1) * sizeof *list);

			*(list + idx) = calloc(sizeof **list, 1);
			( (struct ui_skin_entity *)*(list + idx) )->skin_name = strdup(skin_name);
			( (struct ui_skin_entity *)*(list + idx) )->skin_conf = strdup(ptr);

			*(list + idx + 1) = NULL;

			//printf("%s, %s\n", ((struct ui_skin_entity *)*(list + idx))->skin_name, ((struct ui_skin_entity *)*(list + idx))->skin_conf);

			xmlFreeDoc(doc);
			++idx;
		}

next:
		free(ptr);
		ptr = *++res;
	}

	/* free the shell */
	free(skins);

	return list;
}

static struct ui_skin_image *ui_skin_image_read(xmlNode *root, char *node_name)
{
	struct ui_skin_image *img;

	img = calloc(sizeof *img, 1);

	ui_skin_read_string(root, node_name, "src", &img->src);
	ui_skin_read_integer(root, node_name, "x", &img->x);
	ui_skin_read_integer(root, node_name, "y", &img->y);

	return img;
}

static struct ui_skin_label *ui_skin_label_read(xmlNode *root, char *node_name)
{
	struct ui_skin_label *label;

	label = calloc(sizeof *label, 1);

	ui_skin_read_string(root, node_name, "font", &label->font);
	ui_skin_read_string(root, node_name, "color", &label->color);
	ui_skin_read_integer(root, node_name, "x", &label->x);
	ui_skin_read_integer(root, node_name, "y", &label->y);
	ui_skin_read_integer(root, node_name, "maxchars", &label->max_char);
	ui_skin_read_integer(root, node_name, "enable", &label->enable);
	ui_skin_read_float(root, node_name, "align", &label->align);

	return label;
}

static int ui_skin_read_integer(xmlNode *root, char *node_name, char *attr_name, int *i)
{
	char *str;
	int value;

	ui_skin_read_string(root, node_name, attr_name, &str);

	value = atoi(str);
	free(str);

	if(i)
		*i = value;

	return value;
}

static float ui_skin_read_float(xmlNode *root, char *node_name, char *attr_name, float *d)
{
	char *str;
	float value;
	
	ui_skin_read_string(root, node_name, attr_name, &str);

	value = atof(str);
	free(str);

	if(d)
		*d = value;

	return value;
}

static char *ui_skin_read_string(xmlNode *root, char *node_name, char *attr_name, char **s)
{
	char *str;

	if(str = ui_skin_getattr(root, node_name, attr_name), str == NULL)
	{
		die("Parse skin error, Node(%s) not has attribute:%s", node_name, attr_name);
	}

	if(s)
		*s = strdup(str);

	return str;
}

static void ui_skin_label_free(struct ui_skin_label *ptr)
{
	if(ptr == NULL)
		return;

	if(ptr->color)
		free(ptr->color);

	if(ptr->font)
		free(ptr->font);

	free(ptr);
}

static void ui_skin_image_free(struct ui_skin_image *ptr)
{
	if(ptr == NULL)
		return;

	if(ptr->src)
		free(ptr->src);

	free(ptr);
}

/*int main(int argc, const char *argv[])*/
/*{*/
	/*skin_load();*/
	/*return 0;*/
/*}*/
