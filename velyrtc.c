// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework. 

//
// Library used both by VELY utility and VELY run-time 
// trailing 'c' in the name of this file refers to 'common' 
// code.
//

int vely_errno=0;

#include "vely.h"

// Function prototypes
num vely_core_write_file(FILE *f, num content_len, char *content, char ispos, num pos);

// 
// Close trace file 
// Returns VV_ERR_CLOSE if there is no context or not open, or the same
// as vely_fclose() if it's open
//
num vely_close_trace ()
{
    vely_config *pc = vely_get_config();

    if (pc == NULL) return VV_ERR_CLOSE;

    // open trace file, for fcgi, it will not be NULL (from previous request)
    if (pc->trace.f != NULL)
    {
        return vely_fclose(pc->trace.f);    
    } else return VV_ERR_CLOSE;
}



// 
// Open trace file and write begin-trace message
// Returns 0 if opened, -1 if not
// Any memory alloc here MUST be malloc since it survives fcgi requests and continues
// over many such requests.
//
num vely_open_trace ()
{
    vely_config *pc = vely_get_config();

    if (pc == NULL) return -1;

    // open trace file, for fcgi, it will not be NULL (from previous request)
    if (pc->trace.f != NULL)
    {
        return 0; // reuse tracing for the same process
    }


    if (pc->debug.trace_level > 0)
    {
        // timestamp, as PIDs can recycle
        vely_current_time (pc->trace.time, sizeof(pc->trace.time)-1);
        // append if file exists, otherwise open anew
        snprintf(pc->trace.fname, sizeof(pc->trace.fname), "%s/trace-%lld-%s", pc->app.trace_dir, vely_getpid(), pc->trace.time);
        pc->trace.f = vely_fopen (pc->trace.fname, "a+");
        if (pc->trace.f == NULL) 
        {
            pc->trace.f = vely_fopen (pc->trace.fname, "w+");
            if (pc->trace.f == NULL)
            {
                return -1; 
            }
        }
    }
    return 0;
}



// 
// Trace execution. This is called from VV_TRACE. 
// 'trace_level' is currently always 1. It is compared with the trace parameter in debug file, which is currently
// 0 or 1 (no tracing for 0 and tracing for 1). In the future, there may be more trace levels, with trace level 1
// including all tracing, trace level 2 including levels 2,3.. in trace file, etc.
// 'from_file' is the file name this trace is coming from.
// 'from_line' is the source line number, 'from_fun' is the function name.
// The rest (format,...) is printf-like data, the actual trace content.
// The trace also includes current time and PID.
//
// Trace can be called from memory function like vely_realloc. 
// If trace is called from anywhere else other than vely_* functions, it will  work the same way except there is no double calling of vely_check_memory.
//
void _vely_trace(num trace_level, const char *from_file, num from_line, const char *from_fun, char *format, ...)
{
    vely_config *pc = vely_get_config();
    if (pc == NULL) return; // do nothing if no config


    // THIS FUNCTON MUST NEVER USE ANY FORM OF MALLOC OR VV_MALLOC
    // or it may fail when memory is out or not available (such as in vely_malloc)


    // control which tracing will be done
    if (pc->debug.trace_level < trace_level) return;

    char trc[VV_TRACE_LEN + 1];

    // if this tracing came from this very function, ignore
    if (pc->trace.in_trace == 1) return;
    pc->trace.in_trace = 1;

    if (pc->trace.f == NULL) 
    {
        vely_open_trace(); // trace gets opened during fatal error, so open if not enabled
        if (pc->trace.f == NULL)  // if couldn't open, do not trace
        {
            pc->trace.in_trace = 0;
            return;
        }
    }

    va_list args;
    va_start (args, format);
    vsnprintf (trc, sizeof(trc) , format, args);
    va_end (args);

    // write time, code and message out
    // do NOT use pc->trace.time - this MUST stay constant during the request because it is
    // used in save_HTML to make sure name generated from this value is the same even if this name
    // is generated multiple times.
    // We do not specify PID as it is embedded in file name.
    char curr_time[200];
    vely_current_time (curr_time, sizeof(curr_time)-1);
    fprintf (pc->trace.f, "%s (%s:%lld)| %s %s\n", curr_time, from_file, from_line, from_fun, trc);
    //
    // We do not fflush() here - this is done either at the end of request (vely_shut()) or
    // when program crashes (vely_report_error())
    //
    pc->trace.in_trace = 0;
}

// 
// Get PID
//
num vely_getpid ()
{
    VV_TRACE ("");
    return (num) getpid();
}


//
// Get current time
// Output: outstr is the time string.
// out_str_len is time string buffer size
// If can't get time, output is empty string.
//
void vely_current_time (char *outstr, num out_str_len)
{
    VV_TRACE("");
    time_t t;
    struct tm *tmp;

    // get current time zone - may be set by customer program!
    char *curr_time_zone = getenv("TZ");
#define restore_curr_time_zone if (curr_time_zone!=NULL && curr_time_zone[0]!=0) { putenv(curr_time_zone); tzset(); }

    // set time zone to local - we did this in main() first thing before customer code. We cast vely_get_tz()
    // into mutable char * because putenv does NOT modify its string. The result of vely_get_tz must NOT change by
    // callers.
    putenv((char*)vely_get_tz());
    tzset();

    t = time(NULL);
    tmp = localtime(&t);
    if (tmp == NULL)
    {
        // return to customer TZ
        restore_curr_time_zone
        outstr[0] = 0;
        return;
    }

    if (strftime(outstr, out_str_len, "%F-%H-%M-%S", tmp) == 0)
    {
        outstr[0] = 0;
    }
    // return to customer TZ
    restore_curr_time_zone
}

// 
// Both configuration and run-time information (context, debug, trace, etc.)
// This is really a program  context.
// EVERYTHING IN VV_CONFIG MUST BE MALLOC (NOT VV_MALLOC) AS IT IS USED ACROSS REQUESTS
//
vely_config *vely_pc;

//
// Get config and context data
//
vely_config *vely_alloc_config()
{
    // start /var/log/syslog so that fatal message have a place to go to
    openlog(vely_app_name, LOG_PERROR | LOG_PID, LOG_USER);

    // all of vely config (sub)structures must be zeroed-out - we rely on this when setting directories, user id and such
    vely_pc = (vely_config*)calloc (1, sizeof(vely_config));
    if (vely_pc == NULL)
    {
        VV_FATAL ("Cannot allocate program context");
    }
    vely_init_config (vely_pc);
    return vely_pc;
}


// 
// Initialize program context. This is called only once for the
// life of the process. pc is program context.
//
void vely_init_config(vely_config *pc)
{
    assert (pc);

    // pc->* are set to 0 or NULL - set here only what's not zero
    pc->app.max_upload_size = 5000000;

    vely_reset_config (pc);
}

// 
// Reset program context. This is called for each new web request
//
void vely_reset_config(vely_config *pc)
{
    assert (pc);
    // these need to reset with each request
    // DO NOT RESET debug structure - should stay as it is for all request during the life of process!
    pc->ctx.req = NULL;
    pc->ctx.vely_report_error_is_in_report = 0;
    pc->debug.trace_level = vely_is_trace; // reset tracing, as it is set to 1 during report-error
}

//
// UP TO HERE THERE CAN BE NO VV_MALLOC
//



//
// Find number of occurances in string 'str' of a substring 'find'
// If case_sensitive is 1, then it's case sensitive, if 0 then not.
//
// Returns number of occurances of find in str
//
num vely_count_substring (char *str, char *find, num case_sensitive)
{
    VV_TRACE("");
    num count = 0;
    if (find == NULL || find[0] == 0) return 0;
    // here not empty or NULL
    num len = strlen (find);
    char *tmp = str;
    while((tmp = (case_sensitive == 1 ? strstr(tmp, find) : strcasestr(tmp, find))) != NULL)
    {
       count++;
       tmp += len;
    }
    return count;
}


// 
// Replace string 'find' with string 'subst' in string 'str', which is of size 'strsize' (total size in bytes of buffer 
// that is available). 'all' is 1 if all occurrance of 'find' are to be replaced.
// Output 'last' is the last byte position from which 'find' was searched for, but was not found (i.e.
// last byte position after which there is no more 'find' strings found).
// If case_sensitive is 1 then it's case sensitive, and 0 otherwise.
// Returns length of subst string, or -1 if not enough memory. If -1, whatever substitutions could have been
// made, were made, in which case use 'last' to know where we stopped.
//
num vely_replace_string (char *str, num strsize, char *find, char *subst, num all, char **last, num case_sensitive)
{
    VV_TRACE("");
    assert (str);
    assert (find);
    assert (subst);

    num len = strlen (str);
    num lenf = strlen (find);
    num lens = strlen (subst);
    num occ = 0;
    num currsize = len + 1;

    char *curr = str;

    if (last != NULL) *last = NULL;
    while (1)
    {
        // find a string and move memory to kill the found 
        // string and install new one - minimal memmove
        // based on diff
        char *f = (case_sensitive==1 ? strstr (curr, find) : strcasestr (curr,find));
        if (f == NULL) break;
        currsize -= lenf;
        currsize += lens;

        if (currsize > strsize)
        {
            return -1;
        }

        memmove (f + lens, f + lenf, len - (f - str + lenf) + 1);
        memcpy (f, subst, lens);

        // update length
        len = len - lenf + lens;

        curr = f + lens; // next pos to look from 

        if (last != NULL) *last = curr; 
                        // for caller, where to look next, if in
                        // external loop, for 'all==0' only
        occ++;
        if (all == 0) break;
    }
    return len;
}


//
// Trim string on both left and right and place string back
// into the original buffer. Trims spaces, tabs, newlines.
// str is the string to be vely_trimmed.
// len is the length of string on the input, and it has new length
// on the output. 'len' MUST be the strlen(str) on input!
//
void vely_trim (char *str, num *len)
{
    VV_TRACE("");
    assert (str);
    assert (len);
    
    num i = 0;
    // clear leading spaces
    while (isspace (str[i])) i++;
    // move string back, overriding leading spaces
    if (i) memmove (str, str + i, *len - i + 1);
    // update length
    *len = *len -i;
    // start from the end
    i = *len - 1;
    // find the last non-space char
    while (i>=0 && isspace (str[i])) i--;
    // make the end of string there
    str[i + 1] = 0;
    // update length of string
    *len = i + 1;
}

//
// Trim string on both left and right and return pointer to trimmed string- no movement of string
// Trims spaces, tabs, newlines.
// str is the string to be trimmed
// len is the length of string on the input, and it has new length
// on the output. 'len' MUST be the strlen(str) on input!
//
char *vely_trim_ptr (char *str, num *len)
{
    VV_TRACE("");
    assert (str);
    assert (len);

    char *res = str;
    
    num s = 0;
    // clear leading spaces
    while (isspace (str[s])) s++;
    res = str + s;

    // start from the end
    num e = *len - 1;

    // update length
    *len = *len -s;

    // find the last non-space char
    num j = 0;
    while (e>=s && isspace (str[e])) { e--; j++;}
    // make the end of string there
    str[e + 1] = 0;

    // update length of string
    *len = *len - j;
    return res;
}

// 
// Returns VV_DIR if 'dir' is a directory,
// VV_FILE if it's file, VV_ERR_FAILED if can't stat
//
num vely_file_type (char *dir)
{
    VV_TRACE("");
    struct stat sb;
    if (stat(dir, &sb) == 0)
    {
        if (S_ISDIR(sb.st_mode)) return VV_DIR; else return VV_FILE;
    }
    else 
    {
        VELY_ERR;
        return VV_ERR_FAILED;
    }
}


//
// Get Position from file FILE* f 
// Returns VV_OKAY if okay, VV_ERR_POSITION if cannot do it
//
num vely_get_file_pos(FILE *f, num *pos)
{
    VV_TRACE("");
    long p = ftell (f);
    if (p == -1) {
        VELY_ERR;
        VV_TRACE("Cannot get position in file, errno [%d]", errno);
        return VV_ERR_POSITION;
    }
    *pos = p;
    return VV_OKAY;
}

//
// Position file FILE* f to pos
// Returns VV_OKAY if okay, VV_ERR_POSITION if cannot do it
//
num vely_set_file_pos(FILE *f, num pos)
{
    VV_TRACE("");
    if (fseek (f,pos,SEEK_SET) != 0) {
        VELY_ERR;
        VV_TRACE("Cannot position in file to [%lld], errno [%d]", pos, errno);
        return VV_ERR_POSITION;
    }
    return VV_OKAY;
}

//
// Get size of file from open FILE*
// f is FILE *
// Returns size of the file
//
num vely_get_open_file_size(FILE *f)
{
    VV_TRACE("");
    long ppos = ftell(f);
    fseek(f, 0L, SEEK_END);
    long size=ftell(f);
    fseek(f, ppos, SEEK_SET); // position to where we were before
    return (num)size;
}

//
// Get size of file
// fn is file name.
// Returns size of the file, or -1 if file cannot be stat'
//
num vely_get_file_size(char *fn)
{
    VV_TRACE("");
    struct stat st;
    if (stat(fn, &st) != 0) 
    {
        VELY_ERR;
        return VV_ERR_FAILED;
    }
    return (num)(st.st_size);
}


// 
// Checks if input parameter name (in URL) is valid for vely.
// Valid names are consider to have only alphanumeric characters and
// underscores, and must not start with a digit.
// If hyphen is true, then it's allowed, otherwise it's not.
// Returns 1 if name is valid, 0 if not.
//
num vely_is_valid_param_name (char *name, bool hyphen)
{
    VV_TRACE ("");
    assert (name);

    num i = 0;
    if (!isalpha(name[0]) && name[0] != '_') return 0;
    while (name[i] != 0)
    {
        if (!isalnum(name[i]) && name[i] != '_' && (!hyphen || name[i] != '-')) return 0;
        i++;
    }
    return 1;
}


// 
// Initialize sequential list storage data
// fdata is storage data variable.
// Data can be stored in order and retrieved in the same order and rewound
// any number of times. Once used, must be purged.
//
void vely_store_init (vely_fifo **fdata_ptr)
{
    VV_TRACE ("");
    *fdata_ptr = (vely_fifo*)vely_malloc (sizeof(vely_fifo));
    vely_fifo *fdata = *fdata_ptr;

    fdata->num_of = 1;
    fdata->store_ptr = 0;
    fdata->retrieve_ptr = 0;
    fdata->item = vely_calloc (fdata->num_of, sizeof (vely_fifo_item));
}

// 
// Store name/value pair, with 'name' being the name and 'data' being the value
// in storage data 'fdata'. Both strings are stored in the list as pointers.
//
void vely_store (vely_fifo *fdata, char *name, void *data)
{
    VV_TRACE ("");
    assert (fdata != NULL);
    if (fdata->store_ptr >= fdata->num_of)
    {
        fdata->item = vely_realloc (fdata->item, (fdata->num_of+=VV_FIFO_ADD) * sizeof (vely_fifo_item));
    }
    fdata->item[fdata->store_ptr].data = data;
    fdata->item[fdata->store_ptr].name = name;
    fdata->store_ptr++;
}

// 
// Retrieve name/value pair from storage data 'fdata', with 'name' being the
// name and 'data' being the value. The name/data are simply assigned pointer
// values. Initially, this starts with fist name/value pair put in.
//
void vely_retrieve (vely_fifo *fdata, char **name, void **data)
{
    VV_TRACE ("");
    assert (fdata != NULL);

    if (fdata->retrieve_ptr >= fdata->store_ptr)
    {
        if (data != NULL) *data = NULL;
        if (name!= NULL) *name = NULL;
        return;
    }
    if (data != NULL) *data = fdata->item[fdata->retrieve_ptr].data;
    if (name != NULL) *name = fdata->item[fdata->retrieve_ptr].name;
    fdata->retrieve_ptr++;
}

// 
// Rewind name/value pair in storage 'fdata', so that next vely_retrieve()
// starts from the items first put in.
void vely_rewind (vely_fifo *fdata)
{
    VV_TRACE ("");
    assert (fdata != NULL);
    fdata->retrieve_ptr = 0;
}


// 
// Purge all data from storage 'fdata' and initialize for another use.
// If recreate is 1, then fifo is recreated and still available, otherwise it's utterly deleted
//
void vely_purge (vely_fifo **fdata_p, char recreate)
{
    VV_TRACE ("");
    assert (fdata_p != NULL);
    vely_fifo *fdata = *fdata_p;
    fdata->retrieve_ptr = 0;
    while (fdata->retrieve_ptr < fdata->store_ptr)
    {
        if (fdata->item[fdata->retrieve_ptr].data != NULL) fdata->item[fdata->retrieve_ptr].data = NULL;
        if (fdata->item[fdata->retrieve_ptr].name != NULL) fdata->item[fdata->retrieve_ptr].name = NULL;
        fdata->retrieve_ptr++;
    }
    if (fdata->item != NULL) vely_free (fdata->item);
    fdata->item = NULL;
    vely_free (fdata);
    if (recreate) vely_store_init (fdata_p); // okay fdata_p since fdata_p!=NULL
}


// 
// The same as strncpy() except that zero byte is placed at the end and it returns
// the length of the dest string.
//
num vely_strncpy(char *dest, char *src, num max_len)
{
    VV_TRACE("");
    num len = strlen (src);
    if (len < max_len) 
    {
        memcpy (dest, src, len+1 );
        return len;
    }
    else
    {
        memcpy (dest, src, max_len-1 );
        dest[max_len - 1] = 0;
        return max_len - 1;
    }
}

//
// Initialize a string that is allocated on the heap, like malloc with value of string s
//
char *vely_init_string(char *s)
{
    VV_TRACE("");
    if (s == NULL) return NULL;
    num l = strlen (s);
    char *res = vely_malloc (l+1);
    memcpy (res, s, l+1);
    return res;
}

// 
// Get timezone that's local to this server.
// Returns string in the format TZ=<timezone>, eg. TZ=MST
//
char * vely_get_tz ()
{
    //
    // This static usage is okay because the timezone is the SAME for all modules that could
    // run in this process. We can set timezone once for any of the modules, and the rest can
    // use the timezone just fine.
    //
    static num is_tz = 0;
    static char tz[200]; 

    // TZ variable isn't set by default, so we cannot count on it. Functions
    // that operate on time do CHECK if it's set, but unless we set it, it
    // WONT be set
    if (is_tz == 0)
    {
        is_tz = 1;

        // get localtime zone 
        time_t t = time(NULL);
        struct tm *tm = localtime(&t);
        snprintf (tz, sizeof(tz), "TZ=%s", tm->tm_zone);

    }
    return tz;
}

// 
// Read entire file with path 'name' and store file contents in output 'data', unless
// pos/len is specified (len is <>0 or pos<>0), in which case read len (default is 0, the rest of the file) bytes 
// from position pos (default is 0, the beginning). Returns -1 if cannot open file, -2 if cannot read, 
// -3 cannot position, -4 if nothing to read (pos/len bad), or size of data read.
// Note: zero byte is place after the end, so if return value is 10, it means
// there are 11 bytes, with zero at the end, regardless of whether the data is a
// string or not.
// If there is not enough memory, vely_malloc will error out.
//
num vely_read_file (char *name, char **data, num pos, num len)
{
    VV_TRACE ("");

    if (pos < 0) {VELY_ERR0; return VV_ERR_POSITION;} // len is negative
    if (len < 0) {VELY_ERR0; return VV_ERR_READ;} // pos is negative

    FILE *f = vely_fopen (name, "r");
    if (f == NULL)
    {
        //vely_fopen sets VELY_ERR
        return VV_ERR_OPEN;
    }
    num sz;
    if (len > 0) sz = len; else sz = vely_get_open_file_size(f) - pos;
    if (sz < 0) { VELY_ERR0; return VV_ERR_POSITION;} // pos is beyond size
    if (sz == 0) 
    {
        *data = VV_EMPTY_STRING;
        return 0; // nothing to read just an empty string
    }
    if (pos > 0)
    {
        if (fseek (f,pos,SEEK_SET) != 0) { 
            VELY_ERR;
            VV_TRACE("File [%s], cannot position to [%lld], errno [%d]", name, pos, errno);
            fclose (f);
            return VV_ERR_POSITION;
        }
    }
    *data = vely_malloc (sz + 1);
    num rd;
    rd = fread (*data, 1, sz, f);
    if (ferror (f))
    {
        VELY_ERR;
        fclose(f);
        return VV_ERR_READ;
    }
    (*data)[rd] = 0;
    fclose(f);
    return rd;
}


// 
// Read file with FILE* f and store file contents in output 'data'
// len (default is 0, the rest of the file) is the number of bytes to read
// from position pos, if not specified then from current position . Returns -1 if cannot open file, -2 if cannot read, 
// -3 cannot position, -4 if nothing to read (pos/len bad), or size of data read.
// Note: zero byte is place after the end, so if return value is 10, it means
// there are 11 bytes, with zero at the end, regardless of whether the data is a
// string or not.
// If there is not enough memory, vely_malloc will error out.
// ispos is true if position is given
//
num vely_read_file_id (FILE *f, char **data, num pos, num len, bool ispos)
{
    VV_TRACE ("");

    if (ispos && pos < 0) {VELY_ERR0; return VV_ERR_POSITION;} // len is negative
    if (len < 0) {VELY_ERR0; return VV_ERR_READ;} // len is negative

    if (f == NULL)
    {
        VELY_ERR;
        return VV_ERR_OPEN;
    }
    num sz;
    if (len > 0) sz = len; else
    {
        // If position not specified, find the current position, so that "reading to the 
        // end of the file" is correct exactly.
        if (!ispos) {
            if (vely_get_file_pos (f, &pos) != VV_OKAY) { return VV_ERR_POSITION;}
        }
        sz = vely_get_open_file_size(f) - pos;
    }
    if (sz < 0) {VELY_ERR0; return VV_ERR_POSITION;} // pos is beyond size
    if (sz == 0) 
    {
        *data = VV_EMPTY_STRING;
        return 0; // nothing to read just an empty string
    }
    if (ispos)
    {
        if (fseek (f,pos,SEEK_SET) != 0) { 
            VELY_ERR;
            VV_TRACE("File cannot position to [%lld], errno [%d]", pos, errno);
            return VV_ERR_POSITION;
        }
    }
    *data = vely_malloc (sz + 1);
    num rd;
    rd = fread (*data, 1, sz, f);
    if (ferror (f))
    {
        VELY_ERR;
        return VV_ERR_READ;
    }
    (*data)[rd] = 0;
    return rd;
}


// 
// Encode string v, producing output result res. enc_type is VV_WEB (for
// web encoding) or VV_URL (for url encoding). Pointer to pointer 'res' is allocated
// with sufficient memory in the worst case scenario
// vlen is the length of v, -1 if strlen(), otherwise length
// Returns length of an encoded string.
//
num vely_encode (num enc_type, char *v, num vlen, char **res)
{
    VV_TRACE("");
    return vely_encode_base (enc_type, v, vlen < 0 ? strlen(v) : (size_t)vlen, res, 1);
}


// 
// Encode string v, producing output result res. enc_type is VV_WEB (for
// web encoding) or VV_URL (for url encoding). Pointer to pointer 'res' is allocated
// with sufficient memory in the worst case scenario (if allocate_new is 1), or if it is 0, it MUST
// have enough space to hold VV_MAX_ENC_BLOWUP(vLen) in it), vLen is the string length of v.
// Null character added.
// String v can be smaller than length vLen, vLen is the maximum number of characters encoded.
// Returns length of an encoded string.
//
num vely_encode_base (num enc_type, char *v, num vLen, char **res, num allocate_new)
{
    VV_TRACE("");
    assert (res != NULL);
    assert (v != NULL);


    if (allocate_new==1)
    {
        *res = (char*)vely_malloc (VV_MAX_ENC_BLOWUP(vLen)); // worst case, see below for usage
    }
    num i;
    num j = 0;
    if (enc_type == VV_WEB)
    {
        for (i = 0; i < vLen; i ++)
        {
            switch (v[i])
            {
                case '&': memcpy (*res + j, "&amp;", 5); j+=5; break;
                case '"': memcpy (*res + j, "&quot;", 6); j+=6; break;
                case '\'': memcpy (*res + j, "&apos;", 6); j+=6; break;
                case '<': memcpy (*res + j, "&lt;", 4); j+=4; break;
                case '>': memcpy (*res + j, "&gt;", 4); j+=4; break;
                case 0: break;
                default: (*res)[j++] = v[i]; break;
            }
        }
    }
    else if (enc_type == VV_URL)
    {
        for (i = 0; i < vLen; i ++)
        {
            if (isalnum(v[i]) || v[i] == '-' || v[i] == '.' || v[i] == '_' || v[i] == '~') 
            {
                (*res)[j++] = v[i];
            }
            else
            {
                int h = v[i]/16;
                int l = v[i]%16;
                (*res)[j] = '%';
                (*res)[j+1] = h>=10 ? 'A'+h-10 : '0'+h;
                (*res)[j+2] = l>=10 ? 'A'+l-10 : '0'+l;
                j += 3;
            }
        }
    }
    else 
    {
        assert (1==2);
    }
    (*res)[j] = 0;
    return j;
}

//
// Position to pos position (if ispos is 1), and write content of content_len length (if 0, treat content
// as string and calculate length). Write to file 'f'.
// Return # of bytes written or error. The caller can close the file if needed, it's not closed here.
//
num vely_core_write_file(FILE *f, num content_len, char *content, char ispos, num pos)
{
    if (content_len==0) content_len=strlen(content);
    if (ispos == 1)  // positioning beyond the end of file is allowed. The gap will be filled with \0
    {
        if (fseek (f,pos,SEEK_SET) != 0) {
            VELY_ERR;
            VV_TRACE("File writing, cannot position to [%lld], errno [%d]", pos, errno);
            return VV_ERR_POSITION;
        }
    }
    if (fwrite(content, (size_t)content_len, 1, f) != 1)
    {
        VELY_ERR;
        VV_TRACE("Cannot write file, errno [%d]", errno);
        return VV_ERR_WRITE;
    }
    return content_len;
}

//
// Write file 'file_name' from data 'content' of length 'content_len'. If 'append' is 1,
// then this is appended to the file, otherwise, file is overwritten (or created if it didn't
// exist). If 'content_len' is 0, then write the whole string 'content' (must be 0 terminated).
// If pos is positive or 0 (and ispos==1), then position there in the file and then write - in this case file is not overwritten.
// ispos is 0 for no position, 1 otherwise.
// Cannot have append==1 and ispos==1. Vely checks and errors out if append and position are used before calling this.
// Returns VV_ERR_OPEN is cannot open file, VV_ERR_WRITE if cannot write, VV_ERR_POSITION if cannot
// position, or number of bytes written, which is always the number of bytes requested (otherwise it's an error).
// Maximum size of file is in 0..maxlonglong range.
//
num vely_write_file (char *file_name, char *content, num content_len, char append, num pos, char ispos)
{
    VV_TRACE("");

    if (ispos ==1 && pos < 0)  // any position that is not 0 or positive is an error
    {
        VELY_ERR0;
        return VV_ERR_POSITION;
    }

    FILE *f = NULL;
    // ispos is 0 if no positioning
    if (ispos == 0) f=vely_fopen (file_name,  append==1 ? "a+" : "w+"); // a+ for append, and truncate if neither append nor position
    else f=vely_fopen (file_name,  "r+"); // need read+write for positioning

    if (f==NULL)
    {
        //vely_fopen sets VELY_ERR
        return VV_ERR_OPEN;
    }
    num retw = vely_core_write_file(f, content_len, content, ispos, pos);
    fclose(f);
    return retw;
}

//
// Write file FILE *f(open) from data 'content' of length 'content_len'. If 'append' is 1,
// then this is appended to the file, otherwise, data is written at current position (if ispos is 0).
// If 'content_len' is 0, then write the whole string 'content' (must be 0 terminated).
// If pos is positive or 0 (and ispos 1), then position there in the file and then write - 
// ispos is 0 for no position, 1 otherwise.
// Cannot have append==1 and ispos==1. Vely checks and errors out if append and position are used before calling this.
// Returns VV_ERR_OPEN is cannot open file, VV_ERR_WRITE if cannot write, VV_ERR_POSITION if cannot
// position, or number of bytes written, which is always the number of bytes requested (otherwise it's an error).
// Maximum size of file is in 0..maxlonglong range.
//
num vely_write_file_id (FILE *f, char *content, num content_len, char append, num pos, char ispos)
{
    VV_TRACE("");

    if (f==NULL)
    {
        VELY_ERR;
        return VV_ERR_OPEN;
    }
    if (ispos ==1 && pos < 0)  // any position that is not 0 or positive is an error
    {
        VELY_ERR0;
        return VV_ERR_POSITION;
    }

    if (append == 1) 
    {
        num ef =  vely_get_open_file_size(f);
        if (fseek (f,ef,SEEK_SET) != 0) {
            VELY_ERR;
            VV_TRACE("File writing, cannot position to [%lld], errno [%d]", ef, errno);
            return VV_ERR_POSITION;
        }
    }

    return vely_core_write_file(f, content_len, content, ispos, pos);
}


//
// Add file to Vely's list of files to close. f is the pointer to FILE*.
// When program closes file, it must set *f = NULL in order not to be double freed.
// Returns memory index for FILE* in Vely's mem system.
//
num vely_reg_file(FILE **f)
{
    VV_TRACE("");
    num mind = vely_add_mem (f); // add pointer to file pointer so it can be closed if programmer doesnt do it
                                     // thus preventing file descriptor leakage
    vely_set_mem_status (VV_MEM_FILE, mind);
    return mind;
}


//
// Close a file. 
// Returns EOF if can't close, otherwise 0.
//
int vely_fclose (FILE *f)
{
    VV_TRACE ("");
    if (f == NULL) { VELY_ERR0; return VV_ERR_CLOSE; }
    int res= fclose (f);
    if (res == EOF) {
        VELY_ERR;
        return VV_ERR_CLOSE;
    }
    return VV_OKAY;
}

//
// Open a file. If open for writing, set permissions to 0770
// so it's read/write for owner/group
// Returns NULL if can't open, file pointer if it can
//
FILE *vely_fopen (char *file_name, char *mode)
{
    VV_TRACE ("");
    FILE *f = fopen (file_name, mode);
    // check if file opened
    if (f != NULL)
    {
        // if opened, check if mode has a(append) or w(write)
        // in which case, the file may be created, so change to 
        // -rw-rw----
        if (strchr (mode, 'a') != NULL || strchr (mode, 'w') != NULL)
        {
            fchmod (fileno (f), 0660);
        }
    } 
    else
    {
        VELY_ERR;
    }
    return f;
}



// Return name(id) of operating system
//
char * vely_os_name() {return VV_OS_NAME;}


//
// Return version of operating system
//
char * vely_os_version() {return VV_OS_VERSION;}


//
// Find a keyword 'find' in str, making sure the keyword is NOT quoted or with ()
// () is used to group an expression, which may contain keyword. Quoted string may also contain it.
// Returns the pointer to keyword in str, or NULL if not there
// Note: since each keyword has a space before it in the original Vely statement
// we look for either 1) the space or 2) 0 character that may have been put there - that
// must be the case, unless has_spaces is 0, for example a=b - keyword = does not need space before or after.
// If find is "", then we're looking for end-of-line (i.e. null character). The purpose of this is for
// recog_statement to scan through the rest of line and find unterminated string and unbalanced ()
// If paren is 0, then () is not looked at, only quoting matters for finding keyword (this is for break-string)
// Since a keyword may be contained in another (such as url and url-path in get-req), we also check that keyword
// is followed by a space or a null (unless has_spaces is 0).
//
char *vely_find_keyword0(char *str, char *find, num has_spaces, num paren)
{
    char *beg = str;
    char *f;
    while (1)
    {
        if (find[0] == 0) f = beg + strlen (beg); else f = strstr (beg, find);
        if (f != NULL)
        {
            // check that previous char is empty or space
            // unless we found keyword as the very first char
            // this does not apply if we look for end-of-string
            if (f != beg && find[0] != 0)
            {
                if (has_spaces == 1 && *(f-1) != 0 && *(f-1) != ' ')
                {
                    beg=f+1;
                    continue;
                }
            }
            if (has_spaces == 1 && find[0] != 0) // check that after keyword is 0 or empty space
                                                 // this is not an issue if has_spaces is 0, where there may be no space 
                                                 // before or after
            {
                int l = strlen (find);
                if (f[l-1] != ' ' && f[l] != 0 && f[l] != ' ') 
                {
                    beg =f+1;
                    continue;
                }
            }
            // now go back from found keyword and see how many unescaped quotes there are
            // and how many left and right parenthesis outside a string there are
            // the keyword must be outside the quotes and a string
            num quotes = 0;
            num left_par = 0;
            num right_par = 0;
            char *go_forth = str; // start from the beginning of text
            num within_string = 0;
            while (go_forth != f) // move forth until hitting the found keyword
            {
                if (*go_forth == '"') 
                {
                    // find out if we're in the string (possibly within an expression)
                    if (go_forth == str) quotes++; // at the very beginning (so no char behind)
                    else if (*(go_forth-1)!='\\') quotes++;
                }
                if (quotes % 2 == 1) within_string = 1; else within_string = 0;
                // if paren == 0, then left_par and right_par remain both 0, so technically balanced
                if (paren == 1 && *go_forth == '(' && within_string == 0) left_par++;
                if (paren == 1 && *go_forth == ')' && within_string == 0) right_par++;
                go_forth++;
            }
            if (find[0] == 0)
            {
                // if looking for end-of-string, there is only one of those, so do NOT continue in while loop
                // if quoted or () not balanced. Rather, error out. Otherwise, return pointer to end-of-string.
                if (within_string == 1)
                {
                    _vely_report_error( "Unterminated string");
                }
                if (left_par != right_par)
                {
                    _vely_report_error( "Unbalanced left and right parenthesis () in statement");
                }
                return (char*)f;
            }
            // check if prior to found keyword, all strings are closed and () are balanced, so it means it's outside
            if (within_string == 0) 
            {
                if (left_par == right_par) 
                {
                    break; 
                }
                else beg=f+1;
            }
            else beg=f+1;
        } else break;
    }
    return (char*)f;
}



//
// Decorate a hierarchical request path. Substitute / with __  and - with _. Path must begin with /_ and end with _/
// reqname is the fixed buffer that will hold the decorated path, reqname_len is its size,
// if is_dash is true, we look for _ in /_ and _/ (or just _ at the end), otherwise path starts with / and ends without it.
// p is the path to convert, p_len is its string length. Path is not checked for validity,
// Each path segment must be a valid C identifier, save for leading/ending _ and this must be assured by the caller.
// path must be trimmed by the caller
// if path is found, p is advanced to be on its last / so that parsing may continue (if there's /, otherwise points to null char)
// The caller must advance p by 1 to continue, or do nothing if points to null char
// Returns 1 if hierarchical path found, 0 if not, 2 if has leading /_ but no closure with _/ (only when is_dash is true)
// Returns 3 if there are no / , so just request name is in reqname (basically a copy of *p up to length of reqname_len)
//
char vely_decorate_path (char *reqname, num reqname_len, char **p, num p_len, bool is_dash)
{
    VV_TRACE("");
    num p_off; // offset to begin scanning p; so if it starts with /_ then it's 2, if it starts with / then 1
    num end_off; // offset to end to ereq below, for ending of path, so we're sure *p at the end is such that +1 moves us forward correctly
    if (is_dash) 
    {
        // this is for server-side check, must have /_
        if (**p != '/' || *(*p + 1) != '_') return 0; // quick way to find out if this isn't a hierarchical request path
                                                 // it must start with /_
        p_off = 2;
    } else
    {
        // this is for request-handler, must have just / with or without _
        if (**p != '/') 
        {
            if (strchr(*p, '/') == NULL) 
            {
                // if there are no / in name, it's just reqname, be it just a name or decorated
                strncpy (reqname, *p, reqname_len - 1);
                reqname[reqname_len - 1] = 0;
                return 3; // this is just request name, not a path,
            }
            return 0; 
        }
        if (*(*p + 1) == '_') { p_off = 2; } else { p_off = 1; } // if /_ just skip over _, or not if not there
    }
    if (p_off == 2 && (*p)[p_off] == '/') p_off++; // this is /_/... in this case get passed the second / too, this is just like /_/ in the back

    char *ereq = strstr (*p + p_off, "_/"); // search for ending, but after the beginning, since _/ could be in both
    end_off = 1; // default if there's _/, we end on _, and *p will be /, as this function advertises. Can change below if just _, or nothing.
    // check if _ is at the end, but nothing after it, set ereq to point to _
    if (ereq == NULL && (*p)[p_len - 1] == '_') {ereq = *p + p_len - 1; end_off = 0; } // this assumes PATH_INFO is trimmed by the caller, which it should be

    // if still nothing, this would be just a request that starts with _, but that can't be, since that's a non-request,
    // so this is an error. However for request-handler, neither _ nor _/ are required at the end, though they may be present.
    if (ereq == NULL && is_dash) { return 2; } // error if there is no closure with _/

    if (ereq == NULL && !is_dash) {ereq = *p + p_len; end_off = 0;} // for request-handler, if no _ or _/, then just make it the end
                                                     // because the whole thing is just a request path


    // convert request name given as a path into __ substituting for / and _ for -
    // so add-customer/america/subscription would be add_customer__america__subscription
    char *pbeg = *p + p_off; // begin to read req name from path segments, right after the first / which is always there
                        // see above, we only get here if *p ~ '/_'
    char *pres = reqname; // result into reqname
    num i = 0; // begin filling request
    *ereq = 0; // end request so we only go up until null char
    // exit loop if path segment for request ends, or if too long
    while (*pbeg && i <= reqname_len - 3) // since we +2 for __ below, plus for null
    {
        if (*pbeg == '/')  // subst / for __
        {
            pres[i++]='_'; 
            pres[i++]='_';
        } else if (*pbeg == '-') pres[i++]='_';  // subst - for _
        else pres[i++]=*pbeg;  // copy otherwise
        pbeg++; // move path segment forward
    }
    pres[i] = 0; // end result
    if (i > 1 && *(pres + i - 1) == '_' && *(pres + i - 2) == '_' ) *(pres + i - 2) = 0; // do not ever end with __ (in case of .../_/)
    *p = ereq + end_off; // skip to just passed _/ and start with / as it should (see above *p == '/')
                         // or just passed _ (if there's no _/). For !is_dash, that's actually the nul of end of string. 
    return 1;
}



