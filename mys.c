// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// mariadb native interface, used by db.c for making of db-specific libs
//
// All errors must go through vely_maria_error() and the sole point of entry is vely_maria_exec()
// in order to avoid disorderly rollbacks in case of errors
//

#include "vely.h"
static num vely_maria_stmt_exec();
static int vely_maria_stmt_rows (char ***row, unsigned long **lens);
static num vely_maria_stmt_exec();
static void vely_maria_add_input(num i, char *arg);
static void vely_maria_bind_input ();
static int vely_maria_prep_stmt(void **prep, char *stmt, num num_of_args);

static char *cerror = NULL;


num vely_maria_checkc()
{
    VV_TRACE("");
    return (mysql_ping (VV_CURR_DB.dbc->maria.con) == 0 ? 1 : 0);
}

char *vely_maria_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er, char is_prep)
{
    VV_TRACE("");
    if (is_prep == 0)
    {
        snprintf(errm,errmsize,"Error during query [%s], additional [%s] file [%s], line [%lld] : [%s]%s", s, cerror==NULL?"":cerror, sname, lnum, er ,atol(er) == ER_PARSE_ERROR ?  "Problem with parsing SQL statement" : mysql_error(VV_CURR_DB.dbc->maria.con));
        return errm;
    }
    else
    {
        snprintf(errm,errmsize,"Error during query [%s], additional [%s] file [%s], line [%lld] : [%s]%s", s, cerror==NULL?"":cerror, sname, lnum, er ,atol(er) == ER_PARSE_ERROR ?  "Problem with parsing SQL statement" : mysql_stmt_error(VV_CURR_DB.dbc->maria.stmt));
        return errm;
    }
}


char *vely_maria_error(char *s, char is_prep)
{
    VV_TRACE("");
    VV_UNUSED(s); // used in tracing only
    if (is_prep == 0)
    {
        static char errv[30];
        snprintf (errv, sizeof(errv), "%d", mysql_errno(VV_CURR_DB.dbc->maria.con));
        VV_TRACE ("Error in %s: %s error %s state %s", s, mysql_error(VV_CURR_DB.dbc->maria.con), errv, mysql_sqlstate(VV_CURR_DB.dbc->maria.con));
        return errv;
    }
    else
    {
        static char errv[30];
        snprintf (errv, sizeof(errv), "%d", mysql_stmt_errno(VV_CURR_DB.dbc->maria.stmt));
        VV_TRACE ("Error in %s: %s error %s state %s", s, mysql_stmt_error(VV_CURR_DB.dbc->maria.stmt), errv, mysql_stmt_sqlstate(VV_CURR_DB.dbc->maria.stmt));
        return errv;
    }
}


int vely_maria_rows (char ***row, unsigned long **lens, char is_prep)
{
    VV_TRACE("");
    if (is_prep == 0)
    {
        VV_CURR_DB.need_copy = 1;
        // fetch a row, field by field
        MYSQL_ROW row1; // this is char **, if that changes than code here must be refactored, or made to use
                      // that new structure
        row1 = mysql_fetch_row(VV_CURR_DB.dbc->maria.res);
        *row = row1; // simple now, may not be in the future. In any case, the returning row variable must
                    // be an array of strings
        *lens = mysql_fetch_lengths (VV_CURR_DB.dbc->maria.res);
    }
    else
    {
        VV_CURR_DB.need_copy = 0;
        if (vely_maria_stmt_rows (row, lens)) return 1;
    }
    return 0;
}


//
// Get a row when tuples returned. *row[] is an array of strings which represent fields (columns) and *lens is the
// array of lengths of those fields.
//
int vely_maria_stmt_rows (char ***row, unsigned long **lens)
{
    VV_TRACE("");
    char *sname = "";
    num lnum = 0;
    // get which file and line number is this going on at
    vely_location (&sname, &lnum, 0);

    // number of fields in a row
    num nf = mysql_num_fields(VV_CURR_DB.dbc->maria.res);
    num i;

    // get result binding array
    VV_CURR_DB.dbc->maria.bindout = (MYSQL_BIND*)vely_calloc(nf, sizeof(MYSQL_BIND));

    // get results (columns, lengths)
    *row = (char **)vely_calloc (nf, sizeof(char*));
    *lens = (unsigned long *)vely_calloc (nf, sizeof(unsigned long));

    // rlen is the array of actual lengths of an array of fields in a row, obtained from mariadb fetching
    unsigned long *rlen = (unsigned long*)vely_calloc (nf, sizeof(unsigned long));
    // array of whether fields are NULL
    my_bool *isnull = (char*)vely_calloc (nf, sizeof(char));
    // dummy storage for first pass of fetching
    char dummy[2]; // just in case give it storage
    // set all cols to strings, unknown length

    // set binding for results, we want all as strings. Length is 0 so we can obtain actual lengths in second pass.
    for (i = 0; i < nf; i++) 
    {
        VV_CURR_DB.dbc->maria.bindout[i].buffer_type = MYSQL_TYPE_STRING;
        VV_CURR_DB.dbc->maria.bindout[i].buffer = &(dummy[0]);
        VV_CURR_DB.dbc->maria.bindout[i].buffer_length = 0;
        VV_CURR_DB.dbc->maria.bindout[i].is_null = &(isnull[i]);
        VV_CURR_DB.dbc->maria.bindout[i].length = &(rlen[i]);
    }

    // bind results
    if (mysql_stmt_bind_result(VV_CURR_DB.dbc->maria.stmt, VV_CURR_DB.dbc->maria.bindout) != 0)
    {
        cerror = "Cannot initialize statement";
        return 1;
    }
    num cdata = 0; // index of fields within a row

    //
    // Get a single row
    //
    // set empty for each row, because we don't know the size of its columns and each must be separately allocated
    for (i = 0; i < nf; i++) 
    {
        VV_CURR_DB.dbc->maria.bindout[i].buffer = &(dummy[0]);
        VV_CURR_DB.dbc->maria.bindout[i].buffer_length = 0;
    }
    // fetch a row
    int res = mysql_stmt_fetch(VV_CURR_DB.dbc->maria.stmt);
    // row is either okay or truncated, if not, error
    if (res != 0 && res != MYSQL_DATA_TRUNCATED) 
    {
        cerror = "Cannot fetch row";
        return 1;
    }
    if (res == MYSQL_DATA_TRUNCATED) 
    {
        // if truncated, now we can get actual lengths and allocate them
        for (i = 0; i < nf; i++) 
        {
            VV_CURR_DB.dbc->maria.bindout[i].buffer_length = rlen[i]+1;
            VV_CURR_DB.dbc->maria.bindout[i].buffer = vely_malloc (rlen[i] + 1 + 1); // just in case extra byte of storage
        }
        // now fetch columns based on actual lengths for each field
        for (i = 0; i < nf; i++) 
        {
            if (mysql_stmt_fetch_column(VV_CURR_DB.dbc->maria.stmt, &(VV_CURR_DB.dbc->maria.bindout[i]), i, 0) != 0)
            {
                cerror = "Cannot fetch data for column";
                return 1;
            }
        }
    }
    // present row data for caller, wheter it's truncated or not. The only way it's not truncated is if all columns are NULL
    for (i = 0; i < nf; i++) 
    {
        // use EMPTY_STRING if null, so freeing works
        (*row)[cdata] = VV_CURR_DB.dbc->maria.bindout[i].buffer == NULL ? VV_EMPTY_STRING : VV_CURR_DB.dbc->maria.bindout[i].buffer;
        (*lens)[cdata] = *(VV_CURR_DB.dbc->maria.bindout[i].length);
        cdata++;
    }

    // free allocations
    vely_free (VV_CURR_DB.dbc->maria.bindout);
    vely_free (rlen);
    vely_free (isnull);
    return 0;
}


num vely_maria_nrows(char is_prep)
{
    VV_TRACE("");
    if (is_prep == 0)
    {
        // works since we use buffered (store) method to get results
        return mysql_num_rows (VV_CURR_DB.dbc->maria.res);
    }
    else
    {
        return mysql_stmt_num_rows (VV_CURR_DB.dbc->maria.stmt);
    }
}

void vely_maria_free()
{
    VV_TRACE("");
    if (VV_CURR_DB.dbc->maria.res != NULL) mysql_free_result(VV_CURR_DB.dbc->maria.res);
    VV_CURR_DB.dbc->maria.res = NULL;
}

char *vely_maria_fieldname()
{
    VV_TRACE("");
    return mysql_fetch_field(VV_CURR_DB.dbc->maria.res)->name;
}

num vely_maria_nfield()
{
    VV_TRACE("");
    return mysql_num_fields(VV_CURR_DB.dbc->maria.res);
}

int vely_maria_use(char is_prep)
{
    VV_TRACE("");
    if (is_prep == 0)
    {
        VV_CURR_DB.dbc->maria.res = mysql_use_result(VV_CURR_DB.dbc->maria.con);
        if (VV_CURR_DB.dbc->maria.res == NULL)
        {
            cerror = "Error storing obtained data";
            return 1;
        }
    }
    else 
    {
        VV_CURR_DB.dbc->maria.res = mysql_stmt_result_metadata(VV_CURR_DB.dbc->maria.stmt);
        if (VV_CURR_DB.dbc->maria.res == NULL)
        {
            cerror = "Error storing obtained data (1)";
            return 1;
        }
    }
    return 0;
}

int vely_maria_store(char is_prep)
{
    VV_TRACE("");
    if (is_prep == 0)
    {
        VV_CURR_DB.dbc->maria.res = mysql_store_result(VV_CURR_DB.dbc->maria.con);
        if (VV_CURR_DB.dbc->maria.res == NULL)
        {
            cerror = "Error storing obtained data (2)";
            return 1;
        }
    }
    else
    {
        vely_maria_use (is_prep); // get metadata first
        my_bool update= 1;
        // get lengths
        mysql_stmt_attr_set(VV_CURR_DB.dbc->maria.stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &update);
        // get result to copy to client
        if (mysql_stmt_store_result (VV_CURR_DB.dbc->maria.stmt) != 0)
        {
            cerror = "Error storing obtained data (3)";
            return 1;
        }
    }
    return 0;
}


void vely_maria_close ()
{
    VV_TRACE("");
    mysql_close (VV_CURR_DB.dbc->maria.con);
}


vely_dbc *vely_maria_connect (num abort_if_bad)
{
    VV_TRACE("");
    // reset all prepared statements
    vely_db_prep (NULL);

    // allocate connection to database. Must be done here to account for the actual generated code, which
    // dictates the sizeof(vely_dbc)
    // This must be malloc and NOT vely_malloc since we want to reuse connection across connections.
    // Otherwise vely_malloc would be automatically freed when the request is done, and the next 
    // request would use an invalid pointer. Also must use free to free it, not vely_free.
    if ((VV_CURR_DB.dbc = malloc (sizeof (vely_dbc))) == NULL) VV_FATAL ("Cannot allocate memory for database connection [%m]");

    VV_CURR_DB.dbc->maria.con = mysql_init(NULL);
    VV_CURR_DB.dbc->maria.res = NULL;
    VV_CURR_DB.dbc->maria.bind = NULL;
    VV_CURR_DB.dbc->maria.bindout = NULL;
   
    if (VV_CURR_DB.dbc->maria.con == NULL) 
    {
        char *em = "Cannot initialize database connection";
        VV_TRACE ("%s", em);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        vely_end_connection (0);
        return NULL; 
    }  

    char db_config_name[150];
    snprintf (db_config_name, sizeof(db_config_name), "%s/%s", vely_get_config()->app.dbconf_dir, VV_CURR_DB.db_name);
    VV_TRACE ("Using db config file [%s]", db_config_name);
    mysql_optionsv (VV_CURR_DB.dbc->maria.con, MYSQL_READ_DEFAULT_FILE, db_config_name);

    VV_TRACE ("Logging in to database, config [%s]", db_config_name);
    if (mysql_real_connect(VV_CURR_DB.dbc->maria.con, NULL, NULL, NULL, 
                   NULL, 0, NULL, 0) == NULL) 
    {
        char em[300];
        snprintf (em, sizeof(em), "Error in logging in to database: error [%s], using config file [%s]", mysql_error(VV_CURR_DB.dbc->maria.con), db_config_name);
        VV_TRACE ("%s", em);
        if (abort_if_bad == 1) vely_report_error ("%s",em);
        vely_end_connection (0);
        return NULL;
    }    
    //
    // These are the most common settings. ANSI_QUOTES means that single quotes
    // are used for string and defense agains SQL injection depends on this (could be done
    // either way or for both, but the odds are ansi_quotes is used anyway). UTF8 is used for
    // web communication.
    // So in short, do NOT change either one of these settings!
    //
    if (mysql_set_character_set(VV_CURR_DB.dbc->maria.con, "utf8"))
    {
        char *em = "Cannot set character set to utf8";
        VV_TRACE ("%s", em);
        vely_end_connection (1);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        return NULL;
    }

    if (mysql_query(VV_CURR_DB.dbc->maria.con, "set session sql_mode=ansi_quotes")) 
    {
        char *em = "Cannot set sql_mode to ansi_quotes";
        VV_TRACE ("%s", em);
        vely_end_connection (1);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        return NULL;
    }
    return VV_CURR_DB.dbc;
}

num vely_maria_exec(char *s, char is_prep, void **prep, num paramcount, char **params)
{   
    VV_TRACE("");
    vely_stmt_cached = 0;
    if (is_prep == 0)
    {
        return (mysql_query(VV_CURR_DB.dbc->maria.con, s));
    }
    else
    {
        if (vely_maria_prep_stmt (prep, s, paramcount)) return 1;
        num i;
        for (i = 0; i < paramcount; i++)
        {
            vely_maria_add_input(i, params[i]);
        }
        vely_maria_bind_input ();
        return vely_maria_stmt_exec();
    }
}


//
// deallocate prepared statement (only when connection lost), st is void * to prep stmt.
//
void vely_maria_close_stmt (void *st)
{
    VV_TRACE("");
    if (st == NULL) return; // statement has not been prepared yet, so cannot deallocate
    if (VV_CURR_DB.dbc != NULL) 
    {
        // we only close statements when connection is lost, so we don't care about return code
        // only that memory is deallocated
        mysql_stmt_close((MYSQL_STMT *)st);
    }
}

//
// Prepare statement with source test stmt and num_of_args input parameters
// prep is the address of void * that points to prepared statement. This is a static
// void *, which survives requests (otherwise prepared statements wouldn't be very useful,
// actually would decrease performance), however prep is set to NULL when connection is 
// reestablished (typically if db server recycles), which is generally rare.
//
int vely_maria_prep_stmt(void **prep, char *stmt, num num_of_args)
{
    VV_TRACE("");
    char *sname = "";
    num lnum = 0;
    vely_location (&sname, &lnum, 0);

    // reuse prepared statement from prep
    if (*prep != NULL) 
    {
        vely_stmt_cached = 1;
        VV_TRACE ("reusing prepared statement");
        VV_CURR_DB.dbc->maria.stmt = (MYSQL_STMT*)*prep;
    }
    else
    {
        VV_TRACE ("creating prepared statement");
        // if prep is NULL, create prepared statement
        char *origs = stmt; // original stmt
        stmt = vely_db_prep_text(stmt); // make ? instead of '%s'
        // init prep stmt
        if ((VV_CURR_DB.dbc->maria.stmt = mysql_stmt_init (VV_CURR_DB.dbc->maria.con)) == NULL)
        {
            cerror = "Cannot initialize statement";
            return 1;
        }
        // prepare it
        if (mysql_stmt_prepare(VV_CURR_DB.dbc->maria.stmt, stmt, strlen(stmt)) != 0)
        {
            cerror = "Cannot prepare statement";
            return 1;
        }
        if (stmt != origs) vely_free (stmt); // release statement if actually allocated
        // save it for reuse for as long as the process lives (minus reconnects to db, but those are rare)
        *prep = VV_CURR_DB.dbc->maria.stmt;
    }
    VV_CURR_DB.num_inp = num_of_args;
    // check param count correct before binding as the statement and what vely says may be different, in which case
    // mariadb would access memory that doesn't exist if vely say there's less than really is
    num count = mysql_stmt_param_count(VV_CURR_DB.dbc->maria.stmt);
    if (count != VV_CURR_DB.num_inp)
    {
        cerror = "Wrong number of input parameters";
        return 1;
    }

    if (VV_CURR_DB.num_inp != 0)
    {
        // allocate binds
        VV_CURR_DB.dbc->maria.bind = (MYSQL_BIND*)vely_calloc(num_of_args, sizeof(MYSQL_BIND));
    }
    return 0;
}


//
// Add input parameter to SQL about to be executed. 'i' is the index of parameter
// (which must be 0,1,2.. for each invocation), arg is the argument.
//
void vely_maria_add_input(num i, char *arg)
{
    VV_TRACE("");
    // all inputs are converted to string
    VV_CURR_DB.dbc->maria.bind[i].buffer_type = MYSQL_TYPE_STRING;
    // if null, it's empty
    VV_CURR_DB.dbc->maria.bind[i].buffer = (char*)(arg == NULL ? "":arg);
    VV_CURR_DB.dbc->maria.bind[i].buffer_length = strlen(arg);
    VV_CURR_DB.dbc->maria.bind[i].length = NULL; // this is for array binding
    // never null, it's Vely's simplification
    VV_CURR_DB.dbc->maria.bind[i].is_null = 0;
}

//
// Statement that has been prepared and input params specified, is now bound
//
void vely_maria_bind_input ()
{
    VV_TRACE("");
    if (VV_CURR_DB.num_inp != 0) 
    {
        mysql_stmt_bind_param (VV_CURR_DB.dbc->maria.stmt, VV_CURR_DB.dbc->maria.bind);
    }
}




num vely_maria_affected(char is_prep) 
{
    VV_TRACE("");
    if (is_prep == 0)
    {
        return (num) mysql_affected_rows (VV_CURR_DB.dbc->maria.con);
    }
    else
    {
        return (num) mysql_stmt_affected_rows (VV_CURR_DB.dbc->maria.stmt);
    }
}


//
// Execute statement that's prepared. Input binding is removed.
//
num vely_maria_stmt_exec()
{
    VV_TRACE("");
    num res = mysql_stmt_execute (VV_CURR_DB.dbc->maria.stmt);
    // freeing bind struct must be here and not vely_maria_free because vely_maria_free is not
    // called for non-tuple queries (such as INSERT), but this one is always called. If not like this,
    // bind ptr would remain and double free would be attempted
    if (VV_CURR_DB.dbc->maria.bind != NULL)
    {
        vely_free (VV_CURR_DB.dbc->maria.bind);
        VV_CURR_DB.dbc->maria.bind = NULL;
    }
    return res;
}

//
// Escape string for inclusion in quoted params. MariaDB may use different quotes, but we force ansi quotes.
// Even if we didn't, the string would be properly escaped.
// memory for to must be 2*len+1. *len  is the actual encoded length without zero byte counted.
// Returns 0 for okay, 1 otherwise.
//
int vely_maria_escape(char *from, char *to, num *len)
{
    VV_TRACE("");
    *len = (num)mysql_real_escape_string(VV_CURR_DB.dbc->maria.con, to, from, (unsigned long) *len);
    if ((unsigned long)*len == (unsigned long) -1) return 1;
    return 0;
}


