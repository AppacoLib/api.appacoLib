/*
 * C CGI Library version 1.1
 *
 * Copyright 2009 Stephen C. Losen.  Distributed under the terms
 * of the GNU General Public License (GPL)
 */

#ifndef _CCGI_H
#define _CCGI_H

#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>


typedef struct CGI_val CGI_val;

struct CGI_val {
    CGI_val *next;        /* next entry on list */
    const char value[1];  /* variable value */
};

/*
 * CGI_varlist is an entry in a list of variables.  The fields
 * "iter" and "tail" are only used in the first list entry.
 */
typedef struct CGI_varlist CGI_varlist;
typedef const char * const CGI_value;

struct CGI_varlist
{
    CGI_varlist *next;      /* next entry on list */
    CGI_varlist *tail;      /* last entry on list */
    CGI_varlist *iter;      /* list iteration pointer */
    int numvalue;           /* number of values */
    CGI_val *value;         /* linked list of values */
    CGI_val *valtail;       /* last value on list */
    CGI_value *vector;      /* array of values */
    const char varname[1];  /* variable name */
};

CGI_varlist * findvar(CGI_varlist *v, const char *varname);
CGI_varlist * CGI_add_var(CGI_varlist *v, const char *varname, const char *value);
CGI_varlist *CGI_decode_query(CGI_varlist *v, const char *query);
CGI_varlist *CGI_get_query(CGI_varlist *v);
const char *CGI_lookup(CGI_varlist *v, const char *varname);
CGI_varlist *CGI_get_post(CGI_varlist *v);
CGI_varlist *CGI_get_all();

#endif
