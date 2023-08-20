// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework. 

// 
// Main library used at VELY runtime. Most of the functions used
// within markups are implemented here. See also velyrtc.c which includes
// common functions shared between VELY runtime and VELY preprocessor.
//

#include "vely.h"


//  functions (local)
size_t vely_write_url_response(void *ptr, size_t size, size_t nmemb, void *s);
void vely_init_output_buffer ();
num vely_validate_output ();
void vely_set_arg0 (char *program, char **arg0);
num vely_write_web (bool iserr, vely_config *pc, char *s, num nbyte);
void vely_gen_set_content_type(char *v);
void vely_gen_add_header (char *n, char *v);
void vely_gen_set_status (num st, char *line);
void vely_send_header(vely_input_req *iu);
num vely_gen_util_read (char *content, num len);
void vely_gen_set_content_length(char *v);
void vely_server_error ();
num vely_header_err(vely_config *pc);
void vely_cant_find_file ();
char *vely_gen_get_env(char *n);
num vely_gen_write (bool is_error, char *s, num nbyte);
void vely_flush_trace();
void vely_write_ereport(char *errtext, vely_config *pc);
void vely_read_child (int ofd, char **out_buf, num *out_len);
void vely_gen_header_end ();
void vely_check_set_cookie (char *name, char *val, char *secure, char *samesite, char *httponly, char *safety_clause, size_t safety_clause_len);
// write-string macros
#define VV_WRSTR_CUR (vely_get_config()->ctx.req->curr_write_to_string)
#define VV_WRSTR (vely_get_config()->ctx.req->write_string_arr[VV_WRSTR_CUR])
#define VV_WRSTR_LEN (VV_WRSTR.len)
#define VV_WRSTR_BUF (VV_WRSTR.string)
#define VV_WRSTR_POS (VV_WRSTR.buf_pos)
#define VV_WRSTR_ADD (VV_WRSTR.wlen)
#define VV_WRSTR_ADD_MIN 128 // min memory to add to write-string
#define VV_WRSTR_ADD_MAX 8192 // max memory to add to write-string
#define VV_WRSTR_ADJMEM(x) ((x) = ((x) < VV_WRSTR_ADD_MAX ? 2*(x):(x)))

#ifndef VV_COMMAND
#include "fcgi_config.h"
#include "fcgiapp.h"
static FCGX_Stream *vely_fcgi_in = NULL, *vely_fcgi_out = NULL, *vely_fcgi_err = NULL;
static FCGX_ParamArray vely_fcgi_envp;
#endif
static char finished_output = 0;

extern num vely_end_program;

// 
// Initialize vely_input_req structure for fetching input URL data
// req is URL structure used to hold input data.
// DO NOT USE vely_get_config() here as it's not available yet!
//
void vely_init_input_req (vely_input_req *req)
{
    VV_TRACE("");
    num i;
    for (i=0; i < VV_MAX_NESTED_WRITE_STRING; i++)
    {
        req->write_string_arr[i].string = NULL;
        req->write_string_arr[i].len = 0;
        req->write_string_arr[i].buf_pos = 0;
        req->write_string_arr[i].notrim = 0;
        req->write_string_arr[i].wlen = VV_WRSTR_ADD_MIN;
    }
    req->curr_write_to_string = -1;// each write-to-string first increase it 
    req->disable_output = 0;
    req->app = NULL;
    req->if_none_match = NULL;
    req->cookies = NULL;
    req->num_of_cookies = 0;
    req->ip.names = NULL;
    req->ip.values = NULL;
    req->ip.num_of_input_params = 0;
    req->sent_header = 0;
    req->data_was_output = 0;
    req->silent = 0;
    req->ec = 0;
    req->is_shut = 0;
    req->header=NULL; // no custom headers, set to non-NULL for custom headers
    req->data = NULL; // user must assign request specific data structure
    req->body = VV_EMPTY_STRING;
    req->name = VV_EMPTY_STRING;
    req->body_len = 0;
    req->method = VV_OKAY;
    req->task = -1; // meaning task not set
    finished_output = 0; // reset finish-output indicator
    vely_mem_os = false; // new request means memory garbage collector is on again, regardless
                         // of what it was at the end of the previous one
}

//
//Do not trim the last new line in write-string
//
void vely_write_to_string_notrim ()
{
    VV_TRACE("");
    assert (VV_WRSTR_CUR < VV_MAX_NESTED_WRITE_STRING); // see comment in vely_write_to_string_length ()
    VV_WRSTR.notrim = 1;
}

// 
// Returns length of current top-level write-string (or equivalent API which is
// vely_write_to_string()) string being written.
//
num vely_write_to_string_length ()
{
    VV_TRACE ("");
    vely_input_req *req = vely_get_config()->ctx.req;
    assert (VV_WRSTR_CUR < VV_MAX_NESTED_WRITE_STRING); // overflow if asking within the last level 
                // because the level above it does not exist. We always show the length of previous write-string
                // and that is one level up
    return req->write_string_arr[req->curr_write_to_string+1].buf_pos;
}

// 
// Write to string. str is either a VELY-allocated string into which to write
// or NULL, which signifies end of string writing.
// Once non-NULL string str is passed here, all future writing (such as print-noenc
// or print-web etc) goes to this string, until this function is called with NULL.
// Writing to string can be nested, so writing to string2 (while writing to string1)
// will write to string2 until NULL is passed, when it switches back to string1.
//
void vely_write_to_string (char **str)
{
    VV_TRACE ("");
    if (str == NULL)
    {
        // stop writing to string
        if (VV_WRSTR_CUR<0)
        {
            vely_report_error ("Cannot stop writing to string if it was never initiated, or if stopped already");
        }
        if (VV_WRSTR_BUF == NULL)
        {
            vely_report_error ("Cannot find write-string data block");
        }
        if (VV_WRSTR.notrim == 0)
        {
            while (isspace(VV_WRSTR_BUF[VV_WRSTR_POS-1])) VV_WRSTR_POS--;  
            VV_WRSTR_BUF[VV_WRSTR_POS] = 0;
        }
        *(VV_WRSTR.user_string) =  VV_WRSTR_BUF;
        // Do NOT set VV_WRSTR_POS = 0 because then function vely_write_to_string_length()
        // couldn't possibly work

        // restore notrim so all future write-strings are not 'notrim'
        VV_WRSTR.notrim = 0;
        // no more string to write
        VV_WRSTR_BUF = NULL;
        VV_WRSTR_CUR--;
    }
    else
    {
        // start writing to string
        // Once curr_write_to_string is not -1 (i.e. 0 or more), there is a string writing in progress, even if 
        // nothing has been written to it yet. So the condition for "are we in string writing" is curr_write_to_string!=-1
        VV_WRSTR_CUR++;
        if (VV_WRSTR_CUR >= VV_MAX_NESTED_WRITE_STRING)
        {
            vely_report_error ("Too many nesting levels of writing to string in progress, maximum [%d] nesting levels", VV_MAX_NESTED_WRITE_STRING);
        }
        //
        // assign an empty string to *str, in case it's used somehow within write-string (trying to do recursion,
        // which is not allowed)
        *str = VV_EMPTY_STRING;
        VV_WRSTR.user_string = str;

        // Use internal pointer and memory to build a string. Original user string remains empty until the end.
        VV_WRSTR_ADD = VV_WRSTR_ADD_MIN; // start with min mem
        VV_WRSTR_LEN = VV_WRSTR_ADD; 
        VV_WRSTR_BUF= (char*) vely_malloc (VV_WRSTR_LEN);
        VV_WRSTR_POS = 0;

    }
}


// 
// Send html header out for a dynamically generated page. It is always no-caching.
// req is input request.
// If HTML output is disabled, NO header is sent.
//
void vely_output_http_header(vely_input_req *req)
{
    VV_TRACE("");
    if (req->sent_header == 1) 
    {
        VV_TRACE("Header already sent, attempted to send it again");
        return;
    }
    VV_TRACE ("sent header: [%lld]", req->sent_header);
    if (vely_get_config()->ctx.req->disable_output == 1) return;
    req->sent_header = 1; 
                // complain that header hasn't been sent yet! and cause fatal error at that.
    vely_send_header(req);
}

void vely_check_set_cookie (char *name, char *val, char *secure, char *samesite, char *httponly, char *safety_clause, size_t safety_clause_len)
{
    VV_TRACE("");
    char *chk = name;
    // Per rfc6265, cookie name must adhere to this and be present
    while (*chk != 0)
    {
        if (*chk < 33 || *chk == 127 || // this excludes space, tab and control chars
            *chk == '(' || *chk == ')' || *chk == '@' || *chk == ',' || *chk == ';' || *chk == ':' || *chk == '\\' ||
            *chk == '"' || *chk == '/' || *chk == '[' || *chk == ']' || *chk == '?' || *chk == '=' ||
            *chk == '{' || *chk == '}')
        {
            vely_report_error ("Cookie name [%s] is invalid at [%s]", name, chk);
        }
        chk ++;
    }
    if (name[0] == 0) vely_report_error ("Cookie name is empty");
    // Per rfc6265, cookie value must adhere to this and be present
    chk = val;
    while (*chk != 0)
    {
        if (*chk < 33 || *chk == 127 || // this excludes space, tab and control chars
            *chk == ',' || *chk == ';' || *chk == '\\' ||
            (*chk == '"' && (chk != val && *(chk+1) != 0))) // quote is allowed only as first and last byte, not inside
        {
            vely_report_error ("Cookie value [%s] is invalid at [%s]", val, chk);
        }
        chk ++;
    }
    if (val[0] == 0) vely_report_error ("Cookie value is empty");
    if (strcmp(secure, "Secure; ") && strcmp (secure, ""))
    {
        vely_report_error ("Cookie 'secure' can be only on or off, it is [%s]", secure);
    }
    if (strcmp(httponly, "HttpOnly; ") && strcmp (httponly, ""))
    {
        vely_report_error ("Cookie HttpOnly can be only on or off, it is [%s]", httponly);
    }
    if (samesite[0] != 0 && strcmp(samesite, "Strict") && strcmp (samesite, "Lax") && strcmp (samesite, "None"))
    {
        vely_report_error ("Cookie SameSite can be only empty, Strict, Lax or None");
    }
    if (samesite[0] != 0) snprintf (safety_clause, safety_clause_len, "; %s%sSameSite=%s", secure, httponly, samesite);
    else snprintf (safety_clause, safety_clause_len, "; %s%s", secure, httponly);
}


// 
// Sets cookie that's to be sent out when header is sent. req is input request, cookie_name is the name of the cookie,
// cookie_value is its value, path is the URL for which cookie is valid, expires is the date of exiration.
// SameSite is empty by default. Strict is to prevent cross-site request exploitations, for enhanced safety. Otherwise samesite can be
// Strict, Lax or None.
// httponly can be either "HttpOnly; " or empty string
// cookies[].is_set_by_program is set to  1 if this is the cookie we changed (i.e. not original in the web input).
//
void vely_set_cookie (vely_input_req *req, char *cookie_name, char *cookie_value, char *path, char *expires, char *samesite, char *httponly, char *secure)
{
    VV_TRACE ("cookie path [%s] expires [%s]", path==NULL ? "NULL":path, expires==NULL ? "NULL":expires);

    if (req->data_was_output == 1) 
    {
        vely_report_error ("Cookie can only be set before any data is output, and either before or after header is output.");
    }

    char safety_clause[200];
    vely_check_set_cookie (cookie_name, cookie_value, secure, samesite, httponly, safety_clause, sizeof(safety_clause));

    num ind;
    char *exp = NULL;
    vely_find_cookie (req, cookie_name, &ind, NULL, &exp);
    if (ind == -1)
    {
        if (req->num_of_cookies+1 >= VV_MAX_COOKIES)
        {
            vely_report_error ("Too many cookies [%lld]", req->num_of_cookies+1);
        }
        ind = req->num_of_cookies;
        req->num_of_cookies++;
    }
    else
    {
        vely_free (req->cookies[ind].data);
    }
    char cookie_temp[VV_MAX_COOKIE_SIZE + 1];
    if (expires == NULL || expires[0] == 0)
    {
        if (path == NULL || path[0] == 0)
        {
            snprintf (cookie_temp, sizeof(cookie_temp), "%s=%s%s", cookie_name, cookie_value, safety_clause);
            VV_TRACE("cookie[1] is [%s]", cookie_temp);
        }
        else
        {
            snprintf (cookie_temp, sizeof(cookie_temp), "%s=%s; Path=%s%s", cookie_name, cookie_value, path, safety_clause);
            VV_TRACE("cookie[2] is [%s]", cookie_temp);
        }
    }
    else
    {
        if (path == NULL || path[0] == 0)
        {
            snprintf (cookie_temp, sizeof(cookie_temp), "%s=%s; Expires=%s%s", cookie_name, cookie_value, expires, safety_clause);
            VV_TRACE("cookie[3] is [%s]", cookie_temp);
        }
        else
        {
            snprintf (cookie_temp, sizeof(cookie_temp), "%s=%s; Path=%s; Expires=%s%s", cookie_name, cookie_value, path, expires, safety_clause);
            VV_TRACE("cookie[4] is [%s]", cookie_temp);
        }
    }
    req->cookies[ind].data = vely_strdup (cookie_temp);
    req->cookies[ind].is_set_by_program = 1;
    VV_TRACE("cookie [%lld] is [%s]", ind,req->cookies[ind].data);
}

// 
// Find cookie based on name cookie_name. req is input request. Output: ind is the index in the cookies[] array in
// req, path/exp is path and expiration of the cookie. 
// When searching for a cookie, we search the cookie[] array, which we may have added to or deleted from, so it
// may not be the exact set of cookies from the web input.
// Returns cookie's value.
//
char *vely_find_cookie (vely_input_req *req, char *cookie_name, num *ind, char **path, char **exp)
{
    VV_TRACE ("");

    num ci;
    num name_len = strlen (cookie_name);
    for (ci = 0; ci < req->num_of_cookies; ci++)
    {
        VV_TRACE("Checking cookie [%s] against [%s]", req->cookies[ci].data, cookie_name);
        // Cookie (trimmed) must start with name=value. After that, other options may be in any order
        // But it is always ; Path=   ; Expires=  etc. - we set those in this exact format. We don't get any of 
        // those from the server - we set them, so we can search easily.
        if (!strncmp (req->cookies[ci].data, cookie_name, name_len) && req->cookies[ci].data[name_len] == '=')
        {
            if (ind != NULL) *ind = ci;
            char *val = req->cookies[ci].data+name_len+1;
            char *semi = strchr (val, ';');
            char *ret = NULL;
            if (semi == NULL)
            {
                ret = vely_strdup (val);
            }
            else
            {
                *semi = 0;
                ret = vely_strdup (val);
                *semi = ';';
            }
            if (path != NULL)
            {
                char *p = strcasestr (val, "; Path=");
                if (p != NULL)
                {
                    semi = strchr (p + 7, ';');
                    if (semi != NULL) *semi = 0;
                    *path = vely_strdup (p + 7);
                    if (semi != NULL) *semi = ';';
                }
                else
                {
                    *path = NULL;
                }
            }
            if (exp != NULL)
            {
                char *p = strcasestr (val, "; Expires=");
                if (p != NULL)
                {
                    semi = strchr (p + 10, ';');
                    if (semi != NULL) *semi = 0;
                    *exp = vely_strdup (p + 10);
                    if (semi != NULL) *semi = ';';
                }
                else
                {
                    *exp = NULL;
                }
            }
            return ret;
        }
    }
    if (ind != NULL) *ind = -1;
    return VV_EMPTY_STRING;
}

// 
// Delete cookie with name cookie_name. req is input request. "Deleting" means setting value to 'deleted' and 
// expiration day to Epoch start. But the cookies is still there and will be sent out in header response, however
// it won't come back since browser will delete it (due to expiration date and not because of 'deleted').
// Returns index in cookies[] array of the cookie we just deleted.
// cookies[].is_set_by_program is set to  1 if this is the cookie we deleted (i.e. not original in the web input).
// If path is specified, we use it; if not, we assume it was the same default one (which generally works unless
// you mix different paths, such as via different reverse proxies
num vely_delete_cookie (vely_input_req *req, char *cookie_name, char *path, char *secure)
{
    VV_TRACE ("");

    num ci;
    char *rpath = NULL;
    char *exp = NULL;
    vely_find_cookie (req, cookie_name, &ci, &rpath, &exp);
    if (ci != -1)
    {
        vely_free (req->cookies[ci].data);
        char del_cookie[300];
        char safety_clause[200];
        vely_check_set_cookie (cookie_name, "deleted", secure, "", "", safety_clause, sizeof(safety_clause));
        if (path != NULL)  rpath=path;
        if (rpath != NULL)
        {
            snprintf (del_cookie, sizeof (del_cookie), "%s=deleted; Path=%s; Max-Age=0; Expires=Thu, 01 Jan 1970 01:01:01 GMT%s", cookie_name, rpath, safety_clause);
        }
        else
        {
            snprintf (del_cookie, sizeof (del_cookie), "%s=deleted; Max-Age=0; Expires=Thu, 01 Jan 1970 01:01:01 GMT%s", cookie_name, safety_clause);
        }
        req->cookies[ci].data = vely_strdup (del_cookie);
        req->cookies[ci].is_set_by_program = 1;
        return ci;
    }
    return -1;
}


//
// Send header out. This does it only for web output, i.e. nothing happens for command line program.
// Only changed cookies are sent back (since unchanged ones are already in the browser). Cache is no-cache
// because the html output is ALWAYS generated. Cookies are secure if this is over https connection.
// Charset is always UTF-8.
//
// req can be NULL here, if called from oops page, or req may have very little data in it
// We send header out ONLY:
// 1. if explicitly called via vely_send_header or vely_output_http_header
// 2. it is an error
// 3. we send out a file
// This is important because we do NOT want to send header out just because some text is going
// out - this would break a lot of functionality, such as sending custom headers. 
// HEADER MUST BE EXPLICITLY SENT OUT.
// If req->header is not-NULL (ctype, cache_control, status_id, status_text or control/value), then
// custom headers are sent out. This way any kind of header can be sent.
//
void vely_send_header(vely_input_req *req)
{
    VV_TRACE("");

    vely_header *header = req->header;


    if (header!=NULL  && header->ctype != NULL)
    {
        //
        // Set custom content type if available
        //
        VV_TRACE("Setting custom content type for HTTP header (%s)", header->ctype);
        vely_gen_set_content_type(header->ctype);
    }
    else
    {
        vely_gen_set_content_type("text/html;charset=utf-8");
    }

    
    if (header!=NULL  && header->cache_control != NULL)
    {
        //
        // Set custom cache control if available
        //
        VV_TRACE("Setting custom cache for HTTP header (%s)", header->cache_control);
        vely_gen_add_header ("Cache-Control", header->cache_control);
    }
    else
    {
        // this is for output from VELY files only! for files, we cache-forever by default
        vely_gen_add_header ("Cache-Control", "max-age=0, no-cache");
        vely_gen_add_header ("Pragma", "no-cache");
        VV_TRACE("Setting no cache for HTTP header (1)");
        // the moment first actual data is sent, this is immediately flushed by fcgi?
    }

    //
    // Set status if available
    //
    if (header!=NULL && header->status_id != 0 && header->status_text != NULL)
    {
        vely_gen_set_status (header->status_id, header->status_text);
    } else vely_gen_set_status (200, "OK");

    // 
    // Set any custom headers if available
    //
    if (header!=NULL)
    {
        // add any headers set from the caller
        num i;
        for (i = 0; i<VV_MAX_HTTP_HEADER; i++)
        {
            if (header->control[i]!=NULL && header->value[i]!=NULL)
            {
                // we use add_header because it allows multiple directives of the same kind
                // but must not make duplicates of what's already there, except for Set-Cookie
                vely_gen_add_header (header->control[i], header->value[i]);
            }
            else break;
        }
    }
}

//
// Flush trace.
//
void vely_flush_trace()
{
    //
    // Do Not Trace this call, as it's trace internal workings
    //
    //
    // Make sure tracing is copied over to system buffers, such as before proceeding with error handling, 
    // just in case things go bad
    //
    vely_config *pc = vely_get_config();
    if (pc != NULL && pc->trace.f != NULL)
    {
        fflush (pc->trace.f);
    }
} 

//
// Write error report when fatal error happens. errtext is the error text.
// Guard agains request being NULL. pc must NOT be NULL (the exec context)
//
void vely_write_ereport(char *errtext, vely_config *pc)
{
    //
    //
    // !!
    // req can be NULL here so must guard for it
    // !!
    // This is to display the request itself that is associated with the error
    //
    //
    vely_input_req *req = pc->ctx.req;

    // Static variables are fine (for keeping the stack reserved), but
    // ONLY if they do not initialize! If they do, next time around (for 
    // the next request in fastcgi), they will NOT initialize, and they should.
    static char log_file[300];
    static char time[VV_TIME_LEN + 1];
    static FILE *fout;
    // End of OK static


    vely_current_time (time, sizeof(time)-1);
    snprintf (log_file, sizeof (log_file),  "%s/backtrace", pc->app.trace_dir);


    VV_TRACE ("Error has occurred, trying to open backtrace log [%s]", log_file);
    fout = vely_fopen (log_file, "a+");
    if (fout == NULL) 
    {
        fout = vely_fopen (log_file, "w+");
        if (fout == NULL)
        {
            VV_TRACE ("Cannot open report file, error [%s]", strerror(errno));
            VV_FATAL ("Cannot open report file, error [%m]");
        }
    }
    VV_TRACE ("Writing to backtrace log");
    fseek (fout, 0, SEEK_END);
    fprintf (fout, "%lld: %s: -------- BEGIN REPORT -------- \n", vely_getpid(), time);
    VV_TRACE ("Writing PID");
    fprintf (fout, "%lld: %s: URL: [%s][%s][%s]\n", vely_getpid(), time, vely_getenv("SCRIPT_NAME"), vely_getenv("PATH_INFO"), vely_getenv("QUERY_STRING"));
    num i;
    if (req != NULL && req->ip.names != NULL && req->ip.values != NULL)
    {
        VV_TRACE ("Writing input params");
        for (i = 0; i < req->ip.num_of_input_params; i++)
        {
            fprintf (fout, "%lld: %s:   Param #%lld, [%s]: [%s]\n", vely_getpid(), time, i, 
                req->ip.names[i] == NULL ? "NULL" : req->ip.names[i], 
                req->ip.values[i] == NULL ? "NULL" : req->ip.values[i]);
        }
    }
    VV_TRACE ("Writing error information");
    fprintf (fout, "%lld: %s: The trace of where the problem occurred:\n", vely_getpid(), time);
    fclose (fout); // close because we will be writing to backtrace (which is fout) in vely_get_stack
    VV_TRACE ("Getting stack");
    vely_get_stack (log_file);
    VV_TRACE ("Opening report file");
    fout = vely_fopen (log_file, "a+"); // continue to write to backtrace
    if (fout == NULL) 
    {
        VV_TRACE ("Cannot open report file, error [%s]", strerror(errno));
        VV_FATAL ("Cannot open report file, error [%m]");
    }
    fprintf (fout, "PID [%lld] TIME [%s] TRACE FILE [%s] ERROR: ***** %s *****\n", vely_getpid(), time, vely_get_config()->trace.fname, errtext);
    fprintf (fout, "%lld: %s: -------- END REPORT -------- \n", vely_getpid(), time);
    fclose (fout);

    VV_TRACE ("Before skipping request");
    vely_flush_trace();
} 

// 
// This is called when fatal error happens in program. Catches all errors, from program's report-error to SIGSEGV.
// Report an error in program. printf-like function that outputs error to trace file
// (if enabled). Backtrace files is also written. Exit code (for command line) is set to 99.
// After this, we don't exit, we jump to the end of request, so it will process the next request for FCGI
//
void _vely_report_error (char *format, ...) 
{
    VV_TRACE("");
    vely_mem_os = false; // switch to managed memory though no new heap should be alloc'd

    // THIS FUNCTION MUST NOT USE VV_MALLOC NOR MALLOC
    // as it can be used to report out of memory errors


    // get error message passed, format it into a single string
    // This has no dependencies on pc or anything really
    static char errtext[VV_MAX_ERR_LEN + 1];
    va_list args;
    va_start (args, format);
    num err_len = vsnprintf (errtext, sizeof(errtext), format, args);
    va_end (args);

    vely_config *pc = vely_get_config();
    if (pc == NULL)
    {
        VV_FATAL ("Program context is empty, error [%s]", errtext);
    }

    // when reporting error, any information traced here must go to the trace file, regardless
    // of whether we trace or not. We reset tracelevel at the beginning of each request so it doesn't stay in tracing.
    // trace the message - never needs to trace message just before report-error
    pc->debug.trace_level = 1;

    VV_TRACE ("Error: %s", errtext);
    vely_flush_trace(); // flush because if things go bad, no trace (literally) is left to examine (same for below calls)


    // Never error out more than once, if we do, say we did and move on to the next request
    // UnLikely to happen but still
    if (pc->ctx.vely_report_error_is_in_report == 1) 
    {
        VV_TRACE ("Leaving error handling because an error happened during error handling [%s]", errtext);
        vely_flush_trace();
        // Reason for exiting: if rollback fails (in vely_check_transaction below)
        // , then we would proceed to next request, and this can continue
        // previous request's transaction, leading to corrupt data
        VV_FATAL ("Fatal error during error reporting, error [%s]", errtext); 
        return; // should never happen
    }
    pc->ctx.vely_report_error_is_in_report = 1; 

    // Rollback any open transactions. Error can happen in there too, but if it dies, we exit the process (see above)
    // rather than risk complications
    vely_check_transaction (1);

    vely_write_ereport (errtext, pc);
    vely_flush_trace();


    // send to stderr (for web client, goes to web server error log, for standalone to stderr)

    // we will try to make 500 Server Error here, but if the header has already been output, it won't come out
    // for example vely_bad_request() may have been done prior to this, or just out-header
    vely_server_error();
    // write to stderror (server log)
    vely_gen_write (true, errtext, err_len);
    vely_gen_write (true, "\n", 1);


    //
    //
    // if this report-error happens anywhere outside a request, we will just start a new one
    // Otherwise, close this request, release resources and process the next one (for FCGI).
    //
    //
    vely_error_request(1);// go to process the next request
}



// 
// Decode string encoded previously (web or url encoding). enc_type is VV_WEB or
// VV_URL. String v (which is encoded at the entry) holds decoded value on return.
// inlen is the number of bytes to decode, negative if all until null-char (strlen)
// Return value is the length of decoded string.
//
num vely_decode (num enc_type, char *v, num inlen)
{
    VV_TRACE("");

    if (inlen < 0) inlen = strlen (v);

    num i;
    num j = 0;
    if (enc_type == VV_WEB)
    {
        for (i = 0; i < inlen; i ++)
        {
            if (v[i] == '&')
            {
                if (!strncmp (v+i+1, "amp;", 4))
                {
                    v[j++] = '&';
                    i += 4;
                }
                else if (!strncmp (v+i+1, "quot;", 5))
                {
                    v[j++] = '"';
                    i += 5;
                }
                else if (!strncmp (v+i+1, "apos;", 5))
                {
                    v[j++] = '\'';
                    i += 5;
                }
                else if (!strncmp (v+i+1, "lt;", 3))
                {
                    v[j++] = '<';
                    i += 3;
                }
                else if (!strncmp (v+i+1, "gt;", 3))
                {
                    v[j++] = '>';
                    i += 3;
                }
                else v[j++] = v[i];
            }
            else
            {
                v[j++] = v[i];
            }
        }
        v[j] = 0;
    }
    else if (enc_type == VV_URL)
    {
        // check if each examined hex digit is valid, and if not break and finish by setting 0 at the end
        // this covers %X<null> pr %XY where X or Y are invalid of %<null> and such
        // this macro is specific to the following loop only
#define URLDIG(x,r)  if ((x) >='A' && (x) <= 'F') (r)=((x)-'A')+10;\
                else if ((x) >='a' && (x) <= 'f') (r)=((x)-'a')+10;\
                else if ((x) >='0' && (x) <= '9') (r)=(x)-'0';\
                else break;
        for (i = 0; i < inlen; i ++)
        {
            if (v[i] == '%')
            {
                int h;
                int l;
                URLDIG(v[i+1],h);
                URLDIG(v[i+2],l);
                v[j++] = h*16+l;
                i+=2;
            } else if (v[i] == '+')
            {
                v[j++] = ' ';
            }
            else
            {
                v[j++] = v[i];
            }
        }
        v[j] = 0;
    }
    else 
    {
        assert (1==2);
    }
    return j;
}

//
// Lock a file. filepath is the file. lock_fd is the file descriptor of the locked
// open file. Returns VV_OKAY if locked and other error codes if not.
// This is used for locking resources between various processes. Once a process exits,
// the lock is released - meaning if you fork() a process and then exit, the forked process
// will NOT have the lock.
//
num vely_lockfile(char *filepath, num *lock_fd)
{
    VV_TRACE ("");
    struct flock lock;
    num fd;

    /* Invalid path? */
    if (filepath == NULL || *filepath == '\0')
    {
        VERR0;
        return VV_ERR_INVALID;
    }

    /* Open the file. No checking for EINTR, as it is fatal in chandle.c */
    fd = open(filepath, O_RDWR | O_CREAT, 0600);

    if (fd == -1)
    {
        VERR;
        return VV_ERR_CREATE;
    }

    /* Application must NOT close input/output/error, or those may get occupied*/

    /* Exclusive lock, cover the entire file (regardless of size). */
    lock.l_type = F_WRLCK;
    lock.l_whence = SEEK_SET;
    lock.l_start = 0;
    lock.l_len = 0;
    if (fcntl(fd, F_SETLK, &lock) == -1)
    {
        VERR;
        /* Lock failed. Close file and report locking failure. */
        close(fd);
        return VV_ERR_FAILED;
    }
    if (lock_fd != NULL) *lock_fd = fd;
    // success, the file is open and locked, and will remain locked until the process ends
    // or until the caller does close (*lock_fd);
    return VV_OKAY;
}



// 
// Get input parameters from web input in the form of
// name/value pairs, meaning from a GET, POST, PUT, PATCH or DELETE (with OPTIONS to support for CORS).  
// req is an input request
// If 'method' if NULL, environment REQUEST_METHOD is read, otherwise 'input' must be specified as input for query string.
// this is useful for passing URL from command line.
// For any method we read both body and query string. For some like GET, body is unusual but not forbidden.
// query string is based on environment variable QUERY_STRING
// Returns 1 if ok, 0 if not ok but handled (such as Forbidden), otherwise Errors out.
// Other information is obtained too such as HTTP_REFERRED that could be used to disallow
// viewing images unless referred to by this site (not a functionality here, it
// can be implemented in vely application).
// ETag/If-non-match is obtained for cache management. Cookies are obtained from the client
// ONLY the first time this is called in a request - we may alter cookies later, so if vely_get_input()
// is called again in the same request, cookies are NOT obtained again.
// Input parameters are stored in req variable, where they can be obtained from
// by the program. Files are uploaded automatically and are parameterized in the form of xxx_location, xxx_filename,
// xxx_ext, xxx_size, xxx_id. So if input parameter was myparam, it would be 
// myparam_location, myparam_filename etc. _location is the local file path where file is
// stored. _filename is the actual HTML filename parameter. _ext is extension (with . in 
// front of it, lower cased). _size is the size of the file. _id is the id produced by
// vely_get_document_id() and also id is what _location is based on. If file was empty, then
// all params but _filename are empty.
// If 'input' is specified, it overrides what's from QUERY_STRING. This is only if 'method' present.
//
num vely_get_input(vely_input_req *req, char *method, char *input)
{
    VV_TRACE("");
    req->ip.num_of_input_params = 0;
    req->ip.names = NULL;
    req->ip.values = NULL;

    vely_config *pc = vely_get_config();
    char *req_method = NULL;
    char *qry = NULL;
    char *cont_type = NULL;
    char *cont_len = NULL;
    num cont_len_byte = 0; // default zero if content-length not specified
    char *content = NULL;
    char *cookie = NULL;
    req->ip.num_of_input_params = 0;

    // some env vars are obtained right away, other are rarely used
    // and are obtaineable from $$ variables
    VV_STRDUP (req->referring_url, vely_getenv ("HTTP_REFERER"));
    VV_TRACE ("Referer is [%s]", req->referring_url);
    // when there is a redirection to home page, referring url is empty

    char *sil = vely_getenv ("VV_SILENT_HEADER");
    if (!strcmp (sil, "yes")) 
    {
        req->silent = 1;
    }

    char *nm = vely_getenv ("HTTP_IF_NONE_MATCH");
    if (nm[0] != 0)
    {
        VV_STRDUP (req->if_none_match, nm);
        VV_TRACE("IfNoneMatch received [%s]", nm);
    }

    // this function is often called in "simulation" of a request. ONLY the first request gets cookies
    // from the client (which is HTTP_COOKIE). After this first request, we may alter cookies in memory,
    // and so we do NOT get cookies again from the client.
    if (req->cookies == NULL)
    {
        // make a copy of cookies since we're going to change the string!
        cookie = vely_strdup (vely_getenv ("HTTP_COOKIE"));
        req->cookies = vely_calloc (VV_MAX_COOKIES, sizeof (vely_cookies)); // regardless of whether there are cookies in input or not
                                // since we can set them. This also SETS TO ZERO is_set_by_program which is a MUST, so HAVE to use vely_calloc.
        if (cookie[0] != 0)
        {
            VV_TRACE ("Cookie [%s]", cookie);
            num tot_cookies = 0;
            while (1)
            {
                if (tot_cookies >= VV_MAX_COOKIES) 
                {
                    vely_bad_request();
                    vely_report_error("Too many cookies [%lld]", tot_cookies);
                }
                char *ew = strchr (cookie, ';');
                if (ew != NULL)
                {
                    *ew = 0;
                    ew++;
                    while (isspace(*ew)) ew++;
                }
                req->cookies[tot_cookies].data = vely_strdup (cookie);
                VV_TRACE("Cookie [%s]",req->cookies[tot_cookies].data);
                tot_cookies++;
                if (ew == NULL) break;
                cookie = ew;
            }
            req->num_of_cookies = tot_cookies;
        }
        else 
        {
            req->num_of_cookies = 0;
        }
    }

    // request method, GET or POST
    // method, input override environment
    // if method, 'input' will overrde QUERY_STRING
    if (method != NULL)
    {
        req_method = method;
    }
    else
    {
        req_method = vely_getenv ("REQUEST_METHOD");
    }
   
    if (req_method[0] == 0) 
    {
        vely_bad_request();
        vely_report_error ("REQUEST_METHOD environment variable is not found");
    }

    num is_multipart = 0;

    VV_TRACE ("Request Method: %s", req_method);

    char *new_cont = (char*)vely_malloc (VV_MAX_SIZE_OF_URL + 1);
    num new_cont_ptr = 0;
    num would_write;

    bool is_post = false;
    bool is_patch = false;
    bool is_get = false;
    bool is_delete = false;
    bool is_put = false;
    bool is_other = false;
    bool is_query_string = false; // is the body x-www-form-urlencoded

    if (!strcasecmp(req_method, "POST")) {is_post=true;req->method=VV_POST;}
    else if (!strcasecmp(req_method, "GET")) {is_get=true;req->method=VV_GET;}
    else if (!strcasecmp(req_method, "PATCH")) {is_patch=true;req->method=VV_PATCH;}
    else if (!strcasecmp(req_method, "PUT")) {is_put=true;req->method=VV_PUT;}
    else if (!strcasecmp(req_method, "DELETE")) {is_delete=true;req->method=VV_DELETE;}
    else {is_other=true;req->method=VV_OTHER;}

    // content type is generally not specified for GET or DELETE, but it may be
    // so this is generic processing with a few constraints
    cont_type = vely_getenv ("CONTENT_TYPE");
    // size of input data
    cont_len = vely_getenv ("CONTENT_LENGTH");
    if (cont_type[0] != 0)
    {
        // handle POST request, first check content type
        // x-www-form-urlencoded and multipart/form-data are two content mime types that all clients
        // must support. urlencode is for non-binary and multipart is when files are involved. It is
        // one or the other. Per https://datatracker.ietf.org/doc/html/rfc7578, multipart/mixed is 
        // deprecated and is not implemented here.
        char *mult = "multipart/form-data;";
        char *mform = NULL;
        if ((mform = strcasestr (cont_type, mult)) != NULL)
        {
            if (mform == cont_type || *(mform - 1) == ';' || isspace (*(mform - 1)))
            {
                is_multipart = 1;
            }
        }
        if (!strcasecmp (cont_type, "application/x-www-form-urlencoded")) is_query_string = true;
        if (cont_len[0] != 0)
        {
            cont_len_byte = atoi (cont_len);
            if (cont_len_byte != 0)
            {
                // If query string, then limit of URL, otherwise max upload for file 
                if (!is_query_string)
                {
                    if (cont_len_byte >= pc->app.max_upload_size)
                    {
                        vely_bad_request();
                        vely_report_error ("Upload file size too large [%lld]", cont_len_byte);
                    }
                }
                else
                {
                    if (cont_len_byte >= VV_MAX_SIZE_OF_URL)
                    {
                        vely_bad_request();
                        vely_report_error ("Query string larger than the limit of [%d] bytes (1)", VV_MAX_SIZE_OF_URL);
                    }
                }
                // Must have 2 extra bytes b/c for URL two nulls in a row signify the end of name/value pairs
                // which are normally separated by a null
                content = (char*)vely_malloc (cont_len_byte + 2);
                // get input data
                if (vely_gen_util_read (content, cont_len_byte) != 1)
                {
                    vely_bad_request();
                    vely_report_error ("Error reading request body");
                }
                content [cont_len_byte] = content[cont_len_byte+1] = 0;

                // for example, GET might use body to convey an image of an object that needs to be found
                // plain query string may not be enough. Or a json document needed to find the resource.
                // Here we check if body of message is not multipart
                req->body = content;                
                req->body_len = cont_len_byte;
            }
        }
    }
    else
    {
        // if content-type is empty then we specify empty "content" variable so that further code can for instance
        // add URL fields to it and content isn't NULL for the code that tries to copy it or realloc it.
        // we enforce that content-length must not be present or must be 0 in this case.
        // This works for POST/PUT/PATCH and for GET/DELETE (where content will be created for them)
        // Must have 2 extra bytes b/c for URL two nulls in a row signify the end of name/value pairs
        // which are normally separated by a null
        cont_len_byte = 0;
        content = (char*)vely_malloc (cont_len_byte + 2);
        content[0] = 0;
    }

    // After this point, cont_len_byte is really length of any query string, including for file uploading
    // Because if it's any kind of content that's not form-data or urlencoded, then it's in req->body and req->body_len


    // If client asking for options, present options handled
    // For better CORS (Cross Origin Resource Sharing), use referrer in your vely code to determine
    // access or no-access
    if (!strcasecmp(req_method, "OPTIONS")) {
        vely_gen_set_status (204, "No Content");
        vely_gen_add_header ("Allow", "OPTIONS, GET, POST, PATCH, PUT, DELETE");
        vely_gen_add_header ("Access-Control-Allow-Methods", "OPTIONS, GET, POST, PATCH, PUT, DELETE");
        vely_gen_add_header ("Cache-Control", "max-age: 86400");
        vely_gen_header_end ();
        return 0;
    }
    else if (is_get || is_delete || is_other)
    {
        // if body is a query string, then it should be added to whatever query string is in URL
        // if not a query string, then content from body is ignored for the URL building purposes
        if (!is_query_string) 
        {
            cont_len_byte = 0; // if there is any query string after this, those params are on their own,
            // Must have 2 extra bytes b/c for URL two nulls in a row signify the end of name/value pairs
            // which are normally separated by a null
            content = (char*)vely_malloc (cont_len_byte + 2);
            content[0] = 0;
        }
    }
    else if (is_post || is_put || is_patch)
    {
        //
        // When POST has no files, it's really just one query string in the body, which is
        // application/x-www-form-urlencoded
        // If is_multipart  is 1, then there are binary documents encoded between "boundaries"
        // Any other content type is allowed too, for instance application/json or anything else
        //
        if (cont_type[0] != 0)
        {
            // for non-empty content type, content len must be present not 0
            if (cont_len[0] == 0)
            {
                vely_bad_request();
                vely_report_error ("Missing content length");
            }
            if (cont_len_byte == 0)
            {
                vely_bad_request();
                vely_report_error ("Content length is zero");
            }
            if (is_multipart == 1)
            {
                // for multipart, we get structured delivery, no body as it will be parsed and cut
                req->body = VV_EMPTY_STRING;
                req->body_len = 0;

                // Based on RVM2045 (MIME types) and RVM1867 (file upload in html form)
                // Boundary is always CRLF (\r\n) and for 'multipart' type, the content-transfer-encoding must
                // always be 7bit/8bit/binary, i.e. no base64
                char *boundary_start = "boundary=";
                char *bnd = strcasestr (cont_type, boundary_start);
                if (bnd == NULL)
                {
                    vely_bad_request();
                    vely_report_error ("Cannot find boundary in content type header [%s]",cont_type);
                }
                if (bnd != cont_type && !isspace(*(bnd - 1)) && *(bnd - 1) != ';') 
                {
                    vely_bad_request();
                    vely_report_error ("Cannot find boundary in content type header [%s]",cont_type);
                }
                char *b = bnd + strlen (boundary_start); // b is now boundary string up new line or ;
                num boundary_end =  strcspn (b, "\n;");
                b[boundary_end] = 0; // b is now boundary string
                num boundary_len = strlen (b);
                vely_trim (b, &boundary_len);
                // look for multi-part elements, one by one
                char *c = content; // start with the beginning of content
                char *past_end = content + cont_len_byte;
                num remainder_len;
                while (1)
                {
                    char *file_name = NULL;
                    char *name = NULL;
                    char *name_val = NULL;
                    char *multi_ctype = NULL;
                    num multi_ctype_len = 0;

                    remainder_len = past_end - c; // calculate how much we advanced in the input data

                    char *el = memmem(c, remainder_len, b, boundary_len);// look for boundary in content
                    //char *el = strstr (c, b); 

                    // boundary is always preceded by -- but we don't look for that
                    // if boundary is superceded by -- it's the last one
                    if (el == NULL)  break;
                    el += boundary_len;
                    if (*(el + 1) == '-' && *(el + 2) == '-') break;
                    char *c1 = "Content-Disposition:";
                    char *prev = el;
                    el = strcasestr (el, c1); // pos of 'content-disposition'
                    if (el == NULL) break;
                    if (el != prev && !isspace(*(el - 1)) && *(el - 1) != ';') break;
                    char *end_of_element = strchr (el, '\n');
                    if (end_of_element != NULL) *end_of_element = 0; // mark the end of content-disp. line 
                                                // so we don't go beyond it
                    el += strlen (c1);

                    char *beg_of_line = el; // now we're past content-disposition
                    // Find name
                    char *c2 = "name=";
                    prev = el;
                    el = strcasestr (el, c2); // pos of name=
                    if (el == NULL) break;
                    if (el != prev && !isspace (*(el - 1)) &&  *(el - 1) != ';') break;
                    el += strlen (c2);
                    char *element_end = el + strcspn (el, ";"); // end of name=
                    char char_end = *element_end;
                    *element_end = 0;
                    name = el;
                    num name_len = strlen (name);
                    vely_trim (name, &name_len);
                    VV_STRDUP (name, name);
                    if (name[0] == '"') 
                    { 
                        name[name_len - 1] = 0; 
                        name++;
                    }
                    *element_end = char_end; // restore char

                    // find file name, this one is optional
                    char *c3 = "filename=";
                    el = beg_of_line; // look for file name in the line again
                    prev = el;
                    el = strcasestr (el, c3); // pos of filename=
                    if (el != NULL) 
                    {
                        if (prev != el && !isspace (*(el - 1)) && *(el - 1) != ';') break;
                        el += strlen (c3);
                        // we use strcspn since there may not be ; but rather the string just ends
                        char *element_end = el + strcspn (el, ";"); // end of filename=
                        char char_end = *element_end;
                        *element_end = 0;
                        file_name = el;
                        num filename_len = strlen (file_name);
                        vely_trim (file_name, &filename_len);
                        VV_STRDUP (file_name, file_name);
                        if (file_name[0] == '"') 
                        { 
                            file_name[filename_len - 1] = 0;
                            file_name++;
                        }
                        *element_end = char_end;
                    }
                    else 
                    {
                        VV_STRDUP (file_name, "");
                    }

                    // after looking up name/filename, go to next line. If empty, the following
                    // line is value. If not it could be Content-Type or something else
                    char *cval = end_of_element + 1;
                    char *end_of_val = strchr (cval, '\n');
                    if (end_of_val == NULL) break;
                    *end_of_val = 0;
                    num len_cont_val = strlen (cval);
                    vely_trim (cval, &len_cont_val);
                    if (cval[0] == 0)
                    {
                        // the following line is the actual value of parameter [name], name_val is value
                        name_val = end_of_val + 1;
                        // now find the boundary
                        char *end_name = memmem(name_val, past_end - name_val, b, boundary_len);// look for boundary in content
                        if (end_name == NULL) break;
                        *(end_name - 4) = 0; // account for CRLF prior to boundary and for '--'
                        c = end_name - 3; // continue right after the end, in the middle of CRLF
                        num len_of_name_val = (end_name - 4 - name_val);
                        // name_val is the value
                        // len_of_name_val is the length
                        vely_trim (name_val, &len_of_name_val);
                        VV_STRDUP (name_val, name_val);
                    }
                    else
                    {
                        // Now, we should find Content-Type or something else
                        // We ignore all that is here since we will always just
                        // save the file as binary, whatever it is. It is ip to application
                        // to figure out
                        // this is file, and we should have filename
                        // but not fatal if we do not
                        VV_STRDUP (name_val, "");
                        multi_ctype = end_of_val + 1;
                        char *nlBeforeCont = strchr (multi_ctype, '\n');
                        if (nlBeforeCont == NULL) break;
                        multi_ctype = nlBeforeCont + 1;
                        // now find the boundary
                        char *end_file = memmem(multi_ctype, past_end - multi_ctype, b, boundary_len);// look for boundary in content
                        if (end_file == NULL) break;
                        *(end_file - 4) = 0; // account for CRLF prior to boundary and for '--'
                        c = end_file - 3; // continue right after the end, in the middle of CRLF
                        multi_ctype_len = (end_file - 4 - multi_ctype);
                        // multi_ctype is now the file
                        // multi_ctype_len is the binary length
                    }
                    num avail;
                    char *enc;
                    enc = NULL;
                    // name of attachment input parameter
                    vely_encode (VV_URL, name_val, -1, &enc);
                    would_write = snprintf (new_cont + new_cont_ptr, avail = VV_MAX_SIZE_OF_URL - new_cont_ptr - 2, "%s=%s&", name, enc);
                    vely_free (enc);
                    if (would_write >= avail)
                    {
                        vely_bad_request();
                        vely_report_error ("Web input larger than the limit of [%d] bytes (2)", VV_MAX_SIZE_OF_URL);
                    }
                    new_cont_ptr += would_write;
                    if (file_name[0] != 0)
                    {
                        // provide original (client) file name
                        enc = NULL;
                        vely_encode (VV_URL, file_name, -1, &enc);
                        num would_write = snprintf (new_cont + new_cont_ptr, avail = VV_MAX_SIZE_OF_URL - new_cont_ptr - 2, "%s_filename=%s&", name, enc);
                        vely_free (enc);
                        if (would_write  >= avail)
                        {
                            vely_bad_request();
                            vely_report_error ("Web input larger than the limit of [%d] bytes (3)", VV_MAX_SIZE_OF_URL);
                        }
                        new_cont_ptr += would_write;
                    }
                    // write file
                    // generate unique number for file and directory, create dir if it doesnt exist
                    // as well as file. We can only tell that file name was not uploaded if file name is
                    // empty. All files appear 'uploaded'. If a file was not specified, it still exists
                    // in the content, it's just empty. The only way to tell it was not uploaded is 
                    // by empty file_name.

                    if (multi_ctype != NULL)
                    {
                        if (file_name[0] != 0)
                        {
                            // get extension of filename
                            num flen = strlen (file_name);
                            num j = flen - 1;
                            char *ext = "";
                            while (j > 0 && file_name[j] != '.') j--;
                            if (file_name[j] == '.')
                            {
                                VV_STRDUP (ext, file_name + j); // .something extension captured
                                if (!strcasecmp (ext, ".jpeg")) vely_copy_data (&ext, ".jpg");
                                if (!strcasecmp (ext, ".jpg")) vely_copy_data (&ext, ".jpg");
                                if (!strcasecmp (ext, ".pdf")) vely_copy_data (&ext, ".pdf");
                            }
                            //lower case extension
                            flen = strlen (ext);
                            for (j = 0; j < flen; j++) ext[j] = tolower (ext[j]);

                            VV_DEFINE_STRING (write_dir);
                            FILE *f = vely_make_document (&write_dir, 0);

                            // write the actual uploaded file contents to local file
                            if (fwrite(multi_ctype, multi_ctype_len, 1, f) != 1)
                            {
                                // this is not a bad request, but server error
                                vely_report_error ("Cannot write file [%s], error [%s]", write_dir, strerror (errno));
                            }
                            fclose (f);

                            // provide location where file is actually stored on server
                            enc = NULL;
                            vely_encode (VV_URL, write_dir, -1, &enc);
                            num would_write = snprintf (new_cont + new_cont_ptr, avail = VV_MAX_SIZE_OF_URL - new_cont_ptr - 2, "%s_location=%s&", name, enc);
                            vely_free (enc);
                            if (would_write  >= avail)
                            {
                                vely_bad_request();
                                vely_report_error ("Web input larger than the limit of [%d] bytes (4)", VV_MAX_SIZE_OF_URL);
                            }
                            new_cont_ptr += would_write;

                            // provide extension of the file
                            would_write = snprintf (new_cont + new_cont_ptr, avail = VV_MAX_SIZE_OF_URL - new_cont_ptr - 2, "%s_ext=%s&", name, ext);
                            if (would_write  >= avail)
                            {
                                vely_bad_request();
                                vely_report_error ("Web input larger than the limit of [%d] bytes (5)", VV_MAX_SIZE_OF_URL);
                            }
                            new_cont_ptr += would_write;

                            // provide size in bytes of the file
                            would_write = snprintf (new_cont + new_cont_ptr, avail = VV_MAX_SIZE_OF_URL - new_cont_ptr - 2, "%s_size=%lld&", name, multi_ctype_len);
                            if (would_write  >= avail)
                            {
                                vely_bad_request();
                                vely_report_error ("Web input larger than the limit of [%d] bytes (6)", VV_MAX_SIZE_OF_URL);
                            }
                            new_cont_ptr += would_write;

                        }
                        else
                        {
                            // no file uploaded, just empty filename as an indicator
                            num would_write = snprintf (new_cont + new_cont_ptr, avail = VV_MAX_SIZE_OF_URL - new_cont_ptr - 2, "%s_filename=&", name);
                            if (would_write >= avail)
                            {
                                vely_bad_request();
                                vely_report_error ("Web input larger than the limit of [%d] bytes (8)", VV_MAX_SIZE_OF_URL);
                            }
                            new_cont_ptr += would_write;
                        }
                    }

                }
                if (new_cont_ptr > 0) 
                {
                    new_cont_ptr--;
                    new_cont[new_cont_ptr] = 0; // the extra '&' which is always appended
                }

                content = new_cont; // have URL (built) and pass it along as if it were a regular URL GET
                cont_len_byte = new_cont_ptr;
            }
            else
            {
                // if body is a query string, then it should be added to whatever query string is in URL
                // if not a query string, then content from body is ignored
                if (!is_query_string) 
                {
                    cont_len_byte = 0; // if there is any query string after this, those params are on their own,
                    // Must have 2 extra bytes b/c for URL two nulls in a row signify the end of name/value pairs
                    // which are normally separated by a null
                    content = (char*)vely_malloc (cont_len_byte + 2);
                    content[0] = 0;
                }
            }
        }
        // Empty content type is allowed if content length is zero. This is say for REST when
        // POST is issued with just a URL (no payload, so no need for content type)
        else if (cont_len_byte != 0)
        {
            vely_bad_request();
            vely_report_error ("Content-length found without Content-type");
            return 0;
        }
    }
    else 
    {
        // for now request method must be specified and must be either GET POST PUT PATCH or DELETE
        vely_bad_request();
        vely_report_error ("Unsupported request method [%s]", req_method);
        return 0;
    }

    //
    // At this point content is what's from urlencoded body and cont_len_byte is the index of 0 byte at then 
    // regardless if this is multipart or not. This is for any method (get, post, put, delete, patch)
    //

    // if method,input used, then we only take input for GET, not POST
    // because we convert any POST to GET in the previous code (or not and it remains body) and any files
    // are already saved as actual files
    if (method != NULL)
    {
        qry = input;
    }
    else
    {
        // get GET/DELETE query string
        // POST/PATCH/PUT request can also have additional query string in URL string. Add it if present
        qry = vely_getenv ("QUERY_STRING");
    }

    if (qry != NULL && qry[0] != 0)
    {
        // cont_len_byte is the index where this query string is to be copied, so length alloced is cont_len_byte+1
        num ql = (num)strlen (qry);
        content = (char*)vely_realloc(content, cont_len_byte + ql + 3); // 1 for & (below) and 1 for \0, 1 for good luck
        if (cont_len_byte > 0) 
        {
            content[cont_len_byte] = '&'; // add & to the end of existing URL, if any
            cont_len_byte++; // and move one byte forward, to the point where we can copy query string from URL alone (as like GET)
        }
        memcpy (content + cont_len_byte, qry, ql+1);
        cont_len_byte = cont_len_byte+ql+1; // where any additional text would go to
    }
    // Now the URL is actually built from the POST request that can be parsed as an actual request


    // Convert URL format to a number of zero-delimited chunks
    // in form of name-value-name-value...
    // if name, convert - to _ so URL can use parameters like do-something or customer-id
    num j;
    num i;
    num had_equal = 0;
    bool isname = true;
    for (j = i = 0; content[i]; i++)
    {
        content[i] = (content[i] == '+' ? ' ' : content[i]);
        if (isname && content[i] == '-') content[j++] = '_';
        else if (content[i] == '%')
        {
            content[j++] = VV_CHAR_FROM_HEX (content[i+1])*16+
                VV_CHAR_FROM_HEX (content[i+2]);
            i += 2;
        }
        else
        {
            if (content[i] == '&')
            {
                if (had_equal == 0)
                {
                    vely_bad_request();
                    vely_report_error ("Malformed URL request, encountered ampersand without prior name=value");
                }
                content[j++] = 0;
                had_equal = 0;
                isname = true;
            }
            else if (content[i] == '=')
            {
                had_equal = 1;
                (req->ip.num_of_input_params)++;
                content[j++] = 0;
                isname = false;
            }
            else
                content[j++] = content[i];
        }
    }
    // two nulls in a row signify the end of parameters, this is why content must have at least 2 bytes
    content[j++] = 0;
    content[j] = 0;


    req->ip.names = (char**)vely_calloc (req->ip.num_of_input_params, sizeof (char*));
    req->ip.values = (char**)vely_calloc (req->ip.num_of_input_params, sizeof (char*));

    j = 0;
    num name_length;
    num value_len;
    for (i = 0; i < req->ip.num_of_input_params; i++)
    {
        name_length = strlen (content + j); 
        (req->ip.names)[i] = content +j;
        if (vely_is_valid_param_name (req->ip.names[i]) != 1)
        {
            vely_bad_request();
            vely_report_error ("Invalid input parameter name [%s], can contain alphanumeric characters or underscores only", req->ip.names[i]);
        }
        j += name_length+1;
        value_len = strlen (content + j); 
        (req->ip.values)[i] = content +j;
        num trimmed_len = value_len;
        (req->ip.values)[i] = vely_trim_ptr ((req->ip.values)[i], &trimmed_len);// trim the input parameter for whitespaces (both left and right)
        j += value_len+1;
        // THIS IS TO BE REMOVED WHEN/IF BACKWARD COMPATIBILITY WITH "REQ" ENDS
        if (!strcmp ((req->ip.names)[i], "req"))
        {
            req->name = (req->ip.values)[i]; // request name
        }
        // END OF WHAT'S TO BE REMOVED

        VV_TRACE ("Index: %lld, Name: %s, Value: %s", i, (req->ip.names)[i], (req->ip.values)[i]);
    }

    // 
    // Find out the full path used
    //
    // construct the full path, often path_info is empty for FastCGI
    // according to rfc3875, script_name must start with / and cannot end with / , and 
    // path_info is the same. Thus concatinating them is safe.
    // according to rfc2396, both hyphen and equal sign are allowed (among other chars) in the path,
    // however, hyphen is common and equal sign may confuse some clients/servers and they may ignore it, 
    // encode it, or even unpredictably change it. Thus hyphen is used, because *all* clients and servers
    // process it correctly, and Vely input parameter can NOT have a hyphen. That fits perfectly.
    // script_name is leading, path_info follows
    // Per rfc3875, PATH_INFO values MUST be URL-decoded by the caller, thus nothing there to be decoded.
    // Also per same, PATH_INFO canNOT have "/" in it. SCRIPT_NAME must be a valid path, not URL-encoded, so nothing
    // to decode either, per rfc3875.
    char *script_name = vely_getenv ("SCRIPT_NAME");
    char *path_info = vely_getenv ("PATH_INFO");
    static char full_path[VV_MAX_PATH]; // this static is fine, it's filled every time, and its purpose is to keep the 
                                           // memory for path_req so it will be valid throughout request.
    // haven't seen this but just in case
    if (script_name[0] == 0) script_name = "/";
    if (path_info[0] == 0) path_info = "";
    // make full path
    snprintf (full_path, sizeof(full_path), "%s%s", script_name, path_info);

    //
    // URL path must start with application path which at minimum is application name
    //

    // Check if URL app path actually matches the leading portion of the URL path
    int rlen = strlen (vely_app_path);
    if (!strncasecmp (vely_app_path, full_path, rlen))
    {
        char *p = full_path+rlen;
        // find all path segments
        num iplen = req->ip.num_of_input_params; // currently how many name/value allocated
        num block_ip = 10; // how many input params to add to memory reserved, so it's not realloc-ed for each new one
        //
        // First /xyz/ after app-path is the request name
        // p is now /all/after/app-path
        // *p must be /
        // we check, app path NEVER ends with forward slash
        // *p must be / or there's no /request
        // the exception is normalized path, which is /xyz?... or /xyz/?... both are valid
        if (*p != '/' || (*p == '/' && *(p+1) == '?')) 
        {
            // there is no request name
            // THIS IS TO BE REMOVED WHEN/IF "REQ" BACKWARD COMPATIBILITY ENDS
            if (req->name[0] != 0) // "req" specified in query string, so backwards compatibility
            {
                p = NULL; // skip the while loop ahead
            }
            // END TO BE REMOVED FOR "REQ" BACKWARDS COMPATIBILITY
            else
            {
                vely_bad_request();
                vely_report_error ("URL path [%s] is missing a request name", full_path);
            }
        }
        bool leading = true; // leading part is just request
        while (p != NULL) 
        {
            num ipar_len = 0;
            char *ipar = p + 1; // one after /
            char *next_par = strchr (ipar, '/');
            if (next_par != NULL) 
            {
                *next_par = 0;
                ipar_len = (num)(next_par - ipar);
                p = next_par;
            } 
            else 
            {
                p = NULL;
                ipar_len = strlen (ipar);
            }
            char *name;
            char *value;
            if (leading)  // first /xyz/ is just a request, no value (i.e. value of "req" param)
            {
                leading = false;
                req->name = ipar; // request name
                num il;
                for (il = 0; il < ipar_len; il++) if (req->name[il] == '-') req->name[il] = '_';
                continue;
            }
            else
            {
                name = ipar;
                // subst _ for - in name
                num il;
                for (il = 0; il < ipar_len; il++) if (name[il] == '-') name[il] = '_';
                if (next_par == NULL)
                {
                    value = "true";
                }
                else
                {
                    *next_par = 0; // end the name
                    value = next_par + 1;
                    char *next_par = strchr (value, '/');
                    if (next_par != NULL) 
                    {
                        *next_par = 0;
                        p = next_par;
                    } else p = NULL;
                    // NO decoding value for any value in PATH_INFO and SCRIPT_NAME see above per rfc3875
                }
            }
            // this is /param/val
            req->ip.num_of_input_params++; // new param
            if (iplen <= req->ip.num_of_input_params) // add memory if needed
            {
                iplen += block_ip;
                block_ip += 4; // increase each new block by a fixed amount to avoid runaway realloc
                req->ip.names = (char**)vely_realloc (req->ip.names, iplen*sizeof (char*));
                req->ip.values = (char**)vely_realloc (req->ip.values, iplen*sizeof (char*));
            }
            // this name/value must be added
            req->ip.names[req->ip.num_of_input_params-1] = name;
            req->ip.values[req->ip.num_of_input_params-1] = value;
        }
    }
    else
    {
        vely_bad_request();
        vely_report_error ("URL path [%s] must begin with application path [%s]", full_path, vely_app_path);
    }


    return 1;
}

// 
// In URL list of inputs, set value for input name to val.
// req is the request structure.
// Returns >=0 index where it is in ip.values[] array 
// Value is set, and no copy of it is made. Use copy-string to make a copy if needed.
//
num vely_set_input (vely_input_req *req, char *name, char *val)
{
    VV_TRACE("");

    num i;
    VV_TRACE ("Number of input data [%lld], looking for [%s]", req->ip.num_of_input_params, name);
    for (i = 0; i < req->ip.num_of_input_params; i++)
    {
        if (!strcmp (req->ip.names[i], name))
        {
            VV_TRACE ("Found input [%s] at [%lld]", req->ip.values[i], i);
            req->ip.values[i] = val;
            return i;
        }
    }
    VV_TRACE ("Did not find input, create new one");
    // increase storage for input-params
    req->ip.num_of_input_params++;
    vely_managed(); // needs to be used when managed memory from startup is changed, save current memory mode (managed or unmanaged)
    req->ip.names = (char**)vely_realloc (req->ip.names, req->ip.num_of_input_params*sizeof (char*));
    req->ip.values = (char**)vely_realloc (req->ip.values, req->ip.num_of_input_params*sizeof (char*));
    vely_mrestore(); // needs to be used when managed memory from startup is changed, restore current memory mode
    // add new input param
    req->ip.names[req->ip.num_of_input_params-1] = name;
    req->ip.values[req->ip.num_of_input_params-1] = val;
    return req->ip.num_of_input_params-1;
}


// 
// In URL list of inputs, find an index for an input with a given name
// req is input request. 'name' is the name of input parameters. Search is
// case sensitive.
// is_task is true if this input-param is declared task - if input param not present, if-task
// will match ""
// if input-param not found, then task could be another one, i.e. if not found, it's not
// enforced to be that one. Even if set-input is used, that parameter is still task if it was 
// before, i.e. changing the value of input parameter doesn't affect it as a task.
// Returns value of parameters, or "" if not found.
//
char *vely_get_input_param (vely_input_req *req, char *name, bool is_task)
{
    VV_TRACE("");

    num i;
    VV_TRACE ("Number of input data [%lld], looking for [%s]", req->ip.num_of_input_params, name);
    for (i = 0; i < req->ip.num_of_input_params; i++)
    {
        if (!strcmp (req->ip.names[i], name))
        {
            VV_TRACE ("Found input [%s] at [%lld]", req->ip.values[i], i);
            if (is_task) req->task = i; // we can set index because input-params are never deleted, rather
                                      // they may be updated with set-param or added as well, but the index
                                      // to input-param is always the same.
            return req->ip.values[i];
        }
    }
    VV_TRACE ("Did not find input");
    if (is_task) req->task = -1; // task param not set, if-task will match ""
    return VV_EMPTY_STRING;
}


// 
// Copy 'value' to 'data' at offset 'off'. 
// Returns number of bytes written excluding zero at the end.
// 'data' will be a pointer to allocated data that has *data+(value at offset off of data)
// This is a base function used in other string manipulation routines.
//
num vely_copy_data_at_offset (char **data, num off, char *value)
{
    VV_TRACE ("");

    if (*data == NULL) 
    {
        assert (off==0);
        VV_STRDUP (*data, value);
        return 0;
    }
    else
    {
        if (*data == value) 
        {
            assert (off==0);
            return 0; // copying to itself, with SIGSEGV
        }
        if (value == NULL) value = "";
        num len_val = strlen (value);
        //
        // vely_realloc will handle if *data points to VV_EMPTY_STRING, i.e. if it's uninitialized
        //
        *data = vely_realloc (*data, off+len_val + 1);
        memcpy (*data+off, value, len_val+1);
        return len_val; // returns bytes written, not the new length
    }
}


// 
// Copy string from 'value' to 'data', with 'data' being the output pointer.
// Returns the number of bytes written excluding zero at the end.
//
num vely_copy_data (char **data, char *value)
{
    VV_TRACE ("");
    return vely_copy_data_at_offset(data, 0, value);
}


// 
// Check if string 's' is a number. Return 1 if it is, 0 if not.
// Number can have plus/minus in front and can have one dot somewhere
// in the middle. Outputs: 'prec' is precision: total number of digits, 'scale' is the number of 
// digits after the decimal point. If prec and scale aren't NULL, they are filled.
// Same for 'positive', if number is positive it is 1, otherwise 0.
//
num vely_is_number (char *s, num *prec, num *scale, num *positive)
{
    VV_TRACE("");
    num i = 0;
    if (prec != NULL ) *prec = 0;
    if (scale != NULL) *scale = 0;
    num dot_pos = 0;
    num sign = 0;
    if (positive!=NULL) *positive=1;
    while (s[i] != 0) 
    {
        if (isspace(s[i])) 
        {
            i++;
            continue;
        }
        if (!isdigit(s[i]))
        {
           if (s[i]=='+' || s[i]=='-')
           {
               if (i != 0)
               {
                    // + or - isn't the first
                    return 0;
               }
               else
               {
                   sign = 1;
                   if (s[i]=='-' && positive!=NULL) *positive = 0;
               }
           }
           else if (s[i]=='.' && i>0)
           {
               if (dot_pos > 0)
               {
                   // two dots
                   return 0;
               }
              dot_pos = i; 
           }
           else
           {
               return 0;
           }
        }
        i++;
    }
    if (dot_pos > 0)
    {
        num c_scale= i - dot_pos - 1;
        if (c_scale == 0)
        {
            // this is for example 1234. 
            // i.e. no digits after dot
            return 0;
        }
        if (scale != NULL) *scale = c_scale;
    }
    else
    {
        if (scale != NULL) *scale = 0;
    }
    if (dot_pos > 0) i--;
    if (sign > 0) i--;
    // for example in -123.4, c_prec would be be 4 because 6th byte would be zero and we decrease i
    // after the loop: If there is a single dot, we decrease it by 1. If there is a single +
    // or -, we decrease it by 1. The result is the precision.
    if (prec != NULL) *prec = i;
    if (i == 0) return 0; // no digits, not a number
    return 1;
}

// 
// Returns 1 if string 's' is a positive unsigned integer.
//
num vely_is_positive_num (char *s)
{
    VV_TRACE("");
    num i = 0;
    while (s[i] != 0) 
    {
        if (!isdigit(s[i]))
        {
           return 0;
        }
        i++;
    }
    return 1;
}

//
// Make sure argv[0] is correctly set to program name before executing - sometimes (not always) 
// but sometimes it is necessary for the executing program not to crash. 'program' is the full
// path of the executable. So if 'program' is '/a/b/c/d', then arg0 is 'd'
//
void vely_set_arg0 (char *program, char **arg0)
{
    VV_TRACE("");
    num i =strlen(program) - 1;
    assert (i >= 0); // program cannot be empty
    while (i >= 0)
    {
        if (program[i]=='/')
        {
            break;
        }
        i--;
    }
    *arg0 = program+i+1;
    VV_TRACE("Program name for execution is [%s]", *arg0);
}


//
// Read data from file descriptor ofd, into out_buf (which is allocated inside),
// and if out_buf!=NULL, put length there. The buffer starts very small and grows to 4K as needed,
// with block read of max 4K. Useful for reading a stream.
//
void vely_read_child (int ofd, char **out_buf, num *out_len)
{
    lseek (ofd, SEEK_SET, 0);
    // minimum allocation for dmalloc
#define EXEC_BLEN 32
    num tread = EXEC_BLEN;
    *out_buf = (char*) vely_malloc (tread); 
    num curr = 0;
    while (1) 
    {
        num rd = read (ofd, *out_buf + curr, tread - 1);
        if (rd == 0) break;
        if (rd == -1) vely_report_error ("Cannot read from program execution, error [%d], error text [%s]", errno, strerror(errno));
        curr += rd;
        if (rd < tread - 1) tread = EXEC_BLEN; // if reading close to end, or stalling, do not alloc another 4K chunk and waste it
                                       // rather start small again
        else {
            if (tread < 4096) tread *= 2; // go up to 4K blocks, but no more
        }
        *out_buf = (char*) vely_realloc (*out_buf, curr + tread); 
    }
    (*out_buf)[curr] = 0;
    if (out_len != NULL) *out_len = curr; 
}



// 
// Execute program 
// It can be input data or input file, and with output data or output file. So input data can be mixed with output file etc.
// With command-line arguments and with capture of its exist status, 
// prg is the full path of program or just program name if in PATH (note PATH does not include current directory in latest Linux)
// If *fout is NULL and out_buf==NULL, a temporary file will be created
// Returns exit status of program execution.
// 
// 1. takes input arguments 'argv' with total number of them 'num_args'.
//    (argv[num_args] must be NULL, so if arguments are 'x' and 'y', then argv[0] is 'x', argv[1] is 'y' and 
//     argv[2] is NULL and num_args is 2)
// 2. input string 'inp' of length 'inp_len' is passed as stdin to the program. If inp is NULL or inp[0]==0,
//      then, do NOT write to stdin of a program being executed, rather use fin as input file, and if that is NULL too, then there is
//      no input to the program. If inp_len is 0, then strlen(inp) is used for length, assuming inp is not 
//      empty (NULL or inp[0]==0).
// 3. output of the program (both stdout and stderr) is saved in 'out_buf' if out_buf!=NULL. out_buf 
//      can be NULL, in which case no output is passed back out to string. If out_len is -1 (or negative), then allocate mem for 
//      the entire output. On return out_len is the actual number of bytes read. If out_buf is NULL and *fout!=NULL, then
//      output goes to this file.
// 4. ferr, err_buf and err_len are for stderr, and follow the same rules as fout, out_buf and out_len. If all ferr and *err_buff are NULL,
//      then both stdout/stderr go to fout/out_buf.
// Return value is the exit status.
//
num vely_exec_program (char *prg, char *argv[], num num_args, FILE *fin, FILE **fout, FILE **ferr, char *inp, num inp_len, char **out_buf, num *out_len, char **err_buf)
{
    VV_TRACE("");
    assert (argv);
    assert (num_args > 0);
    assert (prg);

    if (argv[num_args] != NULL)
    {
        vely_report_error ("Number of arguments does not match last NULL");
    }
    // make sure argv[0] is program name
    vely_set_arg0 (prg, &(argv[0]));

    //make sure we either use a file or a string (be it for input, output or error)
    if (fin != NULL && inp != NULL) vely_report_error ("Cannot use both input-file and input in exec-program");
    if (*fout != NULL && out_buf != NULL) vely_report_error ("Cannot use both output-file and output in exec-program");
    if (*ferr != NULL && err_buf != NULL) vely_report_error ("Cannot use both error-file and error in exec-program");

    pid_t pid;

    // redirect output to nothing if no file provided provided, and if out_buf==NULL
    if (*fout == NULL && out_buf == NULL) 
    {
        *fout = fopen ("/dev/null", "w");
        if (*fout==NULL) vely_report_error ("Cannot redirect output to /dev/null, error [%s]", strerror(errno));
    }
    // redirect error to nothing if no file provided provided, and if out_buf==NULL
    if (*ferr == NULL && err_buf == NULL) 
    {
        *ferr = fopen ("/dev/null", "w");
        if (*ferr==NULL) vely_report_error ("Cannot redirect output to /dev/null, error [%s]", strerror(errno));
    }


    int pipe2child[2] = {-1,-1}; // input from parent 2 child
    int pipe2parent[2] = {-1,-1}; // output from child 2 parent
    int errpipe[2] = {-1, -1}; // from child 2 parent (stderr only)

    if (inp != NULL)
    {
        // pipe from owner to child
        if (pipe(pipe2child) == -1) vely_report_error ("Cannot create pipes, error [%s]", strerror(errno));
    }
    if (out_buf != NULL)
    {
        // pipe from child to parent
        if (pipe(pipe2parent) == -1) vely_report_error ("Cannot create pipes, error [%s]", strerror(errno));
    }
    if (err_buf != NULL)
    {
        // stderr pipe from child to parent
        if (pipe(errpipe) == -1) vely_report_error ("Cannot create pipes, error [%s]", strerror(errno));
    }

    // The order in which pipes/files are closed is important. For example, not closing reading end of pipe in process
    // that only writes, makes for a possiblity that accidentaly process may read its own writing. What is not used, should
    // be closed right away.
    // Also, "closing a descriptor", such as pipe end, does NOT mean the file associated with it is closed. If it was dup2-ed
    // before, the file is still open. It is open for as long as there is a descriptor that still points to it. So "closing"
    // really just means dereferencing, while true "closing" happens when all descriptors that reference the stream/file are closed.
    
    // note dup2() cannot get EINTR (interrrupt) on LINUX, so no checking
    pid = fork();
    if (pid == -1) 
    { 
        vely_report_error ("Cannot create child, error [%s]", strerror(errno));
    }
    else if (pid == 0) 
    {
        // child. Note we check and can't have *fout!=NULL && out_buf!=NULL, and we can't have
        // fin!=NULL && inp!=NULL and ferr!=NULL && err_buf!=NULL. So below, for example, either output
        // goes to a file, or to a string but NOT both.
        // make stdout, stderr and stdin be substitued for files specified
        if (*fout != NULL) {dup2(fileno(*fout),STDOUT_FILENO); fclose(*fout);}  
        if (*ferr != NULL) {dup2(fileno(*ferr),STDERR_FILENO); fclose(*ferr);} 
        if (fin != NULL) {dup2(fileno(fin), STDIN_FILENO); fclose(fin);}

        // close read-end of stdout child2parent, so this pipe isn't accidentally reading program's own output
        if (pipe2parent[0] != -1) close(pipe2parent[0]);    
        if (out_buf != NULL) dup2(pipe2parent[1],STDOUT_FILENO); 
        // close read-end of stderr child2parent, so this pipe isn't accidentally reading program's own error stream
        if (errpipe[0] != -1) close(errpipe[0]);    
        if (err_buf != NULL) dup2(errpipe[1],STDERR_FILENO); 
        // close write-end of stdin parent2child, so this pipe isn't accidentally writing to program's own input
        if (pipe2child[1] != -1) close(pipe2child[1]);
        if (inp != NULL) dup2(pipe2child[0],STDIN_FILENO);  

        // once the pipes are pointed to by stdout, stderr and stdin, close original descriptors; those pipes are still open.
        if (errpipe[1] != -1) close(errpipe[1]);
        if (pipe2child[0] != -1) close(pipe2child[0]);    
        if (pipe2parent[1] != -1) close(pipe2parent[1]);

        int res = execvp(prg, (char *const*)argv); // will send data to parent
        if (res) // we're here only if execvp failed, otherwise execvp doesn't return
        {
            VV_FATAL ("Failed to start program [%s], error [%m]", prg); // failed to exec, do not flush stdout twice (if exit()
                // were called, it would call atexit() of parent and flush its stdout
                // and html output to apache would be duplicated!
            exit(-1); // exit child that failed to execvp
        }
    }
    else 
    { 
        // parent
        // close pipes not used, so parent doesn't accidentaly read/write from/to itself (see above comments for child)
        if (pipe2child[0] != -1) close(pipe2child[0]);    
        if (pipe2parent[1] != -1) close(pipe2parent[1]);
        if (errpipe[1] != -1) close(errpipe[1]);

        // these are used by child only; close them. They just redirect child to files. See comment above for child, where
        // it is discussed how we can either have files OR strings for these, but NOT both.
        if (fin != NULL) fclose (fin);
        if (*fout != NULL) fclose (*fout);
        if (*ferr != NULL) fclose (*ferr);

        // if there is no input, do NOT send anything to stdin of executing program
        if (inp!=NULL && inp[0]!=0)
        {
            // Send string (or binary) to child input, this is the write end of that pipe, the child reads from pipe2child[0]
            if (inp_len == 0) inp_len = strlen (inp);
            if (write(pipe2child[1], inp, inp_len) != inp_len) 
            {
                vely_report_error ("Cannot provide input data [%s] of length [%lld] to program [%s], error [%s]", inp, inp_len, prg, strerror (errno));
            }
        }
        if (pipe2child[1] != -1) close(pipe2child[1]); // close the pipe used to write to child's input

        // these must be BEFORE child exits. If this were after wait() below, parent would hang because child's (writing) end of 
        // pipe is now undefined, so can't get anything
        if (out_buf != NULL) vely_read_child (pipe2parent[0], out_buf, out_len); // read child's stdout
        if (err_buf != NULL) vely_read_child (errpipe[0], err_buf, NULL); // read child's stdout

        int st;
        while (wait (&st) != pid) ; // wait until child finishes

        // close pipes used above so we don't accumulate open descriptors. All pipes/files we opened MUST be totally closed.
        if (pipe2parent[0] != -1) close(pipe2parent[0]);    
        if (errpipe[0] != -1) close(errpipe[0]);    

        if (WIFEXITED(st)) 
        {
            return (num)WEXITSTATUS(st);// exit status if child exited with exit() or such
        }
        else if (WIFSIGNALED(st)) 
        {
            return (num)WTERMSIG(st)+128; // exit status 128+signal if child killed with signal
        }
        else
        {
            return (num)126; // any other kind of child termination
        }
    }
    return 1; // will never actually reach here, only for compiler joy

}


// 
// This is to disable any output, but not for strings. This is typically used with finish-request.
// This is also done to output binary files and prevent other output going out.
//
void vely_disable_output()
{
    VV_TRACE ("");
    vely_get_config()->ctx.req->disable_output = 1;
}



// 
// Output string 's'. 'enc_type' is either VV_WEB, VV_URL or 
// VV_NOENC. Returns number of bytes written.
// "Output" means either to the browser (web output) or to the string.
// For example, if within write-string statement, it's to the string, 
// otherwise to the web (unless HTML output is disabled).
//
// There are only 2 output functions: vely_puts and vely_printf, and for string output they both
// call vely_puts_to_string(). NO OTHER way of output should be present and NOTHING
// else should call vely_puts_to_string.
//
//
num vely_puts (num enc_type, char *s)
{
    VV_TRACE ("");

    if (vely_validate_output()!=1) vely_report_error ("Cannot send file because output is disabled, or file already output");

    vely_config *pc = vely_get_config();

    num buf_pos_start = VV_WRSTR_POS;
    VV_UNUSED(buf_pos_start); // used for tracing only
    num vLen = strlen(s);
    num res = 0;
    if (enc_type==VV_NOENC)
    { // no encoding
        if (VV_WRSTR_CUR == -1) // this is to the web
        {
            num res = vely_write_web (false, pc, s, vLen);
            if (res < 0) VV_TRACE ("Error in writing direct, error [%s]", strerror(errno));
            else VV_TRACE("Wrote direct [%lld] bytes", res);
            return res;
        } 
        else 
        { // this is to a string
            return vely_puts_to_string (s, vLen); 
        }
    }
    //
    // if we're encoding, and not buffering, write directly to web socket
    //
    if (VV_WRSTR_CUR == -1)
    {
        char *write_to = (char*)vely_malloc (VV_MAX_ENC_BLOWUP(vLen));
        num total_written = vely_encode_base (enc_type, s, vLen, &(write_to), 0);
        res = vely_write_web (false, pc, write_to, total_written);
        if (res < 0) VV_TRACE ("Error in writing direct (puts) of length [%lld], error [%s]", total_written, strerror(errno));
        else VV_TRACE("Wrote direct (puts) [%lld] bytes", res);
        return res;
    }
    //
    // writing to string
    //
    while (1)
    {
        // resize buffer to needed size and encode directly into the buffer
        // without having to memcpy needlessly
        int bup;
        if ((bup = VV_MAX_ENC_BLOWUP(vLen)) > VV_WRSTR_LEN -1-VV_WRSTR_POS)
        {
            VV_WRSTR_LEN = VV_WRSTR_LEN + bup + VV_WRSTR_ADD;
            VV_WRSTR_ADJMEM(VV_WRSTR_ADD);
            VV_WRSTR_BUF= (char*)vely_realloc (VV_WRSTR_BUF, VV_WRSTR_LEN);
            continue;
        }
        else
        {
            char *write_to = VV_WRSTR_BUF+VV_WRSTR_POS;
            res = vely_encode_base (enc_type, s, vLen, &(write_to), 0);
            VV_WRSTR_POS += res;
        }
        break;
    }
    VV_TRACE ("HTML>> [%s]", VV_WRSTR_BUF+ buf_pos_start);
    return res;
}


// 
// Check if output can happen, if it can, make sure output buffer is present
// and if it needs flushing, flush it.
//
num vely_validate_output ()
{
    VV_TRACE("");

    // if output is disabled, do NOT waste time printing to the bufer!!
    // UNLESS this is a write to a string, in which case write it!!
    // If we allow writing, then say output is disabled and we write and then flush happens at any time
    // and the program CRASHES because header wasn't sent!!!!
    vely_config *pc = vely_get_config();
    if (pc->ctx.req->disable_output == 1 && VV_WRSTR_CUR == -1) return  0;

    return 1;
}

// 
// ** IMPORTANT: THis function is one of the TWO outputters (this and vely_puts) meaning 
// these are SOLE callers of vely_puts_to_string. This is to ensure there is no circumvention of 
// disabled output or anything else.
// Returns total number of bytes written.
// iserr is true if output goes to stderr, otherwise stdout - this is for web output only.
//
// Outputs to web or to strings. enc_type is VV_NOENC, VV_WEB, VV_URL. 
//
num vely_printf (bool iserr, num enc_type, char *format, ...)
{
    VV_TRACE ("");
    
    if (vely_validate_output()!=1) vely_report_error ("Cannot send file because output is disabled, or file already output");
    vely_config *pc = vely_get_config();
    num tot_written = 0;

    va_list args;
    va_start (args, format);

    if (VV_WRSTR_CUR == -1)
    {

        num ebuf_size=256;
        char *ebuf = (char*)vely_malloc (ebuf_size);
        while (1)
        {
            tot_written = vsnprintf (ebuf, ebuf_size, format, args);
            if (tot_written >= ebuf_size)
            {
                ebuf_size += tot_written;
                ebuf = vely_realloc (ebuf, ebuf_size);
                va_end (args); // must restart the va_list before retrying!
                va_start (args, format);
                continue;
            }
            else 
            {
                break;
            }
        }
        va_end (args);
        if (enc_type == VV_WEB || enc_type == VV_URL)
        {
            char *final_out = NULL;
            // here final_out is allocated in vely_encode, and so is free 3 lines down
            num final_len = vely_encode (enc_type, ebuf, -1, &final_out);
            tot_written = vely_write_web (iserr, pc, final_out, final_len);
            vely_free (final_out);
        }
        else
        {
            tot_written = vely_write_web (iserr, pc, ebuf, tot_written);
        }
        vely_free (ebuf); // so there is no leak when unmanaged memory
        if (tot_written < 0) VV_TRACE ("Error in writing direct, error [%s]", strerror(errno));
        else VV_TRACE("Wrote direct [%lld] bytes", tot_written);
        return tot_written;
    }


    // This is writing to string
    // Since this is always VV_NOENC, we can print this (snprintf) directly , avoiding extra copying of buffer
    while (1)
    {
        // tot_written (i.e. the return value) is what would have been written if there was enough space EXCLUDING the null byte.
        // bytes_left (i.e. second parameter) is the number of bytes available to write INCLUDING the null byte.
        // So if bytes_left is 255, and the return value is 255, it means the space needed was 255+1 (because the return value 
        // excludes the null byte!), so if return value is grear OR EQUAL to the size available (ie. second parameter), then we need to realloc.
        num bytes_left = VV_WRSTR_LEN - VV_WRSTR_POS;
        tot_written = vsnprintf (VV_WRSTR_BUF+ VV_WRSTR_POS, bytes_left, format, args);
        if (tot_written >= bytes_left)
        {
            VV_WRSTR_LEN += tot_written + VV_WRSTR_ADD;
            VV_WRSTR_ADJMEM(VV_WRSTR_ADD);
            VV_WRSTR_BUF= vely_realloc (VV_WRSTR_BUF, VV_WRSTR_LEN);
            va_end (args); // must restart the va_list before retrying!
            va_start (args, format);
            continue;
        }
        else 
        {
            // buf_pos does NOT include trailing zero, even though we put it there. 
            VV_WRSTR_POS += tot_written;
            break;
        }
    }
    va_end (args);
    switch (enc_type)
    {
        case VV_URL:
        case VV_WEB:; // has to have ; because declaration (char *final... CANNOT be
                      // after label (which is case ...:)
            char *final_out = NULL;
            VV_WRSTR_POS-=tot_written;
            num final_len = vely_encode (enc_type, VV_WRSTR_BUF+VV_WRSTR_POS, -1, &final_out);
            tot_written = vely_puts_to_string (final_out, final_len);
            vely_free (final_out);
            break;
        case VV_NOENC:
            // nothing to do, what's printed to output buffer is there to stay unchanged
            break;
        default: vely_report_error ("Unknown encoding type [%lld]", enc_type);
    }
    return tot_written;
}

// ** IMPORTANT:
// ** This function can be called only from vely_printf or vely_puts!! The reason is this way we control
// ** exactly what goes out and there's nothing to circumvent that.
//
// Outputs to string ONLY - for outputting to the web, see vely_write_web().
// The memory can grow as needed to accomodate unlimited writes (but limited by virtual memory).
//
// 'final_out' is the string being output, and final_len is its length. Returns
// number of bytes written excluding zero at the end.
//
// NOTE: this might be optimized to account for immutable strings, so they are NOT copied
// every time a buffer is built. By using an array of pointers to immutable strings coupled with
// an array of where in the final buffer those go to, data can be sent without copying (this
// wouldn't work for string construction).
//
num vely_puts_to_string (char *final_out, num final_len)
{
    VV_TRACE("");


    // This is writing to string.
    // here we add to the buffer, which may be periodically flushed, for example
    // if program writes a huge report and then sends it somewhere, but doesn't display
    // to web, this output can be huge and more than available memory.
    num buf_pos_start = VV_WRSTR_POS;
    VV_UNUSED(buf_pos_start); // used for tracing only
    num res = 0;
    while (1)
    {
        // if we need to write more than currently allocated memory, add more
        // final_len is the length of string, we check we have final_len+1 bytes left (at least)
        if (final_len > VV_WRSTR_LEN -1-VV_WRSTR_POS)
        {
            VV_WRSTR_LEN = VV_WRSTR_LEN + final_len + VV_WRSTR_ADD;
            VV_WRSTR_ADJMEM(VV_WRSTR_ADD);
            VV_WRSTR_BUF= (char*)vely_realloc (VV_WRSTR_BUF, VV_WRSTR_LEN);
            continue;
        }
        else
        {
            memcpy (VV_WRSTR_BUF+ VV_WRSTR_POS, final_out, final_len + 1);
            VV_WRSTR_POS += final_len;
            res = final_len;
        }
        break;
    }
    if (res == 0) return 0; // return number of bytes written, minus null at the end
    VV_TRACE ("HTML>> [%s]", VV_WRSTR_BUF+ buf_pos_start);
    return res;
}


// 
// Shut down the request, 'giu' is the request. 
// This will flush any outstanding header to the web. If giu or program context is NULL, it produces a fatal error.
// If command line program, set the flag to end it after this request
//
void vely_shut(vely_input_req *giu)
{

    // vely_shut can be called from vely_report_error AND at the end of vely_main
    // it should not be called twice
    if (giu != NULL && giu->is_shut ==1) return;

    if (giu != NULL) giu->is_shut = 1;

    // if program ended before header could have been finished, finish the header
    if (giu != NULL && giu->sent_header ==1 && giu->data_was_output == 0) vely_gen_header_end (); // send cookies and \r\n divider

    VV_TRACE("Shutting down");
    if (giu == NULL)
    {
        VV_FATAL ("Shutting down, but request handler is NULL");
    }
    vely_config *pc = vely_get_config();
    if (pc == NULL)
    {
        VV_FATAL ("Shutting down, but program context is NULL");
    }
#ifdef VV_COMMAND
    vely_end_program = 1; // end program for command line
#endif

}


//
// Initialize reply/request html header. Default is cached forever (practically).
// 'header' is the html header structure. init_type: VV_HEADER_FILE means caching (files),
// VV_HEADER_PAGE means dynamic page.
// is_request is 1 if this is request header, 0 if reply
//
void vely_init_header (vely_header *header, num init_type, char is_request)
{
    VV_TRACE("");
    char const *errinit = "Unknown initialization type argument";
    if (init_type == VV_HEADER_FILE)
    {
        header->etag=1; // send etag by default, so even if we cache forever, but browser decides it can't, it can still benefit from etag
    }
    else if (init_type == VV_HEADER_PAGE)
    {
        header->etag=0; // no etag for generated page
    }
    else
    {
        vely_report_error ("%s",errinit);
    }

    if (is_request == 0) header->ctype="text/html;charset=utf-8"; // must always be set for reply
    else header->ctype=NULL; // if request header, don't set, will only be set if content-type used in call-web

    header->disp=NULL; // default is show object, not download
    header->file_name=NULL; // this is only if disp is not NULL
    // No status if set to 0
    header->status_id=0;
    header->status_text=NULL;
    // Default header for non-dynamic content such as images, or in general documents.
    // We deliver documents based on a database ID number which never changes if the
    // document doesn't. Any change and the ID changes too. 
    if (init_type == VV_HEADER_FILE)
    {
        header->cache_control= "public, max-age=2000000000, post-check=2000000000, pre-check=2000000000";  // default is cache forever (actually 53 years) - we are staying within an signed int, so to work anywhere
    }
    else if (init_type == VV_HEADER_PAGE)
    {
        header->cache_control= "max-age=0, no-cache";
    }
    else
    {
        vely_report_error ("%s",errinit);
    }
    num i;
    // any number of headers. The first from index 0 that has control or value NULL is where we stop looking. So no gaps.
    for (i = 0; i<VV_MAX_HTTP_HEADER; i++)
    {
        header->control[i]=NULL;
        header->value[i]=NULL;
    }
}



// 
// Create a new unique document and return FILE pointer to a newly created file associated
// with a document. 
// write_dir is the full path file name for which FILE * was opened. is_temp is 1 for temp table, 0 for documents.
// Returns FILE* to the file opened.
// The goal is to create a file and have a unique ID for it that can be used for future tracking and use
// of the file.
//
FILE *vely_make_document (char **write_dir, num is_temp)
{
    VV_TRACE("");

    vely_config *pc = vely_get_config();

    char path[180];
    num file_size = 200;
    char *ufile = (char*)vely_malloc (file_size);
    char *rnd;
    vely_make_random (&rnd, 6, VV_RANDOM_NUM, false); // random of size 5 as maximum is VV_MAX_UPLOAD_DIR (40000), max 5 digits

    if (is_temp == 0)
    {
        snprintf (path, sizeof(path), "%s/%lld", pc->app.file_dir, atoll(rnd)%VV_MAX_UPLOAD_DIR);
    }
    else
    {
        snprintf (path, sizeof(path), "%s/t/%lld", pc->app.file_dir, atoll(rnd)%VV_MAX_UPLOAD_DIR);
    }
    snprintf (ufile, file_size, "%s/%lldXXXXXX", path, (num)getpid());
    vely_free (rnd);
    //
    // CANNOT USE RND BEYOND THIS POINT
    //

    // 
    // make directory based on random number
    // even if mkdir fails, maybe mkstemp will not, so do not error out
    //
    if (mkdir (path, 06770) != 0)
    {
        VV_TRACE ("mkdir [%s] errored with [%s], trying to create a file anyway", path, strerror (errno));

    }

    num fd;
    if ((fd = mkstemp (ufile)) == -1)
    {
        vely_report_error ("Cannot create unique file, error [%s]", strerror(errno));
    }
    VV_TRACE("Creating file [%s]", ufile);
    FILE *f = fdopen (fd, "w");
    if (f == NULL)
    {
        vely_report_error ("Cannot get file pointer from file descriptor [%lld], error [%s]", fd, strerror (errno));
    }
    *write_dir = ufile;
    return f;
}


//
// Upper-cases string 's', returns upped value as well
// 's' is both input and output param.
//
char *vely_upper(char *s)
{
    VV_TRACE("");
    num l = 0;
    while (s[l] != 0) {s[l] = toupper(s[l]); l++;}
    return s;
}

// 
// Lower-cases string 's' and returns it.
// 's' is both input and output param.
//
char *vely_lower(char *s)
{
    VV_TRACE("");
    num l = 0;
    while (s[l] != 0) {s[l] = tolower(s[l]); l++;}
    return s;
}

// 
// Copy file src to file dst. 
// Returns VV_ERR_OPEN if cannot open source, VV_ERR_CREATE if cannot open destination, VV_ERR_READ if cannot read source,
// VV_ERR_WRITE if cannot write destination, number of bytes copied if okay.
// Uses 8K buffer to copy file. 
//
num vely_copy_file (char *src, char *dst)
{
    VV_TRACE("");

    num f_src = open(src, O_RDONLY);
    if (f_src < 0) {
        VERR; 
        VV_TRACE ("Cannot open [%s] for reading, error [%s]", src, strerror(errno));
        return VV_ERR_OPEN;
    }
    num f_dst = open(dst, O_WRONLY|O_CREAT, S_IRWXU);
    if (f_dst < 0) 
    {
        VERR;
        VV_TRACE ("Cannot open [%s] for writing, error [%s]", dst, strerror(errno));
        close (f_src);
        return VV_ERR_CREATE;
    }
    char buf[8192];
    num total_written = 0;

    if (ftruncate64 (f_dst, 0) != 0) 
    {
        VERR;
        VV_TRACE ("Cannot read [%s], error [%s]", src, strerror(errno));
        close (f_src);
        close (f_dst);
        return VV_ERR_WRITE;
    }; // truncate destination just prior to write, not before
                            // otherwise copying smaller content into an existing file leaves the
                            // remainder of the existing data!!! (say copy "xy" to "xyz", the result is "xyz")
    while (1) 
    {
        ssize_t res = read(f_src, &buf[0], sizeof(buf));
        if (res == 0) break;
        if (res < 0) 
        {
            VERR;
            VV_TRACE ("Cannot read [%s], error [%s]", src, strerror(errno));
            close (f_src);
            close (f_dst);
            return VV_ERR_READ;
        }
        ssize_t rwrite= write(f_dst, &buf[0], res);
        if (rwrite != res) 
        {
            VERR;
            VV_TRACE ("Cannot write [%s], error [%s]", dst, strerror(errno));
            close(f_src);
            close(f_dst);
            return VV_ERR_WRITE;
        } else total_written += rwrite;
    }
    close (f_src);
    close (f_dst);
    return total_written;
}



// 
// Get base name of URL. If protocol is missing (such as http://), returns empty string.
// For example, for http://myserver.com/go.service?..., the base name is myserver.com
// Returns base name of 'url'.
//
char *vely_web_name(char *url)
{
    VV_TRACE("");
    char *prot = strstr(url,"://");
    if (prot==NULL) 
    {
        return "";
    }
    char *web_name=vely_strdup(prot+3);
    char *end = strchr (web_name,'/');
    if (end!=NULL)
    {
        *end = 0;
    }
    return web_name;
}

//
// Release split-string resources given by broken_ptr
//
void vely_delete_break_down (vely_split_str **broken_ptr)
{
    VV_TRACE("");
    vely_free ((*broken_ptr)->pieces);
    (*broken_ptr)->num_pieces = 0;
    vely_free (*broken_ptr);
    *broken_ptr = NULL;
}


// 
// Break down 'value' string into pieces, with 'delim' being the delimiter.
// For example, 'x+y+z' could be 'value' and 'delim' could be '+'. 
// The result is stored into datatype vely_split_str's variable 'broken' which is
// allocated if currently NULL.
// This variable has 'num_pieces' as a number of values broken into, and 
// 'pieces[]' array that holds this number of pieces.
void vely_break_down (char *value, char *delim, vely_split_str **broken_ptr)
{
    VV_TRACE("");

    // get object for parsing
    *(broken_ptr) = (vely_split_str*)vely_malloc (sizeof (vely_split_str));
    vely_split_str *broken = *broken_ptr;

    // setup memory for 128 pieces and expand later if necessary
#define MAX_BREAK_DOWN 128

    num tot_break = MAX_BREAK_DOWN;

    broken->pieces = (char**)vely_malloc(tot_break * sizeof(char*));

    num curr_break = 0;
    char *curr_value = value;
    num dlen = strlen (delim);
    char *kwd = NULL;

    while (1)
    {
        kwd = vely_find_keyword0 (curr_value, delim, 0, 0);
        (broken->pieces)[curr_break] = curr_value;

        // end when no delimiter found
        if (kwd==NULL) 
            break;
        else
        {
            curr_value = kwd + dlen;
            *kwd = 0;
        }

        // move forward, expand buffer if needed
        curr_break++;
        if (curr_break >= tot_break)
        {
            tot_break += MAX_BREAK_DOWN;
            broken->pieces = (char**) vely_realloc (broken->pieces, tot_break * sizeof(char*));
        }
    }
    broken->num_pieces = curr_break+1;
    num i;
    // trim all values and skip over quote, if present
    for (i = 0; i < broken->num_pieces; i++)
    {
        // left trim
        char *p = (broken->pieces)[i];
        while (isspace(*p)) p++;
        if (*p == '"') p++;
        // right trim
        char *ep = NULL;
        // position ep to 0 after the end of string
        if (i == broken->num_pieces -1) ep = p + strlen (p); else ep = (broken->pieces)[i+1]-dlen;
        if (ep != p) 
        {
            // if p isn't empty, i.e. if it doesn't point to 0
            ep--;
            // no need to check ep!=p because p is either non-space or equal to ep
            while (isspace(*ep)) ep--;
            if (*ep == '"') ep--;
            ep++; // ep points to last non-white-space, so zero out the next one
            *ep = 0;
        }
        (broken->pieces)[i] = p;
    }
}


//
// Returns time string (now, in the future, or the past)
//
// Input parameter timezone is the name of TZ (timezone) variable to be set. So to get GMT time
// then timezone should be "GMT", if it is Mountain Standard then it is "MST" etc.
//
// 'format' is the format according to strftime(). If NULL, system time suitable for cookies is used.
//
// Input parameters year,month,day,hour,min,sec are the time to add to current time (can be negative too).
// So for example ..(0,0,1,0,0,1) adds 1 day and 1 second to the current time, while
// .. (0,-1,0,0,0) is one month in the past.
//
// Returns time suitable for many purposes, including cookie time (expires).
//
// The format of the return string is thus important and must NOT be changed.
// This time MUST be system time. Used ONLY for system purposes, such as
// cookie time, which must be system since system delivers Date in HTTP header
// to browser, so we MUST use system time in browser as well.
//
// This function will RESTORE the timezone back to what it was when the program first started. So it
// will temporarily set TZ to timezone variable, but before it exits, it will restore TZ to what it was.
//
char *vely_time (char *timezone, char *format, num year, num month, num day, num hour, num min, num sec)
{
    VV_TRACE ("");

    char set_gm[200];
    
    // set timezone to be used
    snprintf (set_gm, sizeof(set_gm), "TZ=%s", timezone);

    //make sure timezone is always GMT prior to using time function
    putenv(set_gm);
    tzset();

    // get absolute time in seconds
    time_t t = time(NULL);
    struct tm tm = *localtime(&t);
    struct tm future;       /* as in future date */

    // get future time
    future.tm_sec = tm.tm_sec+sec;
    future.tm_min = tm.tm_min+min;;
    future.tm_hour = tm.tm_hour+hour;
    future.tm_mday = tm.tm_mday+day;
    future.tm_mon = tm.tm_mon+month;
    future.tm_year = tm.tm_year+year; // years into the future 
    future.tm_isdst = -1;          /* try automaitic, may not work only within 1 hour before DST switch and 1 hour after*/

    // verify time is correct
    t = mktime( &future );
    if ( -1 == t )
    {
        //
        // Set result of vely_get_tz to mutable char *, since putenv does NOT 
        // modify its parameter. The result of vely_get_tz must NOT be modified.
        //
        putenv((char*)vely_get_tz());
        tzset();
        vely_report_error ("Error converting [%d-%d-%d] to time_t time since Epoch", future.tm_mon + 1, future.tm_mday, future.tm_year + 1900);
    }

    // convert time into GMT string suitable for cookies (this function is for
    // cookies ONLY or for anything that needs GMT time in this format)
#define GMT_BUFFER_SIZE 50
    char *buffer=(char*)vely_malloc(GMT_BUFFER_SIZE);
    size_t time_succ = strftime(buffer,GMT_BUFFER_SIZE-1, format==NULL ? "%a, %d %b %Y %H:%M:%S %Z":format, &future);
    if (time_succ == 0)
    {
        vely_report_error ("Error in storing time to buffer, buffer is too small [%d]", GMT_BUFFER_SIZE);
    }
    
    // go back to default timezone. See above about casting vely_get_tz()
    // to (char*)
    putenv((char*)vely_get_tz());
    tzset();

    VV_TRACE("Time is [%s]", buffer);
    return buffer;
}



// Get string representation of integer 'var' into 'data'.
// Returns bytes written.
// 'data' will be a new pointer to allocated data.
//
num vely_copy_data_from_num (char **data, num val)
{
    VV_TRACE ("");
    char n[30];
    snprintf (n, sizeof (n), "%lld", val);
    return vely_copy_data (data, n);
}


//
// Set environment for URL passed to batch program from command line.
// Does the same as GET request from command line
// arg is the query string (i.e. 'req=request&par1=...')
//
void vely_set_env(char *arg)
{
    VV_TRACE("");
    putenv ("REQUEST_METHOD=GET");
    
    char req[4096];
    snprintf (req, sizeof(req), "QUERY_STRING=%s", arg);
    putenv (req);
}


//
// Return value of env variable var from OS only
// Return "" if not found.
//
char *vely_getenv_os (char *var)
{
    VV_TRACE("");
    char *v = secure_getenv (var);
    if (v == NULL) return ""; else return v;
}


//
// Return value of env variable var from web server only
//
char *vely_getenv (char *var)
{
    VV_TRACE("");
    return vely_gen_get_env(var);
}



//
// Write web output. However, if header not sent, error out or output trace warning that header/data MAY NOT be output. 
// The worst that happens is that headers and/or data are not output (but we trace this). We also document this.
// pc is vely configuration. The rest the same as vely_gen_write().
// iserr is true if output goes to stderr, otherwise stdout - this is for web output only.
// Returns the same as vely_gen_write()
//
num vely_write_web (bool iserr, vely_config *pc, char *s, num nbyte)
{
    VV_TRACE("");
    if (pc->ctx.req->sent_header == 1) 
    {
        // vely_gen_header_end() will set data_was_output to 1
        if (pc->ctx.req->data_was_output == 0) vely_gen_header_end (); // send cookies and \r\n divider
        return vely_gen_write (iserr, s, nbyte);  // send actual data
    } 
    else
    {
        if (pc->ctx.vely_report_error_is_in_report == 0) 
        {
            vely_report_error ("Attempting to write to web without outputting header first");
        }
        else
        {
            // this allows to send to stderr even if header not output
            VV_TRACE ("WARNING: writing to web even though header was not sent");
            return vely_gen_write (iserr, s, nbyte);
        }
        return 0; // just to satisfy the compiler, since we're never coming here
    }
    return 0; // just to satisfy the compiler, since we're never coming here
}

//
//
//
//
// Begin FastCGI IO
//
//
//
//
   

//
// Write data to web output
// write data s of length nbyte
// if is_error is 1, write to stderr, otherwise for 0, write to stdout
// returns number of bytes written or -1 if error
// returns nbyte if output is disallowed or if fcgi request not initialized
//
num vely_gen_write (bool is_error, char *s, num nbyte)
{
    VV_TRACE("");
    if (finished_output == 0) 
    {
#ifndef VV_COMMAND
        // when vely_fcgi_out is not NULL, neither are others like vely_fcgi_err
        if (vely_fcgi_out != NULL) 
        {
            if (FCGX_PutStr ((char*)s, nbyte, is_error ? vely_fcgi_err :vely_fcgi_out) != nbyte) return -1; else return nbyte;
        } else return nbyte;
#else
        if ((num)fwrite((char*)s, 1, nbyte, is_error ? stderr : stdout) != nbyte) return -1; else return nbyte;
#endif
    } else return nbyte;
}


//
// Get environment variable 
// n is the name of environment variable. Returns "" if not found.
//
char *vely_gen_get_env (char *n)
{
    VV_TRACE("");
    char *v;
    if (finished_output == 0) 
    {
#ifndef VV_COMMAND
        v = FCGX_GetParam (n, vely_fcgi_envp);
#else
        v = secure_getenv (n);
#endif
        if (v == NULL) return ""; else return v;
    } else return "";
}



//
// Set content length for web output
//
void vely_gen_set_content_length(char *v)
{
    VV_TRACE("");
    if (finished_output == 0 && vely_get_config()->ctx.req != NULL && vely_get_config()->ctx.req->silent == 0) 
#ifndef VV_COMMAND
        if (vely_fcgi_out != NULL) FCGX_FPrintF (vely_fcgi_out, "Content-length: %s\r\n", v);
#else
        fprintf (stdout, "Content-length: %s\r\n", v);
#endif
}



//
// Read data from web client (like POST upload for example)
// content is the data buffer where reading is done into; it must be allocated with at least
// len+1 bytes. len is the length to read. Returns 1 if okay, 0 if not.
//
num vely_gen_util_read (char *content, num len)
{
    VV_TRACE("");
    if (finished_output == 0) 
    {
        num bytes_read;
        num total_read = 0;
        while (total_read < len) {
#ifndef VV_COMMAND
            bytes_read = FCGX_GetStr (content + total_read, len - total_read, vely_fcgi_in);
#else
            bytes_read = (num)fread (content + total_read, 1, len - total_read, stdin);
#endif
            if (bytes_read == 0)
            {
                return 0; // could not read
            } 
            total_read += bytes_read;
        }
        content[len] = 0;
        return 1;
    }
    else
    {
        content[0] = 0;
        return 1;
    }
}


//
// Add header for web output
// RIght now, adding is the same as setting; in the future it should be different
// (adding always adds, setting replaces or adds if doesn't exist)
// n is the header name, v is value.
//
void vely_gen_add_header (char *n, char *v)
{
    VV_TRACE("");
    if (finished_output == 0 && vely_get_config()->ctx.req != NULL && vely_get_config()->ctx.req->silent == 0) 
#ifndef VV_COMMAND
        if (vely_fcgi_out != NULL) FCGX_FPrintF (vely_fcgi_out, "%s: %s\r\n", n, v);
#else
        fprintf (stdout, "%s: %s\r\n", n, v);
#endif
}

// 
// Send new line between header and body.
// This will send any cookies as well. It will remember that this was done and if called again for the
// same request, it will not perform these actions again, as they need to be done only once per request.
//
void vely_gen_header_end ()
{
    VV_TRACE("");
    // because this is done for all output, including errors and such, make sure req is not NULL,
    // also this output was not already done, and also this isn't silent header.
    // but do set data_was_output to 1 even if silent, so this function doesn't get called needlessly
    if (vely_get_config()->ctx.req != NULL && vely_get_config()->ctx.req->data_was_output == 0)
    {
        if (vely_get_config()->ctx.req->silent == 0)
        {
            // Cookies 
            num ci;
            for (ci = 0; ci < vely_get_config()->ctx.req->num_of_cookies; ci++)
            {
                // we send back ONLY cookies set by set-cookie or delete-cookie. Cookies we received and are there
                // but were NOT changed, we do NOT send back because they already exist in the browser. Plus we do NOT
                // keep expired and path, so we would not know to send it back the way it was.
                if (vely_get_config()->ctx.req->cookies[ci].is_set_by_program == 1)
                {
                    VV_TRACE("Cookie sent to browser is [%s]", vely_get_config()->ctx.req->cookies[ci].data);
                    vely_gen_add_header ("Set-Cookie", vely_get_config()->ctx.req->cookies[ci].data);
                }
            }
            // send final \r\n to mark the end of headers
            vely_gen_write (false, "\r\n", 2);
        }
        vely_get_config()->ctx.req->data_was_output = 1; // once data was output, do not output end of header again 
    }
}

//
// Set content type for web output
// v is content type (text/html for instance)
//
void vely_gen_set_content_type(char *v)
{
    VV_TRACE("");
    if (finished_output == 0 && vely_get_config()->ctx.req != NULL && vely_get_config()->ctx.req->silent == 0) 
#ifndef VV_COMMAND
        if (vely_fcgi_out != NULL) FCGX_FPrintF (vely_fcgi_out, "Content-type: %s\r\n", v);
#else
        fprintf (stdout, "Content-type: %s\r\n", v);
#endif
}


//
// FLush any fcgi output
//
void vely_flush_out(void)
{
    VV_TRACE("");
#ifndef VV_COMMAND
    if (vely_fcgi_out != NULL) FCGX_FFlush (vely_fcgi_out);
    if (vely_fcgi_err != NULL) FCGX_FFlush (vely_fcgi_err);
#else
    fflush (stdout);
    fflush (stderr); // not needed as it's done automatically; still.
#endif
}

//
// This is called for finish-output. It is server push of any output. After this, no 
// output already done is outstanding in the buffers.
//
void vely_FCGI_Finish (void)
{
    VV_TRACE("");
//    No finish, since we're in the loop, and once we exit it doesn't matter. 
//    FCGX_Accept will free all the data from the previous request.

    if (finished_output == 0) 
    {
#ifndef VV_COMMAND
        // set status both stdout and err. FCGI says the code will go with the last
        // closed/flushed stream. Given they can be intermixed, it's not clear which one that is.
        // Looking at FCGI source code, seems stdout is where it should be but not sure.
        // So set both
        FCGX_SetExitStatus(vely_get_config()->ctx.req->ec, vely_fcgi_err);
        FCGX_SetExitStatus(vely_get_config()->ctx.req->ec, vely_fcgi_out);
        if (vely_fcgi_out != NULL) FCGX_FFlush (vely_fcgi_out);
        FCGX_Finish();
        // these are closed, so do not try to use them , use NULL as a flag
        vely_fcgi_in = NULL;
        vely_fcgi_out = NULL; 
        vely_fcgi_err = NULL;
#else
        fflush (stdout);
#endif
    }
    finished_output = 1; // this says that output is finished and this flag will guard against any
                         // output that follows (i.e. any output will not output anything)
}

//
//Exit, release resources. This is done in debug mode for testing only. The reason is if something fails,
//it doesn't really matter as it would be released anyway, and we'd miss the chance to properly return exit code, which
//is the only code currently that isn't DEBUG-shielded.
//
void vely_exit (void)
{
    vely_config *pc = vely_get_config();
    VV_UNUSED(pc); // for web server non-debug, not used

#ifdef VV_COMMAND
    // This must be *before* the cleaning down here, because ..req-> anything is managed
    // memory and will be destroyed in vely_done()!!!
    int retcode = (pc != NULL ? pc->ctx.req->ec : -1); // return code for command line program
#endif

#ifdef DEBUG
    vely_end_all_db (); // end any db connections
    vely_close_trace(); // shut off tracing if it was enabled

    if (pc != NULL && pc->ctx.db->conn != NULL) free (pc->ctx.db->conn); // free database list of descriptors

    // Vely memory shutdown and deallocation
    vely_done(); // clean up all the vely memory
    if (VV_EMPTY_STRING != NULL) {free (VV_EMPTY_STRING); VV_EMPTY_STRING=NULL;} // remove one byte of empty string used by velymem
#endif



#ifdef DEBUG
    // Free program context, the VERY LAST thing before exit
    free (pc->app.dbconf_dir);
    free (pc->app.home_dir);
    free (pc->app.file_dir);
    free (pc->app.trace_dir);
    if (pc != NULL) free(pc); 
#endif

    //
    // Final exit, nothing else must be here or after!!!
    //
#ifndef VV_COMMAND
    exit(0);
#else
    exit (retcode);
#endif
}

//
// Called at the top of the loop for FCGI. For command line, just return 1 and continue.
// Vely (managed) memory doesn't exist at this point, and pc->ctx.req is not allocated yet.
// So this must not do anything that may use such memory.
//
num vely_FCGI_Accept (void)
{
    VV_TRACE("");
#ifndef VV_COMMAND
    // 
    // By default, if server accepts client connection, but no data is to be read within 2 seconds,
    // it will close connection. This may cause unexpected failures. This was added to fcgi
    // recently and maybe I don't understand it, but doesn't seem quite useful.
    // Given the original problem of read hangs is no longer present (it was in linux 2.xx), 2 seconds
    // seem like could happen just from swaps. More reasonable value may be 5 seconds?
    // If the user sets "LIBFCGI_IS_AF_UNIX_KEEPER_POLL_TIMEOUT" then it will be used, it must be in 
    // milliseconds. Otherwise we set to 5 seconds.
    // 
    static bool init_once = false;
    if (!init_once)
    {
        init_once = true;
        char *already_set = vely_getenv_os ("LIBFCGI_IS_AF_UNIX_KEEPER_POLL_TIMEOUT");
        if (already_set[0] == 0) setenv ("LIBFCGI_IS_AF_UNIX_KEEPER_POLL_TIMEOUT","5000",1);
    }
    finished_output = 1; // if not, first time accept may try to write to FCGI; this is not clear why it would
                         // but regardless, this will prevent any output until the next request starts
                         // It may have been because vely_FCGI_Finish wasn't called properly until 16.9
                         // in the main loop at the end of each request (but rather it was called only for exiting)
    // FCGX_Accept calls FCGX_Finish first thing, so no need to call prior
    return FCGX_Accept(&vely_fcgi_in, &vely_fcgi_out, &vely_fcgi_err, &vely_fcgi_envp);
#else
    return 1; // just to enter the loop one time
#endif
}

//
// Set page status for web output
// st is the status number, line is the text (200, "OK" for example)
//
void vely_gen_set_status (num st, char *line)
{
    VV_TRACE("");
    if (finished_output == 0 && vely_get_config()->ctx.req != NULL && vely_get_config()->ctx.req->silent == 0) 
    {
        // fastcgi fprintf doesn't know %lld, and status text isn't taken
#ifndef VV_COMMAND
        if (vely_fcgi_out != NULL) FCGX_FPrintF (vely_fcgi_out, "Status: %ld %s\r\n", (long)st, line);
#else
        fprintf (stdout, "Status: %ld %s\r\n", (long)st, line);
#endif
    }

}


//
//
//
//
// End FastCGI IO
//
//
//
//

//
// Returns 0 if header sent. Set status of header to 'sent'
// pc is program context
//
num vely_header_err(vely_config *pc)
{
    VV_TRACE("");
    // if program receives TERM signal at acceping connection and thus exits, this will get called through stack/error reporting
    // and there is no request at that point (it's NULL)
    if (pc->ctx.req != NULL) 
    {
        if (pc->ctx.req->sent_header == 1) 
        {
            VV_TRACE("Header already sent, cannot send again");
            // if program ended before header could have been finished, finish the header
            if (pc->ctx.req->data_was_output == 0) vely_gen_header_end (); // send cookies and \r\n divider
            return 0;
        }
        pc->ctx.req->sent_header = 1;
    }
    return 1;
}


// 
// Output error message if Vely or Vely app encountered server or application error that cannot be fixed.
// This includes report-error, which is fatal.
// Output is 500 Internal Server Error
// No reason is sent to the client. Separately stderr is written with error message.
//
void vely_server_error ()
{
    VV_TRACE ("");
    vely_config *pc = vely_get_config();
    if (vely_header_err(pc) != 1) return;
    vely_gen_set_status (500, "Internal Server Error");
    vely_gen_set_content_type("text/html;charset=utf-8");
    vely_gen_header_end ();
}

// 
// Output error message if file requested (image, document.., a binary file in 
// general) could not be served.
// The reason is generally found in trace (if enabled)
//
void vely_cant_find_file ()
{
    VV_TRACE ("");
    vely_config *pc = vely_get_config();
    if (vely_header_err(pc) != 1) return;
    // never print out for batch mode
    vely_gen_set_status (404, "Not Found");
    vely_gen_set_content_type("text/html;charset=utf-8");
    vely_gen_header_end ();
}


// 
// Output a file. HTML output must be disabled for this to work. Typically, binary files
// (images, documents) are served to the web browser this way.
// fname is file name. 'header' is the header to be output (which must be set by the caller).
// This handles 'if-none-match' (timestamp) so that if web client already has this binary file,
// only the cache confirmation is sent back thus improving performance ('not modified' response).
// File must exist or it will return 'document requested not found' to the client.
//
void vely_out_file (char *fname, vely_header *header)
{
    VV_TRACE("");
    vely_config *pc = vely_get_config();

    // this must be FIRST, before vely_disable_output() as we check if being output
    if (vely_validate_output()!=1) vely_report_error ("Cannot send file because output is disabled, or file already output");

    if (pc->ctx.req->sent_header == 1) 
    {
        vely_report_error ("HTTP header has already been output; you must not output header prior to sending file");
    }


    vely_disable_output();

    if (strstr (fname, "..") != NULL)
    {
        //
        // We do not serve files with .. in them to avoid path traversal attacks. Files must
        // not traverse backwards EVER.
        //
        VV_TRACE("File path insecure, rejected");
        vely_cant_find_file();
        return;
    }


    struct stat attr;
    if (stat(fname, &attr) != 0)
    {
        VV_TRACE ("Cannot stat file name [%s], error [%s]", fname, strerror (errno));
        vely_cant_find_file();
        return;
    }
    long tstamp = (long)attr.st_mtime;

    FILE *f = vely_fopen (fname, "r");
    if (f == NULL)
    {
        VV_TRACE("Cannot open [%s], error [%s]", fname, strerror(errno));
        vely_cant_find_file();
        return;
    }
    else
    {
        //
        // We're using long longs throughout Vely
        //
        fseek(f, 0, SEEK_END);
        long fsize_l = ftell(f);
        if (fsize_l >= (long)INT_MAX)
        {
            VV_TRACE ("File size too long [%ld]", fsize_l);
            vely_cant_find_file();
            return;
        }
        num fsize = (num) fsize_l;
        fseek(f, 0, SEEK_SET);

        // 
        // check if file has already been delivered to the client
        //
        VV_TRACE("IfNoneMatch [%s], tstamp [%ld]", pc->ctx.req->if_none_match == NULL ? "" : pc->ctx.req->if_none_match, tstamp);
        if (pc->ctx.req->if_none_match != NULL && tstamp == atol(pc->ctx.req->if_none_match))
        {
            //
            // File NOT modified
            //
            VV_TRACE("File not modified! [%s]", fname);

            vely_gen_add_header ("Status", "304 Not Modified");
            if (header->cache_control!=NULL)
            {
                VV_TRACE("Setting cache [%s] for HTTP header (2)", header->cache_control);
                vely_gen_add_header ("Cache-Control", header->cache_control);
            }
            else
            {
                VV_TRACE("Setting no cache for HTTP header (3)");
                vely_gen_add_header ("Cache-Control", "max-age=0, no-cache");
                vely_gen_add_header ("Pragma", "no-cache");
            }
            // Before sending any contents, must send \n 
            vely_gen_header_end ();

            fclose (f);
            return;
        }

        // 
        // read file to be sent to the client
        //
        VV_TRACE("File read and to be sent [%s]", fname);
        char *str = vely_malloc(fsize + 1);
        if (fread(str, fsize, 1, f) != 1)
        {
            VV_TRACE ("Cannot read [%lld] bytes from file [%s], error [%s]", fsize, fname, strerror(errno));
            vely_cant_find_file();
            return;
        }
        fclose(f);

        //
        // The data read is in 'str' and the size of data is 'fsize'
        // which is what the following code expects.
        //


        char tm[50];
        char *val;

        if (header->etag==1)
        {
            VV_TRACE("Will send etag [%ld]", tstamp);
        }
        else
        {
            VV_TRACE("Will NOT send etag [%ld]", tstamp);
        }


        if (header->ctype[0] == 0)
        {
            //
            // Content type is missing, we assume it's HTML
            //
            VV_TRACE("Sending HTML, no content type");
            // html has to be empty because we have own set of headers

            if (header->etag==1)
            {
                snprintf (tm, sizeof(tm), "%ld", tstamp);
                VV_STRDUP (val, tm);
                vely_gen_add_header ("Etag", val);
            }


            // outputting the header for html will not work if web output is disallowed
            // we will allow it momentarily and then go back to what it was.
            num saved = pc->ctx.req->disable_output;
            pc->ctx.req->disable_output = 0;
            vely_output_http_header(pc->ctx.req); // all extract tags like Etag must come before because this will
                        // end the header section with two CRLF
            pc->ctx.req->disable_output = saved;
        }
        else
        {
            // 
            // Send file and appropriate header first
            //
            char disp_name[500];
            VV_TRACE("Header disp is [%s]", header->disp==NULL?"NULL":header->disp);
            if (header->disp != NULL)
            {
                if (header->file_name != NULL)
                {
                    char *enc = NULL;
                    vely_encode (VV_URL, header->file_name, -1, &enc);
                    snprintf (disp_name, sizeof(disp_name), "%s; filename*=UTF8''%s", header->disp, enc);
                } 
                else 
                {
                    snprintf (disp_name, sizeof(disp_name), "%s", header->disp);
                }
            }

            VV_STRDUP (val, header->ctype);
            vely_gen_set_content_type(val);
            snprintf (tm, sizeof(tm), "%lld", fsize);
            VV_STRDUP (val, tm);
            vely_gen_set_content_length(val);
            if (header->disp != NULL)
            {
                vely_gen_add_header ("Content-Disposition", disp_name);
            }
            if (header->cache_control!=NULL)
            {
                VV_TRACE("Setting cache [%s] for HTTP header (4)", header->cache_control);
                vely_gen_add_header ("Cache-Control", header->cache_control);
            }
            else
            {
                VV_TRACE("Setting no cache for HTTP header (5)");
                vely_gen_add_header ("Cache-Control", "max-age=0, no-cache");
                vely_gen_add_header ("Pragma", "no-cache");
            }

            // send etag for client to send back when asking again. If ETag is the same
            // for this file name, then it will be considered cached at the client and
            // 'not modified' message will be sent back.
            if (header->etag==1)
            {
                snprintf (tm, sizeof(tm), "%ld", tstamp);
                VV_STRDUP (val, tm);
                vely_gen_add_header ("Etag", val);
            }

            // add any headers set from the caller
            num i;
            for (i = 0; i<VV_MAX_HTTP_HEADER; i++)
            {
                if (header->control[i]!=NULL && header->value[i]!=NULL)
                {
                    // we use add_header because it allows multiple directives of the same kind
                    // but must not make duplicates of what's already there, except for Set-Cookie
                    vely_gen_add_header (header->control[i], header->value[i]);
                }
                else break;
            }
            pc->ctx.req->sent_header = 1; // because we just sent a header, and vely_write_web() needs this info    

        }

        // Send actual file contents
        if (vely_write_web (false, pc, str, fsize) != fsize)
        {
            VV_TRACE ("Cannot write [%lld] bytes to client from file [%s], error [%s]", fsize, fname, strerror(errno));
            // In this case, since vely_gen_write is synchronous, the server couldn't send all the data and it closed the connection
            // with the client. Nothing else to do for us.  
        }

    }
}


// 
// Output error message if request method is bad, for whatever reason
// Output is 400 Bad Request
// No reason is sent to the client. Separately stderr is written with error message.
// Does nothing for standalone
//
void vely_bad_request ()
{
    VV_TRACE ("");
    vely_config *pc = vely_get_config();
    if (vely_header_err(pc) != 1) return;
    vely_gen_set_status (400, "Bad Request");
    vely_gen_set_content_type("text/html;charset=utf-8");
    vely_gen_header_end ();
}



//
// report-error exit implementation. Go back prior to request, then goto right before vely_shut in the main loop.
// We guard against going nowhere by jumping only if sigsetjmp was done
// If we can't jump, we exit because we surely don't want to continue after report-error
//
void vely_error_request(num retval)
{
    VV_TRACE("");
    if (vely_done_err_setjmp == 1) 
    {
        siglongjmp (vely_err_jmp_buffer, retval);
    }
    else
    {
        // do not exit if in handling signal in chandle.c, which will fatally exit afterwards
        // this allows to continue and print any remaining error messages there
        if (vely_in_fatal_exit == 0)
        {
            VV_FATAL ("Cannot complete long jump from report-error"); // if cannot jump and recover, exit
        }
    }
}

//
// exit-request implementation. Go back prior to request, then goto right after so after-request is done.
// It can be called in _startup, but it will do nothing. It CAN be called in _before(), in which case it goes straight to _after().
// We guard against going nowhere by jumping only if sigsetjmp was done
//
void vely_exit_request(num retval)
{
    VV_TRACE("");
    if (vely_done_setjmp == 1) siglongjmp (vely_jmp_buffer, retval);
    // else do nothing
}



//
//
// Set application params at run time. config file is not required. 
// If config file not used, then current user/group is what program uses, 
// current directory is what it uses, and all other directories are based on current directory. Very simple.
// If config file used:
// User must be specified, and it can't be root. When cannot set to user specified, exit with error.
// This cannot be a root owned process, or we will exit with error message.
// If home directory not specified, taken to be current. You can use ~ in home directory to specify $HOME for a current user.
// Directories other than home can be specified, and if not, they are defaulted to the home directory plus appropriate subdir.
// 
//
// MUST NOT USE VV_MALLOC and such. This is allocated once per execution and if vely_malloc/*, it would be deallocated at the end of each request in fcgi!!
//


void vely_get_runtime_options()
{

    VV_TRACE("");
    vely_config *pc = vely_get_config ();

    char dir_name[300];

    snprintf (dir_name, sizeof(dir_name), "/var/lib/vv/%s/app/db", vely_app_name);
    pc->app.dbconf_dir = strdup(dir_name);
    snprintf (dir_name, sizeof(dir_name), "/var/lib/vv/%s/app", vely_app_name);
    pc->app.home_dir = strdup(dir_name);
    snprintf (dir_name, sizeof(dir_name), "/var/lib/vv/%s/app/file", vely_app_name);
    pc->app.file_dir = strdup(dir_name);
    snprintf (dir_name, sizeof(dir_name), "/var/lib/vv/%s/app/trace", vely_app_name);
    pc->app.trace_dir = strdup(dir_name);

    pc->app.max_upload_size  = vely_max_upload;
    pc->debug.trace_level = vely_is_trace;

#define VV_OTHER_DIR_OVERHEAD 50


    // Make sure that program cannot setuid to root
    if (setuid(0) == 0 || seteuid(0) == 0)
    {
        VV_FATAL ("Program can never run as effective user ID of root");
    }

    // We can now set directories, and use ~ based on user to get home directory, if available 


    if (chdir (pc->app.home_dir) != 0) // set current directory to be HOME_DIR
    {
        VV_FATAL ("Cannot change directory to [%s], error [%m]", pc->app.home_dir);
    }


    //
    // END OF SETTING UID and home directory 
    //

    return;
}

// CAN USE VV_MALLOC AGAIN

//
// Make random string based on current time/PID as random generator for srand().
// 'rnd' is the output, and rnd_len-1 is its length - rnd buffer will be allocated with
// rnd_len bytes of storage allocated on output, regardless of what it was coming in.
// rnd_len must be at least 2 (for 1 random byte).
// Generated random string does ends with zero byte
// (i.e. it is a valid string. The result
// is the numeric string if type is VV_RANDOM_NUM, mixed digits and upper/lower-case letters if 
// VV_RANDOM_STR, and binary if VV_RANDOM_BIN. VV_RANDOM_STR is default.
// If this is to be cryptographically secure pseudo random generator (CSPRNG) generator, then
// crypto is true. If this can't be done, then the error will be raised.
//
void vely_make_random (char **rnd, num rnd_len, char type, bool crypto)
{
    VV_TRACE("");

    *rnd = vely_malloc (rnd_len);

    if (crypto) 
    {
        // CSPRNG generator called by random-crypto
        if (vely_RAND_bytes((unsigned char*)(*rnd),(int)(rnd_len - 1)) != 1)
        {
            vely_report_error ("Cannot produce crypto random strng");
        }
        (*rnd)[rnd_len-1] = 0; // finish with zero for sanity, not a part of the actual random data
        return;
    }

    // Ensure next call to this function has a bit more random seed because time() will return the same value within
    // the same second
    // This is okay as static, even if this value is carried over from request to request, serving different modules,
    // and as it is, it actually increases the randomness of the result.

    // Character Source for final result
    static char ranged[] = "0123456789";
    static char rangea[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
    static char rangeb[256]; // for binary random

    static char isinit = 0; // per process init flag


    // init the random see with PID and time
    // this is a single srand per process. So different requests will get somewhat random intervals
    // making this stronger than if srand-ed for each request. Even if the sequence starts rotating,
    // it becomes less predictable because each request gets only a random interval of the roll.
    // Another randomization is that the same request would likely be served from a number of processes,
    // each serving to it partial random intervals based on different seeds, further enhancing the quality.
    if (isinit == 0) 
    {
        // init the seed, make low-bytes of PID the high of seed, and keep low bytes of current time
        num s = getpid()<<16;
        srand ((time(NULL)&0xFFFF)+s);
        // init rangeb for binary, from 0 to 255
        // this way binary random is always null terminated
        num i;
        for (i = 0; i < 256; i++) rangeb[i] = i;
        // init once
        isinit = 1;
    }


    num i;
    // get number of random characters requested
    for (i = 0; i < rnd_len - 1; i++) 
    {
       if (type == VV_RANDOM_STR) (*rnd)[i] = rangea[random()%62]; // 62 chars in rangea[], start with default
       else if (type == VV_RANDOM_NUM) (*rnd)[i] = ranged[random()%10]; // number 0-9 
       else if (type == VV_RANDOM_BIN) (*rnd)[i] = rangeb[random()%256]; // 256 values, from 0 to 255
       else vely_report_error ("Unknown random type [%d]", type);
    }
    (*rnd)[i] = 0; // finish with zero for sanity, this is byte rnd[rnd_len], not a part of random data

}


//
// get basename of file 'path' as vely_string. Path can be of a file or directory.
// returns basename
//
char *vely_basename (char *path)
{
    VV_TRACE("");
    char *pcopy = vely_strdup(path);
    char *res = vely_strdup(basename (pcopy));
    vely_free (pcopy); 
    return res;
}


//
// get realpath of file 'path' as vely_string. Path can be of a file or directory.
// returns realpath.
//
char *vely_realpath (char *path)
{
    VV_TRACE("");
    char *res;
    char *pcopy = vely_strdup (path);
    if ((res = realpath (dirname (pcopy), NULL)) != NULL)
    {
        vely_free (pcopy);
        char *retval = vely_strdup(res); // in order to avoid memory leaks
        free (res);
        return retval;
    } 
    else 
    {
        VERR;
        vely_free (pcopy);
        return VV_EMPTY_STRING;
    }
}

//
// hex to bin data. Convert hex src to dst, which is allocated. src is of len 'ilen',
// and 'olen' has dst length, excluding null byte. olen can be NULL. ilen can be -1
// in which case length of src is computed via strlen.
//
void vely_hex2bin(char *src, char **dst, num ilen, num *olen)
{
    VV_TRACE("");
    if (ilen == -1) ilen = strlen (src);
    *dst = (char*)vely_malloc (ilen/2 + 2); // +2 in case bad data/odd # of bytes
    num i;
    num j = 0;
    for (i = 0; i < ilen; )
    {
        int h;
        int l;
        URLDIG(src[i],h);
        URLDIG(src[i+1],l);
        (*dst)[j++] = (h<<4)+l;
        i+=2;
    }
    (*dst)[j] = 0;
    if (olen != NULL) *olen = j;
}

//
// Bin to hex data. Convert binary src to hex data, which is allocated. src
// is of len 'ilen'. 'pref' is added as prefix, unless NULL.
// ilen can be -1, in which case it is the string length of src.
// olen is the string length of dst, not counting zero-char at the end.
//
void vely_bin2hex(char *src, char **dst, num ilen, num *olen, char *pref)
{
    VV_TRACE("");
    if (ilen == -1) ilen = strlen (src);
    num l;
    if (pref != NULL) l = strlen (pref); else l = 0;
    *dst = (char*)vely_malloc (l+ilen*2+1);
    num i;
    if (pref != NULL) memcpy (*dst, pref, l);
    num j = l;
    for (i = 0; i < ilen; i ++)
    {
        VV_HEX_FROM_BYTE ((*dst)+j, (unsigned int)src[i]);
        j += 2;
    }
    (*dst)[j] = 0;
    if (olen != NULL) *olen=j;
}

//
// Calculate base b to the power of p, both integer, return b^p
//
num vely_topower(num b,num p)
{
    VV_TRACE("");
    num i; 
    num res = 1;

    for (i = 0; i < p; i++) res *= b;

    return res;
}

//
// Return web environment variable from header h.
// HTTP_ is prefixed, and all - are replaced with _
//
char *vely_getheader(char *h)
{
    VV_TRACE("");
    num hlen = strlen (h);
    char *hd = vely_malloc (hlen + 6); // account for length of HTTP_ plus 1 byte
    memcpy (hd, "HTTP_", 5); // HTTP_ at the beginning
    memcpy (hd + 5, h, hlen + 1); // copy the rest of h after HTTP_ including null byte
    vely_upper (hd + 5); // convert to upper case, skip HTTP_
    num i;
    for (i = 5; i < hlen + 5; i ++) if (hd[i] == '-') hd[i]='_'; // replace - with _, skip HTTP_
    char *res = vely_getenv(hd);
    vely_free (hd);
    return res;
}



