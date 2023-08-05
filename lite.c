// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// sqlite native interface, used by db.c for making of db-specific libs
//
// All errors must go through vely_lite_error() and the sole point of entry is vely_lite_exec()
// in order to avoid disorderly rollbacks in case of errors
//

#include "vely.h"
static int vely_lite_stmt_rows (char ***row, unsigned long **lens);
static int vely_lite_add_input(num i, char *arg);
static int vely_lite_prep_stmt(char is_prep, void **prep, char *stmt, num num_of_args);

static char *cerror = NULL;
static num qnumrows = 0;
static num qnumfields = 0;
static num qrownow = 0;
static char **qrows = NULL;
static unsigned long *qlens = NULL;
static num qcolid = 0;
static int vely_lite_get_data ();

num vely_lite_checkc()
{
    VV_TRACE("");
    return 1; // sqlite is a library, thus always "up" as long as the process is up
}

char *vely_lite_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er, char is_prep)
{
    VV_TRACE("");
    VV_UNUSED(is_prep);
    snprintf(errm,errmsize,"Error during query [%s], additional [%s] file [%s], line [%lld] : [%s]%s", s, cerror==NULL?"":cerror, sname, lnum, er ,atol(er) == ER_PARSE_ERROR ?  "Problem with parsing SQL statement" : sqlite3_errmsg(VV_CURR_DB.dbc->sqlite.con));
    return errm;
}


char *vely_lite_error(char *s, char is_prep)
{
    VV_TRACE("");
    VV_UNUSED(s); // used only for tracing
    VV_UNUSED(is_prep);
    static char errv[30];
    int errc;
    snprintf (errv, sizeof(errv), "%d", errc = sqlite3_errcode(VV_CURR_DB.dbc->sqlite.con));
    VV_TRACE ("Error in %s: %s error code %s", s, sqlite3_errstr(errc), errv);
    return errv;
}


int vely_lite_rows(char ***row, unsigned long **lens)
{
    VV_TRACE("");
    VV_CURR_DB.need_copy = 0;
    if (vely_lite_stmt_rows (row, lens)) return 1;
    return 0;
}


//
// Get a row when tuples returned. *row[] is an array of strings which represent fields (columns) and *lens is the
// array of lengths of those fields.
//
int vely_lite_stmt_rows (char ***row, unsigned long **lens)
{
    VV_TRACE("");
    if (qrows == NULL || qlens == NULL) 
    {
        cerror = "Cannot get row field and length data";
        return 1;
    }

// advance row when asked
    *row = &(qrows[qnumfields * qrownow]);
    *lens = &(qlens[qnumfields * qrownow]) ;
    qrownow++; 
    return 0;
}

int vely_lite_get_data ()
{
    VV_TRACE("");
    char *sname = "";
    num lnum = 0;
    // get which file and line number is this going on at
    vely_location (&sname, &lnum, 0);

    num i;

    // set qrows, qlens to NULL as vely_lite_get_data() is called only once for a query
    // below is freeing of these, and if not NULL, it would free bad data if no rows selected and there's an error (like
    // for example if inserting data and duplicate violation)
    qrows = NULL;
    qlens = NULL;
    num cdata = 0; // index of fields within a row
    qnumrows = 0;
    int r;
    char firstr = 0;
    num initr = 1; // initial number of rows
    while (1) 
    {
        if ((r = sqlite3_step(VV_CURR_DB.dbc->sqlite.stmt)) == SQLITE_ROW)
        {
            // number of fields in a row
            if (firstr == 0)
            {
                firstr = 1;
                qnumfields = sqlite3_column_count(VV_CURR_DB.dbc->sqlite.stmt);
                // get results (columns, lengths)
                qrows = (char **)vely_calloc (qnumfields * initr, sizeof(char*));
                qlens = (unsigned long *)vely_calloc (qnumfields * initr, sizeof(unsigned long));
            }
            if (qnumrows >= initr) 
            {
                initr = qnumrows + 1;
                qrows = (char **)vely_realloc (qrows, qnumfields * initr * sizeof(char*));
                qlens = (unsigned long *)vely_realloc (qlens, qnumfields * initr * sizeof(unsigned long));
            }
            //
            // Get a single row
            //
            // present row data for caller, wheter it's truncated or not. The only way it's not truncated is if all columns are NULL
            for (i = 0; i < qnumfields; i++) 
            {
                // use EMPTY_STRING if null, so freeing works
                char *res = (char*)sqlite3_column_text(VV_CURR_DB.dbc->sqlite.stmt, i);
                (qlens)[cdata] = sqlite3_column_bytes(VV_CURR_DB.dbc->sqlite.stmt, i);
                if (res == NULL)
                {
                    if (sqlite3_errcode(VV_CURR_DB.dbc->sqlite.con) == SQLITE_NOMEM)
                    {
                        cerror = "Out of memory when obtaining query result";
                        return 1;
                    } 
                    else
                    {
                        (qrows)[cdata] = VV_EMPTY_STRING;
                    }
                }
                else
                {
                    (qrows)[cdata] = vely_malloc ((qlens)[cdata] + 1);
                    memcpy ((qrows)[cdata], res, (qlens)[cdata]);
                    (qrows)[cdata][(qlens)[cdata]] = 0;
                }
                cdata++;
            }
            qnumrows++;
        }
        else 
        {
            if (r == SQLITE_DONE)  break;
            else
            {
                num i;
                for (i = 0; i < cdata; i ++) 
                {
                    vely_free ((qrows)[i]);
                }
                if (qrows != NULL) vely_free (qrows);
                if (qlens != NULL) vely_free (qlens);
                return 1;
            } 
        }
    }

    qcolid = 0; // starting index for fetching column names

    return 0;
}


num vely_lite_nrows()
{
    VV_TRACE("");
    return qnumrows;
}

void vely_lite_free(char is_prep)
{
    VV_TRACE("");
    if (is_prep == 0) vely_lite_close_stmt (VV_CURR_DB.dbc->sqlite.stmt);
    return;
}

char *vely_lite_fieldname()
{
    VV_TRACE("");
    return (char*)sqlite3_column_name(VV_CURR_DB.dbc->sqlite.stmt, qcolid++);

}

num vely_lite_nfield()
{
    VV_TRACE("");
    return qnumfields;
}

int vely_lite_use(char is_prep)
{
    VV_TRACE("");
    VV_UNUSED(is_prep);
    return 0;
}

int vely_lite_store(char is_prep)
{
    VV_TRACE("");
    VV_UNUSED(is_prep);
    return 0;
}


void vely_lite_close ()
{
    VV_TRACE("");
    sqlite3_close(VV_CURR_DB.dbc->sqlite.con);
}


vely_dbc *vely_lite_connect (num abort_if_bad)
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

    char db_config_name[150];
    snprintf (db_config_name, sizeof(db_config_name), "%s/%s", vely_get_config()->app.dbconf_dir, VV_CURR_DB.db_name);
    VV_TRACE ("Using db config file [%s]", db_config_name);
    char *cinfo;
    if (vely_read_file (db_config_name, &cinfo, 0, 0) < 0)
    {
        char em[300];
        snprintf (em, sizeof(em), "Cannot read database configuration file [%s]", db_config_name);
        VV_TRACE ("%s", em);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        vely_end_connection (0); // without it, we would think connection exists
        return NULL; // just for compiler, never gets here
    }
    // clean up config file, must be just file name
    num l = (num)strlen (cinfo);
    char *ts = vely_trim_ptr (cinfo, &l);
    if (strstr (ts, "\n")) 
    {
        char em[300];
        snprintf (em, sizeof(em), "Database file [%s] cannot have new line", ts);
        VV_TRACE ("%s", em);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        return NULL;
    }
    // make connection to database
    if (sqlite3_open(ts, &(VV_CURR_DB.dbc->sqlite.con)) != SQLITE_OK) 
    {
        char em[300];
        snprintf (em, sizeof(em), "Cannot cannot open database [%s]", ts);
        VV_TRACE ("%s", em);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        vely_end_connection (0); // without it, we would think connection exists
        return NULL;
    }

    return VV_CURR_DB.dbc;
}

num vely_lite_exec(char *s, char is_prep, void **prep, num paramcount, char **params)
{   
    VV_TRACE("");
    if (vely_lite_prep_stmt (is_prep, prep, s, paramcount)) return 1;
    num i;
    for (i = 0; i < paramcount; i++)
    {
        if (vely_lite_add_input(i, params[i]) != 0) return 1;
    }
    if (vely_lite_get_data () != 0) {
        return 1;
    }
    qrownow = 0; // row number currently served
    // sqlite must be reset to be able to reuse statement
    sqlite3_reset(VV_CURR_DB.dbc->sqlite.stmt);
    sqlite3_clear_bindings(VV_CURR_DB.dbc->sqlite.stmt);
    return 0;
}


//
// deallocate prepared statement (only when connection lost), st is void * to prep stmt.
//
void vely_lite_close_stmt (void *st)
{
    VV_TRACE("");
    if (st == NULL) return; // statement has not been prepared yet, so cannot deallocate
    if (VV_CURR_DB.dbc != NULL) 
    {
        // we only close statements when connection is lost, so we don't care about return code
        // only that memory is deallocated
        sqlite3_finalize((sqlite3_stmt *)st);
    }
}

//
// Prepare statement with source test stmt and num_of_args input parameters
// prep is the address of void * that points to prepared statement. This is a static
// void *, which survives requests (otherwise prepared statements wouldn't be very useful,
// actually would decrease performance), however prep is set to NULL when connection is 
// reestablished (typically if db server recycles), which is generally rare.
// is_prep is 0 if not prepare, 1 if it is. We need this because prep is !=NULL even when prepared
// is not used, resulting in the same SQL executed over and over
//
int vely_lite_prep_stmt(char is_prep, void **prep, char *stmt, num num_of_args)
{
    VV_TRACE("");
    char *sname = "";
    num lnum = 0;
    vely_location (&sname, &lnum, 0);

    // reuse prepared statement from prep
    // some statements (begin transaction, exec-query <dynamic query> and such) must be re-prepared
    // because they are *always* purely dynamic statements
    if (is_prep ==1 && prep != NULL && *prep != NULL) 
    {
        vely_stmt_cached = 1;
        VV_TRACE ("reusing prepared statement");
        VV_CURR_DB.dbc->sqlite.stmt = (sqlite3_stmt*)*prep;
    }
    else
    {
        VV_TRACE ("creating prepared statement");
        // if prep is NULL, create prepared statement
        char *origs = stmt; // original stmt
        stmt = vely_db_prep_text(stmt); // make ? instead of '%s'
        // prep stmt
        if (sqlite3_prepare_v2(VV_CURR_DB.dbc->sqlite.con, stmt, -1, &(VV_CURR_DB.dbc->sqlite.stmt), 0) != SQLITE_OK) 
        {
            cerror = "Cannot prepare statement";
            return 1;
        }
        if (stmt != origs) vely_free (stmt); // release statement if actually allocated
        // save it for reuse for as long as the process lives (minus reconnects to db, but those are rare)
        if (prep != NULL) *prep = VV_CURR_DB.dbc->sqlite.stmt;
    }
    VV_CURR_DB.num_inp = num_of_args;
    // check param count correct before binding as the statement and what vely says may be different, in which case
    // mariadb would access memory that doesn't exist if vely say there's less than really is
    num count = sqlite3_bind_parameter_count(VV_CURR_DB.dbc->sqlite.stmt);
    if (count != VV_CURR_DB.num_inp)
    {
        cerror = "Wrong number of input parameters";
        return 1;
    }

    return 0;
}


//
// Add input parameter to SQL about to be executed. 'i' is the index of parameter
// (which must be 0,1,2.. for each invocation), arg is the argument.
// return 0 on okay, 1 on error
//
int vely_lite_add_input(num i, char *arg)
{
    VV_TRACE("");
    if (sqlite3_bind_text(VV_CURR_DB.dbc->sqlite.stmt,i+1,arg,-1,SQLITE_STATIC) != SQLITE_OK) 
    {
        cerror = "Cannot bind input parameter";
        return 1;
    }
    return 0;
}


num vely_lite_affected(char is_prep) 
{
    VV_TRACE("");
    VV_UNUSED(is_prep);
    return (num) sqlite3_changes(VV_CURR_DB.dbc->sqlite.con);
}


//
// Escape string for inclusion in quoted params. MariaDB may use different quotes, but we force ansi quotes.
// Even if we didn't, the string would be properly escaped.
// memory for to must be 2*len+1. *len  is the actual encoded length without zero byte counted.
// Returns 0 for okay, 1 otherwise.
//
int vely_lite_escape(char *from, char *to, num *len)
{
    VV_TRACE("");
    memcpy (to, from, *len + 1);
    if (vely_replace_string (to, 2* *len+1, "\\", "\\\\", 1, NULL, 1) == -1) return 1;
    if ((*len = vely_replace_string (to, 2* *len+1, "'", "''", 1, NULL, 1)) == -1) return 1;
    return 0;
}


