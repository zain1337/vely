// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// postgres native interface, used by db.c for making of db-specific libs
//

#include "vely.h"
static char *vely_pg_stmt(void **prep);
static int vely_pg_prep_stmt(void **prep, char *stmt, num num_of_args);
static char *cerror = NULL;

num vely_pg_checkc()
{
    VV_TRACE("");
    return (PQstatus(VV_CURR_DB.dbc->pg.con) != CONNECTION_OK ? 0 : 1);
}

char *vely_pg_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er)
{
    VV_TRACE("");
    char *detail = PQresultErrorField(VV_CURR_DB.dbc->pg.res, PG_DIAG_MESSAGE_DETAIL);
    snprintf(errm,errmsize,"Error during query [%s], additional [%s], detail [%s], file [%s], line [%lld] : [%s]%s", s, cerror==NULL?"":cerror, detail==NULL?"":detail, sname, lnum, er ,PQerrorMessage(VV_CURR_DB.dbc->pg.con));
    return errm;
}

char *vely_pg_error(char *s)
{
    VV_TRACE("");
    VV_UNUSED(s); // used for tracing only
    char *local_error = PQresultErrorField(VV_CURR_DB.dbc->pg.res, PG_DIAG_SQLSTATE);
    VV_TRACE ("Error in %s: [%s] error [%d] state [%s]", s, PQerrorMessage(VV_CURR_DB.dbc->pg.con), PQresultStatus(VV_CURR_DB.dbc->pg.res), local_error == NULL ? "": local_error);
    return local_error == NULL ? "" : local_error;
}

void vely_pg_rows(char ***row, num num_fields, num nrow, unsigned long **lens)
{
    VV_TRACE("");
    VV_CURR_DB.need_copy = 1;
    *row = (char**)vely_malloc(num_fields*sizeof(char*));
    num i;
    for(i = 0; i < num_fields; i++) (*row)[i] = PQgetvalue (VV_CURR_DB.dbc->pg.res, nrow, i);
    *lens = (unsigned long*)vely_malloc(num_fields*sizeof(unsigned long));
    for(i = 0; i < num_fields; i++) (*lens)[i] = PQgetlength (VV_CURR_DB.dbc->pg.res, nrow, i);
}


num vely_pg_nrows()
{
    VV_TRACE("");
    return PQntuples(VV_CURR_DB.dbc->pg.res);
}

void vely_pg_free()
{
    VV_TRACE("");
    if (VV_CURR_DB.dbc->pg.res != NULL) PQclear (VV_CURR_DB.dbc->pg.res);
    VV_CURR_DB.dbc->pg.res = NULL;
}

char *vely_pg_fieldname(num fnum)
{
    VV_TRACE("");
    return PQfname(VV_CURR_DB.dbc->pg.res, fnum);
}

num vely_pg_nfield()
{
    VV_TRACE("");
    return PQnfields(VV_CURR_DB.dbc->pg.res);
}

void vely_pg_close()
{
    VV_TRACE("");
    PQclear (VV_CURR_DB.dbc->pg.res);
    PQfinish (VV_CURR_DB.dbc->pg.con);
}


vely_dbc *vely_pg_connect (num abort_if_bad)
{
    VV_TRACE("");
    // reset all prepared statements
    vely_db_prep (NULL);


    char db_config_name[150];
    // allocate connection to database. Must be done here to account for the actual generated code, which
    // dictates the sizeof(vely_dbc)
    // This must be malloc and NOT vely_malloc since we want to reuse connection across connections.
    // Otherwise vely_malloc would be automatically freed when the request is done, and the next 
    // request would use an invalid pointer.  Also must use free to free it, not vely_free.
    if ((VV_CURR_DB.dbc = malloc (sizeof (vely_dbc))) == NULL) VV_FATAL ("Cannot allocate memory for database connection [%m]");

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
    // make connection to database
    VV_CURR_DB.dbc->pg.con = PQconnectdb(cinfo);
    VV_CURR_DB.dbc->pg.res = NULL;

    if (PQstatus (VV_CURR_DB.dbc->pg.con) != CONNECTION_OK)
    {
        char em[300];
        snprintf (em, sizeof(em), "Cannot initialize database connection [%s]", PQerrorMessage(VV_CURR_DB.dbc->pg.con));
        VV_TRACE ("%s", em);
        if (abort_if_bad == 1) vely_report_error ("%s", em);
        vely_end_connection (0); // without it, we would think connection exists
        return NULL;
    }
    return VV_CURR_DB.dbc;
}


num vely_pg_exec(char *s, num returns_tuple, char is_prep, void **prep, num paramcount, char **params)
{
    VV_TRACE("");
    vely_stmt_cached = 0;
    if (is_prep == 0)
    {
        return (PQresultStatus(VV_CURR_DB.dbc->pg.res = PQexec(VV_CURR_DB.dbc->pg.con, s)) != (returns_tuple == 1 ? PGRES_TUPLES_OK : PGRES_COMMAND_OK));
    }
    else
    {
        if (vely_pg_prep_stmt(prep, s, paramcount)) return 1;
        num num = PQresultStatus (VV_CURR_DB.dbc->pg.res = PQexecPrepared(VV_CURR_DB.dbc->pg.con, VV_CURR_DB.dbc->pg.name, paramcount, (const char * const *)params, NULL,NULL,0)) != (returns_tuple == 1 ? PGRES_TUPLES_OK : PGRES_COMMAND_OK);

        return num;
    }
}

num vely_pg_affected()
{
    VV_TRACE("");
    return atoll(PQcmdTuples (VV_CURR_DB.dbc->pg.res));
}


//
// deallocate prepared statement (only when connection lost), st is void * to prep stmt.
// the function will attempt to deallocate at server too; this is for future use, when we would deallocate in situation when connection is still up
//
void vely_pg_close_stmt (void *st)
{
    VV_TRACE("");
    if (st == NULL) return; // statement has not been prepared yet, so cannot deallocate
    if (VV_CURR_DB.dbc != NULL) 
    {
        // we only close statements when connection is lost, so we don't care about return code
        // only that memory is deallocated
        char dstm[60];
        snprintf(dstm, sizeof(dstm), "DEALLOCATE %s", (char*)st); // statement name is st
        PQexec(VV_CURR_DB.dbc->pg.con, dstm);
    }
    free (st); // deallocate name itself, was strdup-ed
}

char *vely_pg_stmt(void **prep)
{
    VV_TRACE("");
#define PGSTMTNAMEL 30
    static char tmp[PGSTMTNAMEL];
    snprintf (tmp, PGSTMTNAMEL, "%p", prep);// name is void *, which is unique, easy to get
    num i;
    if (!isalpha(tmp[0])) tmp[0] = 'A';
    for (i = 0; i < (num)strlen (tmp); i++)
    {
        if (!isalnum(tmp[i])) tmp[i] = 'A';
    }
    return tmp;
}

//
// Prepare statement with source text stmt and num_of_args input parameters. 
// prep is the address of void * that points to prepared statement. This is a static
// void *, which survives requests (otherwise prepared statements wouldn't be very useful,
// actually would decrease performance), however prep is set to NULL when connection is 
// reestablished (typically if db server recycles), which is generally rare.
//
int vely_pg_prep_stmt(void **prep, char *stmt, num num_of_args)
{
    VV_TRACE("");
    char *sname = "";
    num lnum = 0;
    vely_location (&sname, &lnum, 0);

    // reuse prepared statement from prep
    if (*prep != NULL) 
    {
        vely_stmt_cached = 1;
        VV_CURR_DB.dbc->pg.name = (char*)*prep;
    }
    else 
    {
        // if prep is NULL, create prepared statement
        // must duplicate as vely_pg_stmt returns static char*
        // must be strdup to survive request change (vely memory is gone, so no vely_strdup)
        if ((VV_CURR_DB.dbc->pg.name = strdup(vely_pg_stmt(prep))) == NULL)
        {
            cerror = "Out of memory for prepared statement";
            return 1;
        }

        // if prep is NULL, create prepared statement
        char *origs = stmt; // original stmt
        stmt = vely_db_prep_text(stmt); // make $N instead of '%s' newly alloc'd, or return original stmt if nothing to do
        // prepare statement
        PGresult *res = PQprepare(VV_CURR_DB.dbc->pg.con, VV_CURR_DB.dbc->pg.name, stmt, num_of_args, NULL);
        if (PQresultStatus(res) != PGRES_COMMAND_OK)
        {
            cerror = "Cannot prepare statement";
            return 1;
        }
        if (stmt != origs) vely_free (stmt); // release statement if actually allocated
        *prep = VV_CURR_DB.dbc->pg.name; // the name of prepared statements, lives for the life of the process
                                        // minus reconnects, which are rare
    }
    return 0;
}


//
// Escape string for inclusion in quoted params. Postgres uses single quotes only, which cannot be changed.
// memory for to must be 2*len+1. *len is the length of encoded string, not counting zero byte
//
int vely_pg_escape(char *from, char *to, num *len)
{
    VV_TRACE("");
    int err;
    *len = (num)PQescapeStringConn (VV_CURR_DB.dbc->pg.con, to, from, (size_t) *len, &err);
    if (err != 0) return 1;
    return 0;
}

