/*
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of the
 * License, or (at your option) any later version.
 *
 * tn.razy@gmail.com
 */

#ifndef UTILS_H
#define UTILS_H

#include <sys/select.h>
#include <errno.h>
#include <stdlib.h>

enum time_flag
{
    /* socket can be read */
    TIMEOUT_READ        = 1 << 0,

    /* socket can be write */
    TIMEOUT_WRITE       = 1 << 1,

    /* socket has an error */
    TIMEOUT_ERROR       = 1 << 2,

    /* all flag */
    TIMEOUT_ALL         = (1 << 3) - 1,

    /* unknow flag */
    TIMEOUT_UNKNOW
};

#define HAS_FLAG(flags, f)                          	((flags) & (f)) > 0
#define BEGIN_WITH(str, s)                          	( strncmp((str), (s), strlen((s))) == 0 )
#define END_WITH(str, s) 			  	( strncmp((str) + strlen((str)) - strlen((s)), (s), strlen((s))) == 0 )

/* time unit is a second, if connfd can be read or write return, if status is 0 is ok else errno */
#define timeout_start(connfd, timeout, flag, status)                                            \
{                                                                                               \
    if((flag <= 0 || flag >= TIMEOUT_UNKNOW))                                                   \
    {                                                                                           \
        _ERROR("Timeout flag is error.");                                                       \
        exit(1);                                                                                \
    }                                                                                           \
                                                                                                \
    *(status) = 1;                                                                              \
                                                                                                \
    fd_set r_fds, w_fds, e_fds;                                                                 \
    FD_ZERO(&r_fds);                                                                            \
    FD_ZERO(&w_fds);                                                                            \
    FD_ZERO(&e_fds);                                                                            \
                                                                                                \
    FD_SET((connfd), &r_fds);                                                                   \
    FD_SET((connfd), &w_fds);                                                                   \
    FD_SET((connfd), &e_fds);                                                                   \
                                                                                                \
    struct timeval tv = { (timeout), 0 };                                                       \
                                                                                                \
    switch(select((connfd) + 1,     HAS_FLAG((flag), TIMEOUT_READ)  ? &r_fds : NULL,            \
                                    HAS_FLAG((flag), TIMEOUT_WRITE) ? &w_fds : NULL,            \
                                    HAS_FLAG((flag), TIMEOUT_ERROR) ? &e_fds : NULL,            \
                                    &tv))                                                       \
    {                                                                                           \
        case -1:                                                                                \
            /* has an error occurrence, and return the errno */                                 \
            *(status) = errno;                                                                  \
            break;                                                                              \
        case 0:                                                                                 \
            /* operate timeout, return a signal of timeout */                                   \
            *(status) = 0;                                                                      \
            break;                                                                              \
            /* slow system call... */                                                           \
        default:

            /* ... */

#define timeout_end()                                                                           \
            /* end... */                                                                        \
    }                                                                                           \
}

#define if_null_return(value)                               if((value) == NULL)                                 \
                                                            {                                                   \
                                                                _ERROR(#value " is null, and return.");         \
                                                                return;                                         \
                                                            }

#define if_null_return_null(value)                          if((value) == NULL)                                 \
                                                            {                                                   \
                                                                _ERROR(#value " is null. return null.");        \
                                                                return 0;                                       \
                                                            }

char *strstr_igcase(const char *str, const char *need, int contain_need);

/* 
 * get a line end by token from the src, 
 * buf_len must be larger than line length
 * if arguments error or signal line return null, otherwise return the next line
 * */
char *strstr_token(const char *src, const char *token, char *buf, size_t size);

char *url_decode(const char *str);

char *url_encode(const char *str);

char *clean_reg(char *str);

/*
 * name is a array
 * */
char *clean_name(char *name);

/*
 * search depth 0-8
 * */
#define DIR_SEARCH_DEPTH	7

char **dir_search(const char *path, const char *pattern, int depth, int max_matched);

char *trim(char *str);

#endif
