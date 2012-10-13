/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#include "utils.h"

#include <stdio.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <dirent.h>
#include <pcre.h>
#include <ctype.h>
#include <sys/types.h>

static char to_hex(char ch);

static char from_hex(char ch);

static char **dir_search_p(const char *path, const char *pattern, int depth, char ***res, int *idx, int max_matched);

char *strstr_igcase(const char *str, const char *need, int contain_need)
{
	char *s = (char *)str, *n = (char *)need;
	int i, matched = 0;

	while(str)
	{
		/* whether reach end */
		if(*s == '\0' || *n == '\0')
		{
			break;
		}

		i = *s - *n;

		/* not matched */
		if(i != 0 && i != 'A' - 'a' && i != -('A' - 'a'))
		{            
			/* match the next character */
			++s;

			/* reset need string */
			n = (char *)need;

			/* set flag is not matched */
			matched = 0;

			continue;
		}

		/* has matched */
		++s;
		++n;

		++matched;
	}

	/* "aaaa;" find ";" will return 0 */
	return (matched && *n == '\0') ? (contain_need ? s - (n - need) : s) : NULL;
}

char *strstr_token(const char *src, const char *token, char *buf, size_t size)
{
	size_t len;
	char *offset;

	/* token not matched */
	if(offset = strstr_igcase(src, token, 0), offset == NULL)
	{
		return NULL;
	}

	len = offset - src;

	/* line too long */
	//if(len > size - 1)
	//{
	//	return NULL;
	//}

	while(len-- > 0 && --size > 0)
	{
		*buf++ = *src++;
	}

	*buf = '\0';

	return offset;
}

char *url_encode(const char *str)
{
    	char *s = calloc(strlen(str) * 3 + 1, 1), *ptr = s;

	while(*str)
	{
		if(*str == '-' || *str == '~' || *str == '_' || *str == '.'
		  /* isalnum */
		  || (*str >= 0 && *str <= 9)
		  || (*str >= 'a' && *str <= 'z')
		  || (*str >= 'A' && *str <= 'Z'))
			*s++ = *str;

		else if(*str == ' ')
		{
			*s++ = '+';
		}
		else
		{
			/* %H4L4 */
			*s++ = '%', *s++ = to_hex(*str >> 4), *s++ = to_hex(*str & 0XF);
		}

		++str;
	}

	*s = '\0';

	return ptr;
}

char *url_decode(const char *str)
{
    	char *s = calloc(strlen(str) + 1, 1), *ptr = s;

	while(*str)
	{
		if(*str == '%')
		{
			if(str[1] && str[2])
			{
				/* H4 | L4 */
				*s++ = from_hex(str[1]) << 4 | from_hex(str[2]);

				/* skip H4 and L4 */
				str += 2;
			}
		}
		else if(*str == '+')
		{
			*s++ = ' ';
		}

		else
		{
			*s++ = *str;
		}

		++str;
	}

	*s = '\0';

	return ptr;
}

char **dir_search(const char *path, const char *pattern, int depth, int max_matched)
{
	if(path == NULL || pattern == NULL)
	{
		return NULL;
	}

	char **res = calloc(sizeof *res, 1);
	int idx = 0;

	/* search depth 0111 */
	depth = depth & DIR_SEARCH_DEPTH;

	max_matched = max_matched < 0 ? 0 : max_matched;

	dir_search_p(path, pattern, depth, &res, &idx, max_matched);

	if(idx == 0)
	{
		/* free the shell */
		free(res);
		res = NULL;
	}

	return res;
}

/*int main(int argc, const char *argv[])*/
/*{*/
	/*char **res;*/

	/*res = dir_search("/etc/", "\\w*\\.conf$", 3, 5);*/


	/*for(char **list = res, *ptr = *list; ptr;)*/
	/*{*/
		/*printf("%s\n", ptr);*/
		/*ptr = *++list;*/
	/*}*/

	/*return 0;*/
/*}*/

char *clean_reg(char *str)
{
    	char *pattern, *tmp;

	pattern = calloc(strlen(str) * 3 + 1, 1);
	tmp = pattern;

	while(*str)
	{
		if(*str == ')' || *str == '(' || *str == '^' || *str == '-' ||
		  *str == '[' || *str == ']')
			*tmp++ = '\\';

		*tmp++ = *str;

		++str;
	}

	return pattern;
}

/*
 * name is a array
 * */
char *clean_name(char *name)
{
    	char *ptr;

	ptr = name;

	while(*ptr)
	{
		if(*ptr == '[' || *ptr == ']' || *ptr == '(' || *ptr == ')'
		  || *ptr == '_' || *ptr == '-' || *ptr == '.' || *ptr == '\"'
		  || *ptr == '\'' || *ptr == '+' || *ptr == '<' || *ptr == '>'
		  || *ptr == '{' || *ptr == '}' || *ptr == '&' || *ptr == '~')
		{
			*ptr++ = ' ';
			continue;
		}

		++ptr;
	}

	return name;
}

static char to_hex(char ch)
{
    	static char hex[] = "0123456789abcdef";
	return hex[ch & 15];
}

static char from_hex(char ch)
{
    	if(ch > 0 && ch < 9)
		return ch - '0';
	else
	{ 
		if(ch > 'A' && ch < 'Z')
			ch = ch + 'a' - 'A';
	}

	return ch - 'a' + 10;
}

static char **dir_search_p(const char *path, const char *pattern, int depth, char ***res, int *idx, int max_matched)
{
	pcre *reg;
	DIR *dir;
	
	int ovector[30];
	int erroffset;
	char *err;

	struct dirent *ptr;
	char *child_path = NULL;

	if(reg = pcre_compile(pattern, 0, (const char **)&err, &erroffset, NULL), reg == NULL)
	{
		//printf("PCRE compile error.");
		return NULL;
	}

	if(dir = opendir(path), dir == NULL)
		return NULL;

	while(ptr = readdir(dir), ptr != NULL)
	{
		/* just search the regular file */
		if(ptr->d_type == DT_REG)
		{
			if(max_matched != 0 && *idx == max_matched)
				{
					/* exit current stack */
					break;
				}

			if(pcre_exec(reg, NULL, ptr->d_name, strlen(ptr->d_name), 0, 0, ovector, sizeof ovector) > 0)
			{
				/* increase index, ++matched */
				++*idx;

				/* +1 for NULL */
				*res = realloc(*res, (*idx + 1) * sizeof **res);

				/* SA + EA */
				*(*res + *idx - 1) = calloc(strlen(path) + strlen(ptr->d_name) + 1, 1);

				/* copy matched result */
				sprintf(*(*res + *idx - 1), "%s%s", path, ptr->d_name);

				/* end of null */
				*(*res + *idx) = NULL;
			}
		}

		else if(ptr->d_type == DT_DIR && depth > 0)
		{
			/* skip '.' '..' */
			if(strcmp(ptr->d_name, ".") == 0 || strcmp(ptr->d_name, "..") == 0)
				continue;

			/* +2 /\0 */
			child_path = calloc(strlen(path) + strlen(ptr->d_name) + 2, 1);

			sprintf(child_path, "%s%s/", path, ptr->d_name);

			/* recursive search the child folder */
			dir_search_p(child_path, pattern, --depth, res, idx, max_matched);

			/* back to the caller stack, restore depth and reset the child_path is null, continue search */
			++depth;

			free(child_path);
			child_path = NULL;
		}
	}

	closedir(dir);
	free(reg);

	/* chain */
	return *res;
}

char *trim(char *str)
{
	char *s = NULL;

	while(isspace(*str++));

	char *left = --str;

	/* -1 '\0' */
	char *right = left + strlen(left) - 1;

	while(isspace(*right) && right > left)
	{
		*right-- = '\0';
	}

	s = left;

	return s;
}
