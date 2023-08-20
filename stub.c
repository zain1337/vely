// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// Stubs for unused modules
//


#include "vely.h"


#ifdef VV_PCRE2
num vely_regex(char *look_here, char *find_this, char *replace, char **res, num utf8, num case_insensitive, num single_match, regex_t **cached, num *reslen) {
    VV_UNUSED(look_here);
    VV_UNUSED(find_this);
    VV_UNUSED(replace);
    VV_UNUSED(res);
    VV_UNUSED(utf8);
    VV_UNUSED(case_insensitive);
    VV_UNUSED(single_match);
    VV_UNUSED(cached);
    VV_UNUSED(reslen);
    return -1;
}
void vely_regfree(regex_t *preg) {
    VV_UNUSED(preg);
}
#endif

#ifdef VV_CRYPTO
void vely_sec_load_algos(void) {}
int vely_RAND_bytes(unsigned char *buf, int num) {
    VV_UNUSED(buf);
    VV_UNUSED(num);
    return 0; // failure if used by accident
}
#endif

#ifdef VV_FCGI
void vv_set_fcgi (vv_fc **callin, char *server, char *req_method, char *app_path, char *req, char *url_payload, char *ctype, char *body, int clen, int timeout, char **env) {
    VV_UNUSED(callin);
    VV_UNUSED(server);
    VV_UNUSED(req_method);
    VV_UNUSED(app_path);
    VV_UNUSED(req);
    VV_UNUSED(url_payload);
    VV_UNUSED(ctype);
    VV_UNUSED(body);
    VV_UNUSED(clen);
    VV_UNUSED(timeout);
    VV_UNUSED(env);
}
void vv_fc_delete (vv_fc *callin) {
    VV_UNUSED(callin);
}
num vv_call_fcgi (vv_fc **req, num threads, num *finokay, num *started) {
    VV_UNUSED(req);
    VV_UNUSED(finokay);
    VV_UNUSED(started);
    VV_UNUSED(threads);
    return 0;
}
#endif

#ifdef VV_CURL
void curl_global_cleanup(void) {}

CURLcode curl_global_init(long flags)
{
    VV_UNUSED(flags);
    return 0;
}
#endif


#ifdef VV_STUB_GENDB
void vely_check_transaction(num check_mode)
{
    VV_UNUSED(check_mode);
}
void vely_end_all_db() {}
#endif


#ifdef VV_STUB_POSTGRES
void vely_pg_close() {}
vely_dbc *vely_pg_connect (num abort_if_bad)
{
    VV_UNUSED(abort_if_bad);
    return NULL;
}
num vely_pg_exec(char *s, num returns_tuple,  char is_prep, void **prep, num paramcount, char **params)
{
    VV_UNUSED(s);
    VV_UNUSED(returns_tuple);
    VV_UNUSED(is_prep);
    VV_UNUSED(prep);
    VV_UNUSED(paramcount);
    VV_UNUSED(params);
    return 0;
}
num vely_pg_affected() {return 0;}
num vely_pg_nfield() {return 0;}
char *vely_pg_fieldname(num fnum)
{
    VV_UNUSED(fnum);
    return NULL;
}
void vely_pg_free() {}
num vely_pg_nrows() {return 0;}
void vely_pg_rows(char ***row, num num_fields, num nrow, unsigned long **lens)
{
    VV_UNUSED(row);
    VV_UNUSED(num_fields);
    VV_UNUSED(nrow);
    VV_UNUSED(lens);
}
char *vely_pg_error(char *s) 
{
    VV_UNUSED(s);
    return NULL;
}
char *vely_pg_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er)
{
    VV_UNUSED(errm);
    VV_UNUSED(errmsize);
    VV_UNUSED(s);
    VV_UNUSED(sname);
    VV_UNUSED(lnum);
    VV_UNUSED(er);
    return NULL;
}
num vely_pg_checkc() { return 0;}
void vely_pg_close_stmt (void *st)
{
    VV_UNUSED(st);
}
int vely_pg_escape(char *from, char *to, num *len)
{
    VV_UNUSED(from);
    VV_UNUSED(to);
    VV_UNUSED(len);
    return 0;
}
#endif


#ifdef VV_STUB_MARIADB
void vely_maria_close () {}
vely_dbc *vely_maria_connect (num abort_if_bad)
{
    VV_UNUSED(abort_if_bad);
    return NULL;
}
num vely_maria_exec(char *s,  char is_prep, void **prep, num paramcount, char **params)
{
    VV_UNUSED(s);
    VV_UNUSED(is_prep);
    VV_UNUSED(prep);
    VV_UNUSED(paramcount);
    VV_UNUSED(params);
    return 0;
}
num vely_maria_affected(char is_prep) 
{
    VV_UNUSED(is_prep);
    return 0;
}
int vely_maria_use(char is_prep)
{
    VV_UNUSED(is_prep);
    return 0;
}
int vely_maria_store(char is_prep)
{
    VV_UNUSED(is_prep);
    return 0;
}
num vely_maria_nfield() {return 0;}
char *vely_maria_fieldname() { return NULL; }
void vely_maria_free() {}
num vely_maria_nrows(char is_prep) 
{
    VV_UNUSED(is_prep);
    return 0;
}
int vely_maria_rows (char ***row, unsigned long **lens, char is_prep)
{
    VV_UNUSED(row);
    VV_UNUSED(lens);
    VV_UNUSED(is_prep);
    return 0;
}
char *vely_maria_error(char *s, char is_prep) 
{
    VV_UNUSED(s);
    VV_UNUSED(is_prep);
    return NULL;
}
char *vely_maria_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er, char is_prep)
{
    VV_UNUSED(errm);
    VV_UNUSED(errmsize);
    VV_UNUSED(s);
    VV_UNUSED(sname);
    VV_UNUSED(lnum);
    VV_UNUSED(er);
    VV_UNUSED(is_prep);
    return NULL;
}
num vely_maria_checkc() { return 0;}
void vely_maria_close_stmt (void *st)
{
    VV_UNUSED(st);
}
int vely_maria_escape(char *from, char *to, num *len)
{
    VV_UNUSED(from);
    VV_UNUSED(to);
    VV_UNUSED(len);
    return 0;
}
#endif


#ifdef VV_STUB_SQLITE
void vely_lite_close () {}
vely_dbc *vely_lite_connect (num abort_if_bad)
{
    VV_UNUSED(abort_if_bad);
    return NULL;
}
num vely_lite_exec(char *s,  char is_prep, void **prep, num paramcount, char **params)
{
    VV_UNUSED(s);
    VV_UNUSED(is_prep);
    VV_UNUSED(prep);
    VV_UNUSED(paramcount);
    VV_UNUSED(params);
    return 0;
}
num vely_lite_affected(char is_prep) 
{
    VV_UNUSED(is_prep);
    return 0;
}
int vely_lite_use(char is_prep)
{
    VV_UNUSED(is_prep);
    return 0;
}
int vely_lite_store(char is_prep)
{
    VV_UNUSED(is_prep);
    return 0;
}
num vely_lite_nfield() {return 0;}
char *vely_lite_fieldname() { return NULL; }
void vely_lite_free(char is_prep) {VV_UNUSED(is_prep);}
num vely_lite_nrows()
{
    return 0;
}
int vely_lite_rows (char ***row, unsigned long **lens)
{
    VV_UNUSED(row);
    VV_UNUSED(lens);
    return 0;
}
char *vely_lite_error(char *s, char is_prep) 
{
    VV_UNUSED(s);
    VV_UNUSED(is_prep);
    return NULL;
}
char *vely_lite_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er, char is_prep)
{
    VV_UNUSED(errm);
    VV_UNUSED(errmsize);
    VV_UNUSED(s);
    VV_UNUSED(sname);
    VV_UNUSED(lnum);
    VV_UNUSED(er);
    VV_UNUSED(is_prep);
    return NULL;
}
num vely_lite_checkc() { return 0;}
void vely_lite_close_stmt (void *st)
{
    VV_UNUSED(st);
}
int vely_lite_escape(char *from, char *to, num *len)
{
    VV_UNUSED(from);
    VV_UNUSED(to);
    VV_UNUSED(len);
    return 0;
}
#endif

