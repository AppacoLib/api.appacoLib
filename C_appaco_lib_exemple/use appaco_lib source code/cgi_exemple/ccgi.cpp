
#include"ccgi.h"


/* mymalloc() is a malloc() wrapper that exits on failure */

static void * mymalloc(int size)
{
    void *ret = malloc(size);
    if (ret == 0) {
        fputs("C CGI Library out of memory\n", stderr);
        exit(1);
    }
    return ret;
}

/*
 * hex() returns the numeric value of hexadecimal digit "digit"
 * or returns -1 if "digit" is not a hexadecimal digit.
 */

static int hex(int digit)
{
    switch(digit) {

    case '0': case '1': case '2': case '3': case '4':
    case '5': case '6': case '7': case '8': case '9':
        return digit - '0';

    case 'A': case 'B': case 'C': case 'D': case 'E': case 'F':
        return 10 + digit - 'A';

    case 'a': case 'b': case 'c': case 'd': case 'e': case 'f':
        return 10 + digit - 'a';

    default:
        return -1;
    }
}


/*
 * CGI_add_var() adds a new variable name and value to variable list
 * "v" and returns the resulting list.  If "v" is null or if the
 * variable name is not on the list, then we create a new entry.
 * We add the value to the appropriate list entry.
 */

CGI_varlist * CGI_add_var(CGI_varlist *v, const char *varname, const char *value)
{
    CGI_val     *val;
    CGI_varlist *v2;

    if (varname == 0 || value == 0)
    {
        return v;
    }

    /* create a new value */

    val = (CGI_val *) mymalloc(sizeof(*val) + strlen(value));
    strcpy((char *) val->value, value);
    val->next = 0;

    /*
     * find the list entry or else create a new one.  Add the
     * new value.  We use "tail" pointers to keep the lists
     * in the same order as the input.
     */

    if ((v2 = findvar(v, varname)) == 0)
    {
        v2 = (CGI_varlist *) mymalloc(sizeof(*v2) + strlen(varname));
        strcpy((char *) v2->varname, varname);
        v2->value = val;
        v2->numvalue = 1;
        v2->next = v2->iter = v2->tail = 0;
        v2->vector = 0;
        if (v == 0) {
            v = v2;
        }
        else {
            v->tail->next = v2;
        }
        v->tail = v2;
    }
    else {
        v2->valtail->next = val;
        v2->numvalue++;
    }
    v2->valtail = val;
    if (v2->vector != 0) {
        free((void *)v2->vector);
        v2->vector = 0;
    }
    v->iter = 0;
    return v;
}


/*
 * CGI_decode_query() adds all the names and values in query string
 * "query" to variable list "v" (which may be null) and returns the
 * resulting variable list.  The query string has the form
 *
 * name1=value1&name2=value2&name3=value3
 *
 * We convert '+' to ' ' and convert %xx to the character whose
 * hex numeric value is xx.
 */

CGI_varlist *CGI_decode_query(CGI_varlist *v, const char *query)
{
    char *buf;
    const char *name, *value;
    int i, k, L, R, done;

    if (query == 0)
    {
        return v;
    }

    buf = (char *) mymalloc(strlen(query) + 1);
    name = value = 0;

    for (i = k = done = 0; done == 0; i++)
    {
        switch (query[i])
        {

        case '=':
            if (name != 0) {
                break;  /* treat extraneous '=' as data */
            }
            if (name == 0 && k > 0) {
                name = buf;
                buf[k++] = 0;
                value = buf + k;
            }
            continue;

        case 0:
            done = 1;  /* fall through */

        case '&':
            buf[k] = 0;
            if (name == 0 && k > 0) {
                name = buf;
                value = buf + k;
            }
            if (name != 0) {
                v = CGI_add_var(v, name, value);
            }
            k = 0;
            name = value = 0;
            continue;

        case '+':
            buf[k++] = ' ';
            continue;

        case '%':
            if ((L = hex(query[i + 1])) >= 0 &&
                (R = hex(query[i + 2])) >= 0)
            {
                buf[k++] = (L << 4) + R;
                i += 2;
                continue;
            }
            break;  /* treat extraneous '%' as data */
        }
        buf[k++] = query[i];
    }
    free(buf);
    return v;
}

/*
 * CGI_get_query() adds all the field names and values from the
 * environment variable QUERY_STRING to variable list "v" (which
 * may be null) and returns the resulting variable list.
 */

CGI_varlist *CGI_get_query(CGI_varlist *v)
 {
    return CGI_decode_query(v, getenv("QUERY_STRING"));
}


CGI_varlist * findvar(CGI_varlist *v, const char *varname)
{
    if (varname == 0 && v != 0)
    {
        return v->iter;
    }
    for (; v != 0; v = v->next)
    {
        if (v->varname[0] == varname[0] &&
            strcmp(v->varname, varname) == 0)
        {
            break;
        }
    }

    return v;
}


/*
 * CGI_lookup() searches variable list "v" for an entry named
 * "varname" and returns null if not found.  Otherwise we return the
 * first value associated with "varname", which is a null terminated
 * string.  If varname is null then we return the first value of the
 * "current entry", which was set using the iterating functions
 * CGI_first_name() and CGI_next_name().
 */

const char *CGI_lookup(CGI_varlist *v, const char *varname)
{
    return (v = findvar(v, varname)) == 0 ? 0 : v->value->value;
}

/*
 * CGI_get_post() reads field names and values from stdin and adds
 * them to variable list "v" (which may be null) and returns the
 * resulting variable list.  We accept input encoded as
 * "application/x-www-form-urlencoded" or as "multipart/form-data".
 * In the case of a file upload, we write to a new file created
 * with mkstemp() and "template".  If the template is null or if
 * mkstemp() fails then we silently discard the uploaded file data.
 * The local name of the file (created by mkstemp()) and the remote
 * name (as specified by the user) can be obtained with
 * CGI_lookup_all(v, fieldname).
 */

CGI_varlist *CGI_get_post(CGI_varlist *v)
{
    const char *env;
    char *buf;
    int len;

    if ((env = getenv("CONTENT_TYPE")) != 0 &&
        strcasecmp(env, "application/x-www-form-urlencoded") == 0 &&
        (env = getenv("CONTENT_LENGTH")) != 0 &&
        (len = atoi(env)) > 0)
    {
        buf = (char *) mymalloc(len + 1);
        if (fread(buf, 1, len, stdin) == len) {
            buf[len] = 0;
            v = CGI_decode_query(v, buf);
        }
        free(buf);
    }

    return v;
}

/*
 * CGI_get_all() returns a variable list that contains a combination of the
 * following: cookie names and values from HTTP_COOKIE, field names and
 * values from QUERY_STRING, and POSTed field names and values from stdin.
 * File uploads are handled using "template" (see CGI_get_post())
 */

CGI_varlist *CGI_get_all()
{
    CGI_varlist *v = 0;

    v = CGI_get_query(v);
    v = CGI_get_post(v);

    return v;
}
