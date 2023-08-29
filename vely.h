// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC. Written by Sergio Mijatovic.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// Include file for VELY run time development and apps
//


#ifndef _VV_INC

#define _VV_INC

// needed for crash handling (obtaining source file name and line numbers)
#ifndef _GNU_SOURCE
#define _GNU_SOURCE 
#endif

// Version+Release. We use major plus minor plus release, as in 1.3.34,2.1.11,3.7.41... 
#define VV_VERSION "18.0.0"

// OS Name and Version
#define VV_OS_NAME  VV_OSNAME
#define VV_OS_VERSION  VV_OSVERSION

// database (MariaDB) related. Include if not application (i.e. if Vely itself), and if application, only if mariadb actually used
#if VV_APPMAKE==1 
#   if defined(VV_MARIADB_INCLUDE)
#       define VV_INC_MARIADB
#   endif
#   if defined(VV_POSTGRES_INCLUDE)
#       define VV_INC_POSTGRES
#   endif
#   if defined(VV_SQLITE_INCLUDE)
#       define VV_INC_SQLITE
#   endif
#else
#   define VV_INC_MARIADB
#   define VV_INC_POSTGRES
#   define VV_INC_SQLITE
#endif
#ifdef VV_INC_SQLITE
#   include <sqlite3.h>
#endif
#ifdef VV_INC_POSTGRES
#   include <libpq-fe.h>
#endif
#ifdef VV_INC_MARIADB
#   include <mysql.h>
#   include <mysqld_error.h>
#   include <errmsg.h>
#endif


// Includes
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>
#include <errno.h>
#include <ctype.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <sys/ioctl.h>
#include <unistd.h>
#include <pwd.h>
#include <limits.h>
#include <fcntl.h>
#include <setjmp.h>
#include <stdbool.h>
#include <sys/ipc.h>
#include <sys/shm.h>
// param.h for min/max without side effects
#include <sys/param.h>
#include <stdint.h>
// Fatal error loggin
#include <syslog.h>
// File ops
#include <libgen.h>
#include <sys/file.h>


// PCRE2 calls
#if VV_APPMAKE==1 
#   if defined(VV_PCRE2_INCLUDE)
#       define VV_INC_PCRE2
#   endif
#else
#   define VV_INC_PCRE2
#endif
#ifdef VV_INC_PCRE2
#   ifdef VV_C_POSIXREGEX
#       include "regex.h"
#   else
#       include "pcre2posix.h"
#   endif
#endif

// Crypto calls (encrypt, hash)
#if VV_APPMAKE==1 
#   if defined(VV_CRYPTO_INCLUDE)
#       define VV_INC_CRYPTO
#   endif
#else
#   define VV_INC_CRYPTO
#endif
#ifdef VV_INC_CRYPTO
#   include "openssl/sha.h"
#   include "openssl/evp.h"
#   include "openssl/aes.h"
#   include "openssl/bio.h"
#   include "openssl/buffer.h"
#   include "openssl/rand.h"
#endif

// Fcgi calls (new-fcgi)
#if VV_APPMAKE==1 
#   if defined(VV_FCGI_INCLUDE)
#       define VV_INC_FCGI
#   endif
#else
#   define VV_INC_FCGI
#endif
#ifdef VV_INC_FCGI
#   include "vfcgi.h"
#endif

// Web calls (curl)
#if VV_APPMAKE==1 
#   if defined(VV_CURL_INCLUDE)
#       define VV_INC_CURL
#   endif
#else
#   define VV_INC_CURL
#endif
#ifdef VV_INC_CURL
#   include <curl/curl.h>
#endif

// fast version of getting a config pointer
#define vely_get_config() (vely_pc)

// Stringize defines
#define VV_DEF_STR(s) VV_STR(s)
#define VV_STR(s) #s

// Types
typedef unsigned long long unum;
typedef long long num;
typedef int32_t num32;
typedef double dbl;

// 
// Defines
// 
#define VV_GET 1
#define VV_PUT 2
#define VV_POST 3
#define VV_PATCH 4
#define VV_DELETE 5
#define VV_OTHER 6
#define VV_MARIADB "mariadb"
#define VV_POSTGRES "postgres"
#define VV_SQLITE "sqlite"
#define VERR vely_errno=errno // save errno at the point of error for further examination later, if desired
#define VERR0 vely_errno=0 // no error, we caught it
#define VV_DEFINE_STRING(x) char *x = VV_EMPTY_STRING // define string as empty for use with vely_malloc etc
#define VV_INIT_STRING(x) x = VV_EMPTY_STRING // initialize existing string as empty for use with vely_malloc etc
#define VV_TRACE_DIR "trace" // the name of trace directory is always 'trace'
// since we're only supporting Centos 7+, this is what comes with it, or if it's not there, user needs to install
// When we go to Centos 8, the code for Centos 7 remains in its own branch, and we set something else here for 8
// (that is, if the default changes)
// for vely_<memory handling> inline or not?  Currently not, otherwise would be 'inline' instead of empty
#define VV_TRACE_LEN 12000 // max length of line in trace file and max length of line in verbose output of 
#define VV_MAX_NESTED_WRITE_STRING 5 // max # of nests of write-string
// max # of custom header a programmer can add to custom reply when replying with a file
#define VV_MAX_HTTP_HEADER 32
#define VV_MAX_SIZE_OF_URL 32000 /* maximum length of browser url (get) */
#define VV_MAX_ERR_LEN 12000 /* maximum error length in report error */
#define VV_MAX_UPLOAD_DIR  40000 /* max directories in file directory */
#define VV_MAX_FILENAME_LEN 512 /* max file name for make document */
#define VV_ERROR_EXIT_CODE 99 // exit code of command line program when it hits any error
// constants for encoding
#define VV_URL 1
#define VV_WEB 2
#define VV_NOENC 3
// Max cookies per request and max length of a single cookie
#define VV_MAX_COOKIES 256
#define VV_MAX_COOKIE_SIZE 2048
// Max length of URL path (between web address and query string) 
#define VV_MAX_PATH 2512
#define VV_TIME_LEN 200 // max length of time strings
// maximum number of bytes needed to encode either URL or WEB encoded string
#define VV_MAX_ENC_BLOWUP(x) ((x)*6+1)
#define VV_MAX_QUERY_OUTPUTS 1000 // maximum # of output parameters for each query, per query, essentially # of result columns
// Constants for encryption
#define VV_KEYLEN 64 // SHA256 hashed key or encryption key - 64 bytes - the shared memory allocated will be 64+1 bytes for zero terminator byte
// add this many for fifo list 
#define VV_FIFO_ADD 100
// database connection limit
#define VV_CURR_DB (vely_get_config()->ctx.db->conn[vely_get_config()->ctx.db->ind_current_db]) // context of execution
// on-error for database (continue or exit), statement level only
#define VV_ON_ERROR_CONTINUE 1
#define VV_ON_ERROR_EXIT 2
// Buffered output 
#define VV_BUFFERED_OUTPUT 1
#define VV_UNBUFFERED_OUTPUT 0
// Type of header init
#define VV_HEADER_FILE 0
#define VV_HEADER_PAGE 1
// Type of object in file system
#define VV_FILE 1
#define VV_DIR 2
// Status that isn't error
#define VV_COLLISION 1
// Error codes
#define VV_OKAY 0
#define VV_ERR_OPEN -1
#define VV_ERR_OPEN_TARGET -2
#define VV_ERR_READ -3
#define VV_ERR_WRITE -4
#define VV_ERR_POSITION -5
#define VV_ERR_TOO_MANY -6
#define VV_ERR_DELETE -7
#define VV_ERR_FAILED -8
#define VV_ERR_WEB_CALL -9
#define VV_ERR_CREATE -10 
#define VV_ERR_EXIST -11
#define VV_ERR_INVALID -12
#define VV_ERR_RENAME -13
#define VV_ERR_MEMORY -14
#define VV_ERR_UTF8 -15
#define VV_ERR_JSON -16
#define VV_ERR_CLOSE -17
// new errors below
// the last error
#define VV_ERR_UNKNOWN -127
//types of database
#define VV_DB_MARIADB 0
#define VV_DB_POSTGRES 1
#define VV_DB_SQLITE 2
//types of random data generation
#define VV_RANDOM_NUM 0
#define VV_RANDOM_STR 1
#define VV_RANDOM_BIN 2
//types of memory
#define VV_MEM_FREE 1
#define VV_MEM_FILE 2

// types of json value
#define VV_JSON_TYPE_STRING 0
#define VV_JSON_TYPE_NUMBER 1
#define VV_JSON_TYPE_REAL 2
#define VV_JSON_TYPE_BOOL 3
#define VV_JSON_TYPE_NULL 4
// errors in json parsin
#define VV_ERR_JSON_UNKNOWN "Unknown entity (true, false or null)"
#define VV_ERR_JSON_NUMBER "Cannot parse a number"
#define VV_ERR_JSON_COLON_EXPECTED "Colon is expected here"
#define VV_ERR_JSON_NAME_EXPECTED "Name is expected here"
#define VV_ERR_JSON_COMMA_END_OBJECT_EXPECTED "Comma or end of object is expected here"
#define VV_ERR_JSON_COMMA_END_ARRAY_EXPECTED "Comma or end of array is expected here"
#define VV_ERR_JSON_UNRECOGNIZED "Unrecognized token"
#define VV_ERR_JSON_SYNTAX "Extra characters remaining unparsed"
#define VV_ERR_JSON_BADUTF "Bad UTF character"
#define VV_ERR_JSON_BADESCAPE "Unknown escape sequence"
#define VV_ERR_JSON_NOQUOTE "Double quote is missing"
#define VV_ERR_JSON_SYNTAX "Extra characters remaining unparsed"
#define VV_ERR_JSON_DEPTH "Depth of leaf node too great"
#define VV_ERR_JSON_SURROGATE "Surrogate UTF-8 value missing"
#define VV_ERR_JSON_INTERRUPTED "JSON parsing interrupted by a request handler"

//UTF8 errors
#define VV_UTF8_ERR_ILLEGAL_CHARACTER_FEFF "Illegal character code 0xFEFF"
#define VV_UTF8_ERR_UTF16_SURROGATE "Illegal UTF16 surrogate range"
#define VV_UTF8_ERR_OUT_OF_RANGE "UTF8 character out of range"
#define VV_UTF8_ERR_SECOND_BYTE "Second UTF8 byte invalid"
#define VV_UTF8_ERR_THIRD_BYTE "Third UTF8 byte invalid"
#define VV_UTF8_ERR_FOURTH_BYTE "Fourth UTF8 byte invalid"
#define VV_UTF8_INVALID "Invalid UTF8 value"
#define VV_UTF8_NO_SURROGATE "Invalid surrogate UTF8 value"

//Request parsing errors
#define VV_ERR_DUPREQ "Input parameter 'req' specified more than once."

//
// Vely memory adjustment:
// sizeof(num) is the overhead for VELY memory checksum before each block(see below), which is 8 bytes
// however, since all memory needs to be 16 bytes aligned (as of now), we need to have that
// much memory for the overhead, so that the actual memory served is always 16-bytes aligned
// What is important is that our overhead is smaller, i.e. sizeof(num)<=k*16
// Another important requirement is that num MUST always be on a 8 byte boundary (or sizeof(num))
// whatever it is. If it is not, CPU will NOT read it correctly, it may be garbage.
// change this if in the future CPU alignment is 32 bytes alignment,instead of 16 for example! Note that CPU 
// alignment is INDEPENDENT of sizeof(num). 16 here is the CPU alignment. It must be minimum that.
// But if the amount needed is greater than that, then it must be a multiple of 16, as computed below.
#define VV_CPUALIGN (16)
#define VV_MULTALIGN(x) (((x)/VV_CPUALIGN+(x%VV_CPUALIGN !=0 ? 1:0)) * VV_CPUALIGN)
#define VV_ALIGN (VV_MULTALIGN(sizeof(num)))

// 
// Data type definitions
//

// 
// Debug information obtained from trace/debug file
//
typedef struct s_vely_debug_app
{
    num trace_level; // trace level, currently 0 (no trace) or 1 (trace)
    num trace_size;  // # of stack items in stack dump after the crash (obtained at crash from backtrace())
} vely_debug_app;
// 
// Name/value pair for sequential list API
//
typedef struct vely_fifo_item_s
{
    void *data;
    char *name;
} vely_fifo_item;
// 
// Information needed to traverse and rewide the sequential list API, plus the array of items itself
typedef struct vely_fifo_s
{
    vely_fifo_item *item; // array of items
    num num_of; // # of items
    num store_ptr; // end of list
    num retrieve_ptr; // where to get next one
} vely_fifo;
//
// File structure, for now just FILE *
//
typedef struct vely_file_s
{
    FILE **f; // pointer to file pointer
    num memind; // pointer to file pointer's location in Vely's memory mgmt system
} vely_file;
// 
// Configuration context data for application, read from config file. Does not change during a request.
//
typedef struct s_vely_app_data
{
    char *run_dir; // the current directory at the moment program starts
    char *home_dir; // home directory
    char *dbconf_dir; // database connections dir
    char *trace_dir; // directory for tracing
    char *file_dir; // directory for uploads
    long max_upload_size; // maximum upload size for any file
} vely_app_data;
// 
// Run-time information for tracing
//
typedef struct s_vely_conf_trace
{
    num in_trace; // if 1, the caller that is attempting to use tracing function which originated in tracing code
    FILE *f; // file used for tracing file, located in trace directory
    char fname[300]; // name of trace file
    char time[VV_TIME_LEN + 1]; // time of last tracing
} vely_conf_trace;
// 
// 
// Input arguments (argc,argv) from main for FCGI program 
//
typedef struct s_vely_args
{
    num num_of_args; // number of arguments (minus 1, we don't count argv[0])
    char **args; // arguments, starts with argv[1]
} vely_args;
// 
// Input parameters from a request (URL input parameters or POST without uploads).
//
typedef struct s_vely_input_params
{
    char **names; // URL names for GET/POST request
    char **values; // URL values for GET/POST request
    num num_of_input_params; // # of name/values in GET/POST request
} vely_input_params;
// 
// Write string (write-string markup) information
typedef struct vely_write_string_t
{
    char *string; // Actual data being built. 
    char **user_string; // user string to be built
    num len; // allocated length
    num buf_pos; // current write position
    num notrim; // default 0; if 1, do not trim the last newline
    num wlen; // length of writing in the buffer (variable)
} vely_write_string;
// 
// Cookies. Array of cookies from input requests and any changes.
//
typedef struct vely_cookies_s
{
    // These are cookies as received from the client mixed with cookies set by the program
    char *data; // cookie string
    char is_set_by_program; // if 1, this cookie has been changed (deleted, or added)
} vely_cookies;
// header structure to send back file to a web client
typedef struct s_vely_header
{
    char *ctype; // content type
    num clen; // content length (call-web only?)
    char *disp; // header content disposition
    char *file_name; // file name being sent
    char *cache_control; // cache control http header
    num etag; // if 1,include etag which is the time stamp of last modification date of the file
    // the status_* are for status setting. status_id is the status id (such as 302) and status_text is it's corresponding text (such as '302 Found')
    // The example here is for redirection, but can be anything
    num status_id;
    char *status_text;
    // the following are for generic http header of any kind, in fact content type, cache control etc. can all be done here while leaving others empty
    char *control[VV_MAX_HTTP_HEADER+1];
    char *value[VV_MAX_HTTP_HEADER+1];
} vely_header;
// 
// Input request. Overarching structure that contains much information not just about
// input request, but about current configuration, run-time state of the program.
typedef struct vely_input_req_s
{
    vely_app_data *app; // context, could be obtained as pc->app, but here is for convenience
    num sent_header; // 1 if http header sent already 
    num data_was_output; // 1 if any data (after header) was output
    char *if_none_match; // IF_NONE_MATCH from HTTP request
    num disable_output; // if 1, HTML output is disabled 
    vely_write_string write_string_arr[VV_MAX_NESTED_WRITE_STRING]; // holds a stack for write-string
    num curr_write_to_string; // current write-to-string nest level from 0 to VV_MAX_NESTED_WRITE_STRING-1
    // cookies
    vely_cookies *cookies;
    num num_of_cookies;
    vely_args args; // main() input params to FCGI program 
    vely_input_params ip; // URL input params
    char *referring_url; // where we came from 
    num from_here; // did the current request come from this web server? 0 if not, 1 if yes.
    num is_shut; // 1 if vely_shut already called
    vely_header *header; // if NULL, do nothing (no custom headers), if not-NULL use it to set custom headers
    char silent; // if 1, headers not output
    int ec; // exit code for command-line
    void *data; // request-specific (and request-bound, i.e. only available to code executing in the same request) data
    char *body; // if POST/PUT/PATCH.. has just a body (no multipart), this is it
    num body_len; // if POST/PUT/PATCH.. this is body length (for no multipart)
    num method; // VV_GET/POST/PATCH/PUT/DELETE
    char *name; // request name, value of "req" param
    num task; // index into name/values in ip (vely_input_params)
} vely_input_req;
// 
// Context of execution. Contains input request, flags
typedef union s_vely_dbc
{
    // In all Vely libraries, VV_INC_POSTGRES/MARIADB are defined. So, db-specific pointers are used instead of void*
    // In generated Vely program, we could have any of the db-specific pointers, or void, depending on which dbs are used.
    // So if MariaDB is used, we will have MYSQL* data in the union. If Postgres is used, we will have PGresult. If any of
    // those is not used, we will have void pointers instead in the appropriate part of the union. The important thing is 
    // that the size of any pointer is the same; thus size and access to fields does not change, as long as we have the same
    // number of pointers as a substitute in each union. So:
    // 1. nothing but pointers must be used here
    // 2. the number of pointers and their names must be the same in #ifdef part as it is in #else part
    // otherwise there will be unpredictable results, because accessing fields, and allocating will be different in 
    // libraries than it is in generated code.
    struct s_pg
    {
#ifdef VV_INC_POSTGRES
        PGresult *res;
        PGconn *con;
        char *name;
#else
        // generic substitute if no db used 
        void *con; 
        void *res; 
        void *name;
#endif
    } pg;
    struct s_l
    {
#ifdef VV_INC_SQLITE
        sqlite3 *con;
        sqlite3_stmt *stmt;
#else
        // generic substitute if no db used 
        void *con; 
        void *stmt;
#endif
    } sqlite;
    struct s_m
    {
#ifdef VV_INC_MARIADB
        MYSQL *con;
        MYSQL_RES *res;
        MYSQL_BIND *bind;
        MYSQL_BIND *bindout;
        MYSQL_STMT *stmt;
#else
        // generic substitute if no db used 
        void *con; 
        void *res; 
        void *bind;
        void *bindout;
        void *stmt;
#endif
    } maria;
} vely_dbc;

typedef struct one_db_s
{
    // Important: dbc is a pointer and should stay that way. The sizeof(vely_dbc) may mutate, while the size of
    // pointer to it (or to whatever) never will
    vely_dbc *dbc; // database connector. Includes any databases actually used, so one generic database connector
                  // can mutate into any database supported.
    num is_begin_transaction; // are we in transaction in this process
    num num_inp; // number of input parameters to query
    char need_copy; // 1 if result fields must be copied to newly alloced storage
    num has_connected; // are we connected to db at this moment
    char *db_name; // the name of current connection
    num db_type; // the type of current connection (0 is "mariadb" etc) - used only in compilation, not in runtime
    num exit_on_error; // if 1, exit on error, otherwise, let app handle. Can be changed at run-time.
} one_db;
typedef struct s_vely_db_connections
{
    one_db *conn;
    num ind_current_db; // the index (in conn array) of the currently connected db
} vely_db_connections;

typedef struct s_vely_context
{
    void *data; // process-specific data, available across requests
    vely_input_req *req; // input request (see definition)
    num vely_report_error_is_in_report; // 1 if in progress of reporting an error 

    //
    // Each server KEEPS connection between requests.
    // We keep a static value in the application, and application sets the pointer below to this application static value.
    //
    vely_db_connections *db;
    num tot_dbs; // number of dbs

} vely_context;
// 
// Configuration and run-time status. Config file, debug file, tracing, output buffers, context of request
//
typedef struct s_vely_config
{
    // these stay the same once set
    vely_app_data app; // does not change during a request
    vely_debug_app debug; // does not change during a request

    // these change during a request
    vely_conf_trace trace; // tracing info
    vely_context ctx; // context of execution, not config, but convenient to
                // have it handy. That is why it's separate type. Changes at run-time
} vely_config;
// 
// Structure for breaking up strings into pieces based on delimiters
//
typedef struct vely_split_str_s
{
    char **pieces; // array of pieces
    num num_pieces; // #num of pieces in 'pieces'
} vely_split_str;
//
// Information we collect about each shared library that is linked here
//
typedef struct s_vely_so_info 
{
    // Module information, used in addr2line 
    void *mod_addr; // module start load address
    long mod_offset; // offset of elf in the file (for addr2line)
    void *mod_end; // module end load address
    char mod_name[256]; // module name
} vely_so_info;


//
// Hash structures
//
// Hash table structure, this is an element of a linked list
typedef struct vely_s_hash_table
{
    char *key; // key for hashing
    void *data; // data
    struct vely_s_hash_table *next; // next element in the list
} vely_hash_table;
// Hash structure, top-level object
typedef struct vely_s_hash
{
    num size; // size of hash table
    vely_hash_table **table; // actual table, array of lists, array of size 'size'
    num dnext; // the index in table[] from which to continue dumping key/value pairs
    vely_hash_table *dcurr; // pointer to current dumping member of linked list within [dnext] bucket
    num tot; // total how many elements in cache
    num hits; // total number of searches
    num reads; // total number of comparisons
} vely_hash;

//
// An array of normalized JSON name/value pairs
//
typedef struct vely_jsonn_s
{
    char *name; // name of object, normalized
    char type; // type of value, see VV_JSON_TYPE_...
    char *str; // string value
} vely_jsonn;
//
// A normalized path in json file being traversed
//
typedef struct vely_s_json_node
{
    char *name; // name of node
    num name_len; // length of name of node
    num index; // index of node if array, otherwise -1
    num index_len; // length of array number, so for 23 it's 2
} json_node;
//
// Json node handler 
//
typedef num (*vely_json_node_handler)(num node_count, json_node *list, char *val, num type); // node_handler when in use in new-json
//
// Json structure sent back to Vely
//
typedef struct vely_json_s
{
    vely_jsonn *nodes; // list of nodes
    num node_c; // number of nodes
    vely_hash *hash; // hash for normalized names
    num maxhash; // maximum size of hash
    num dnext; // index of traverse read
    bool usehash; // true if hash
    vely_json_node_handler node_handler; // function user uses to handle incoming json data with node-handler
} vely_json;



// 
// Macros and function call related
// Some of these may have a double evaluation problem (TODO) in macros
//
#define VV_UNUSED(x) (void)(x)
#define  VV_FATAL(...) {syslog(LOG_ERR, __VA_ARGS__); _Exit(-1);}
//trace is available only if Vely compiled with DI=1
#ifdef DEBUG
#define  VV_TRACE(...) (vely_get_config()->debug.trace_level !=0 ? _vely_trace(1, __FILE__, __LINE__, __FUNCTION__,  __VA_ARGS__) : 0)
#else
#define  VV_TRACE(...) true
#endif
#define  vely_report_error(...) {_vely_report_error(__VA_ARGS__);exit(0);}
#define VV_STRDUP(x, y) {char *_vv_temp = (y); (x) = vely_strdup (_vv_temp == NULL ? "" : _vv_temp); }
#define VV_STRLDUP(x, y, r) {char *_vv_temp = (y); num _vv_ltemp = strlen(_vv_temp); ((x)=memcpy (vely_malloc (_vv_ltemp+1), _vv_temp, _vv_ltemp))[_vv_ltemp] = 0; if ((r)!=NULL) *(r) = _vv_ltemp; }
/*hexadecimal conversions*/
#define VV_CHAR_FROM_HEX(x) (((x)>'9') ? (((x)>='a') ? ((x)-'a'+10) : ((x)-'A'+10)) : ((x)-'0')) /* for conversion in URL - ASCII ONLY!
                        numbers are lower than capital letter are lower than lower case letters!! */
#define VV_TO_HEX(x) ((x) <= 9 ? '0' + (x) : 'a' - 10 + (x))
#define VV_HEX_FROM_BYTE(p,x) ((p)[0] = VV_TO_HEX(((x)&0xF0)>>4), (p)[1] = VV_TO_HEX((x)&0x0F))
#define VV_HEX_FROM_INT16(p,x) (VV_HEX_FROM_BYTE(p, (((x)&0xFF00)>>8)), VV_HEX_FROM_BYTE(p+2, ((x)&0xFF)))
#define VV_HEX_FROM_INT32(p,x) (VV_HEX_FROM_BYTE(p, (((x)&0xFF000000)>>24)), VV_HEX_FROM_BYTE(p+2, (((x)&0xFF0000)>>16)), VV_HEX_FROM_BYTE(p+4, (((x)&0xFF00)>>8)), VV_HEX_FROM_BYTE(p+6, ((x)&0xFF)))
/*end hexadecimal conversions*/
#define VV_MEMINLINE 
// The actual calls for memoryhandling
#define vely_malloc _vely_malloc
#define vely_realloc(x,y) _vely_realloc((x),(y),0)
#define vely_free(x) _vely_free((x), 0)
#define vely_strdup _vely_strdup
#define vely_calloc _vely_calloc


// 
//
// Function declarations
//
//
void vely_init_input_req (vely_input_req *iu);
num vely_open_trace ();
num vely_close_trace();
void vely_make_SQL (char **dest, num num_of_args, char *format, ...) __attribute__ ((format (printf, 3, 4)));
void vely_output_http_header(vely_input_req *iu);
void _vely_report_error (char *format, ...) __attribute__ ((format (printf, 1, 2)));
num vely_encode (num enc_type, char *v, num vlen, char **res);
num vely_get_input(vely_input_req *req, char *method, char *input);
char *vely_get_input_param (vely_input_req *iu, char *name, bool is_task);
num vely_is_positive_num (char *s);
num vely_exec_program (char *prg, char *argv[], num num_args, FILE *fin, FILE **fout, FILE **ferr, char *inp, num inp_len, char **out_buf, num *out_len, char **err_buf);
void vely_get_debug_options();
num vely_flush_printf(num fin);
void vely_printf_close();
num vely_printf (bool iserr, num enc_type, char *format, ...) __attribute__ ((format (printf, 3, 4)));
void vely_shut(vely_input_req *giu);
FILE * vely_make_document (char **write_dir, num is_temp);
char *vely_getenv (char *var);
char *vely_getenv_os (char *var);
void vely_putenv (char *env);
void vely_replace_all (num v, char *look, char *subst);
void vely_current_time (char *outstr, num out_str_len);
vely_config *vely_alloc_config();
void vely_init_config(vely_config *pc);
void vely_reset_config(vely_config *pc);
num vely_count_substring (char *str, char *find, num case_sensitive);
num vely_replace_string (char *str, num strsize, char *find, char *subst, num all, char **last, num case_sensitive);
void vely_trim (char *str, num *len);
char *vely_trim_ptr (char *str, num *len);
num vely_file_type (char *dir);
num vely_get_open_file_size(FILE *f);
num vely_get_file_size(char *fn);
void vely_memory_init ();
VV_MEMINLINE void *_vely_malloc(size_t size);
VV_MEMINLINE void *_vely_calloc(size_t nmemb, size_t size);
VV_MEMINLINE void *_vely_realloc(void *ptr, size_t size, char safe);
VV_MEMINLINE bool _vely_free (void *ptr, char check);
VV_MEMINLINE num vely_safe_free (void *ptr);
VV_MEMINLINE char *_vely_strdup (char *s);
VV_MEMINLINE void vely_set_mem_status (unsigned char s, num ind);
VV_MEMINLINE num vely_add_mem (void *p);
VV_MEMINLINE void *vely_vmset (void *p, num r);
void vely_done ();
void vely_get_stack(char *fname);
#if defined(VV_INC_MARIADB) || defined(VV_INC_POSTGRES)
vely_dbc *vely_get_db_connection (num abort_if_bad);
#endif
void vely_close_db_conn ();
num vely_begin_transaction(char *t, char erract, char **err, char **errt);
num vely_commit(char *t, char erract, char **err, char **errt);
num vely_rollback(char *t, char erract, char **err, char **errt);
void vely_get_insert_id(char *val, num sizeVal);
void vely_select_table (char *s, num *arow, num *nrow, num *ncol, char ***col_names, char ***data, num **dlen, char **er, char **errm, char is_prep, void **prep, num paramcount, char **params, char erract);
void _vely_trace(num trace_level, const char *fromFile, num fromLine, const char *fromFun, char *format, ...) __attribute__((format(printf, 5, 6)));
char *vely_hash_data( char *val, char *digest_name, bool binary, num *outlen );
char *vely_derive_key( char *val, num val_len, char *digest_name, num iter_count, char *salt, num salt_len, num key_len, bool binary );
num vely_ws_util_read (void * rp, char *content, num len);
num vely_main (void *r);
num vely_ws_printf (void *r, char *fmt, ...) __attribute__ ((format (printf, 2, 3)));
void posix_print_stack_trace();
void vely_disable_output();
void vely_file_being_output();
void vely_set_crash_handler(char *dir);
void vely_dispatch_request();
num vely_copy_data (char **data, char *value);
num vely_puts_to_string (char *final_out, num final_len);
char *vely_init_string(char *s);
num vely_puts (num enc_type, char *s);
num vely_copy_data_at_offset (char **data, num off, char *value);
num vely_is_valid_param_name (char *name);
void vely_write_to_string (char **str);
num vely_write_to_string_length ();
void vely_write_to_string_notrim ();
num _vely_check_memory(void *ptr);
void vely_set_cookie (vely_input_req *req, char *cookie_name, char *cookie_value, char *ypath, char *expires, char *samesite, char *httponly, char *secure);
char *vely_find_cookie (vely_input_req *req, char *cookie_name, num *ind, char **path, char **exp);
num vely_delete_cookie (vely_input_req *req, char *cookie_name, char *path, char *secure);
num vely_decode (num enc_type, char *v, num inlen);
char *vely_lower(char *s);
char *vely_upper(char *s);
void vely_location (char **fname, num *lnum, num set);
void vely_store_init (vely_fifo **fdata);
void vely_store (vely_fifo *fdata, char *name, void *data);
void vely_retrieve (vely_fifo *fdata, char **name, void **data);
void vely_rewind (vely_fifo *fdata);
void vely_purge (vely_fifo **fdata, char recreate);
num vely_lockfile(char *filepath, num *lock_fd);
void vely_append_string (char *from, char **to);
void vely_get_runtime_options();
void vely_out_file (char *fname, vely_header *header);
num vely_strncpy(char *dest, char *src, num max_len);
num vely_getpid ();
num vely_post_url_with_response(char *url, char **result, char **head, char **error, char *cert, char *cookiejar, num *resp_code, long timeout, char post, char *fields[], char *files[], vely_header *vh, char *method, char *payload, num payload_len);
num vely_copy_file (char *src, char *dst);
void vely_b64_decode (char* in, num ilen, char** out, num* olen);
void vely_b64_encode(char* in, num in_len, char** out, num* olen);
num vely_read_file (char *name, char **data, num pos, num len);
num vely_read_file_id (FILE *f, char **data, num pos, num len, bool ispos);
num vely_is_number (char *s, num *prec, num *scale, num *positive);
void vely_clear_config();
void vely_init_header (vely_header *header, num init_type, char is_request);
num vely_write_file (char *file_name, char *content, num content_len, char append, num pos, char ispos);
num vely_write_file_id (FILE *f, char *content, num content_len, char append, num pos, char ispos);
num vely_get_file_pos(FILE *f, num *pos);
num vely_set_file_pos(FILE *f, num pos);
num vely_reg_file(FILE **f);
VV_MEMINLINE void vely_clear_mem (num ind);
char *vely_web_name(char *url);
void vely_check_transaction(num check_mode);
void vely_break_down (char *value, char *delim, vely_split_str **broken);
void vely_delete_break_down (vely_split_str **broken_ptr);
char * vely_get_tz ();
vely_dbc *vely_execute_SQL (char *s,  num *rows, char **er, char **err_message, num returns_tuples, num user_check, char is_prep, void **prep, num paramcount, char **params, char erract);
char *vely_time (char *timezone, char *format, num year, num month, num day, num hour, num min, num sec);
num vely_encode_base (num enc_type, char *v, num vLen, char **res, num allocate_new);
void vely_make_random (char **rnd, num rnd_len, char type, bool crypto);
void vely_checkmem ();
num vely_copy_data_from_num (char **data, num val);
void file_too_large(vely_input_req *iu, num max_size);
void oops(vely_input_req *iu, char *err);
num vely_total_so(vely_so_info **sos);
FILE *vely_fopen (char *file_name, char *mode);
int vely_fclose (FILE *f);
#ifdef VV_INC_PCRE2
num vely_regex(char *look_here, char *find_this, char *replace, char **res, num utf8, num case_insensitive, num single_match, regex_t **cached, num *reslen);
void vely_regfree(regex_t *preg);
#endif 
void vely_set_env(char *arg);
char * vely_os_name();
char * vely_os_version();
char *vely_home_dir();
void vely_FCGI_Finish (void);
num vely_FCGI_Accept (void);
void vely_exit_request(num retval);
void vely_error_request(num retval);
void _startup ();
void _after();
void _before ();
char *vely_basename (char *path);
char *vely_realpath (char *path);
void vely_end_connection(num close_db);
char *vely_find_keyword0(char *str, char *find, num has_spaces, num paren);
void vely_db_prep(void **prep);
char *vely_db_prep_text(char *t);
int vely_db_escape(char *from, char *to, num *len);
void vely_hex2bin(char *src, char **dst, num ilen, num *olen);
void vely_bin2hex(char *src, char **dst, num ilen, num *olen, char *pref);
void vely_db_free_result (char is_prep);
num vely_json_new (char *val, num *curr, num len, char dec);
void vely_set_json (vely_json **j, num maxhash, char nohash, vely_json_node_handler nodeh);
num vely_topower(num b,num p);
num vely_read_json (vely_json *j, char *key, char **keylist, char **to, num *type);
char *vely_json_err();
void vely_del_json (vely_json *j);
num vely_decode_utf8 (num32 u, unsigned char *r, char **e);
num vely_encode_utf8 (char *r, num32 *u, char **e);
num32 vely_make_from_utf8_surrogate (num32 u0, num32 u1);
void vely_get_utf8_surrogate (num32 u, num32 *u0, num32 *u1);
void vely_create_hash (vely_hash **hres_ptr, num size);
void vely_delete_hash (vely_hash **h, char recreate);
void *vely_find_hash (vely_hash *h, char *key, char **keylist, char del, num *found, char **oldkey);
char *vely_add_hash (vely_hash *h, char *key, char **keylist, void *data, num *st, char **oldkey);
char *vely_next_hash(vely_hash *h, void **data);
void vely_del_hash_traverse (vely_hash *h);
void vely_del_hash_entry (vely_hash *h, vely_hash_table *todel, vely_hash_table *prev, num hashind);
void vely_rewind_hash(vely_hash *h);
num vely_total_hash (vely_hash *h);
num vely_hash_size (vely_hash *h);
void vely_resize_hash (vely_hash **h, num newsize);
dbl vely_hash_reads (vely_hash *h);
void vely_begin_json (vely_json *j);
num vely_next_json (vely_json *j, char **key, char **to, num *type);
char *vely_json_to_utf8 (char *val, char quoted, char **o_errm, char dec);
num vely_utf8_to_json (char *val, num len, char **res, char **err);
char *vely_getheader(char *h);
void vely_bad_request ();
num vely_set_input (vely_input_req *req, char *name, char *val);
char *vely_getpath ();
int vely_fcgi_client_request (char *fcgi_server, char *req_method, char *path_info, char *script_name, char *content_type, int content_len, char *payload);
void vely_flush_out(void);
void vely_end_all_db();
void vely_exit (void);
void vely_unmanaged();
void vely_managed();
void vely_mrestore();

#ifdef VV_INC_FCGI
void vv_set_fcgi (vv_fc **callin, char *server, char *req_method, char *app_path, char *req, char *url_payload, char *ctype, char *body, int clen, int timeout, char **env);
void vv_fc_delete (vv_fc *callin);
num vv_call_fcgi (vv_fc **req, num threads, num *finokay, num *started);
#endif

#ifdef VV_INC_CRYPTO
void vely_sec_load_algos(void);
num vely_get_enc_key(char *password, char *salt, num salt_len, num iter_count, EVP_CIPHER_CTX *e_ctx, EVP_CIPHER_CTX *d_ctx,  char *cipher_name, char *digest_name);
char *vely_encrypt(EVP_CIPHER_CTX *e, const unsigned char *plaintext, num *len, num is_binary, unsigned char *iv);
char *vely_decrypt(EVP_CIPHER_CTX *e, unsigned char *ciphertext, num *len, num is_binary, unsigned char *iv);
int vely_RAND_bytes(unsigned char *buf, int num);
#endif

#ifdef VV_INC_POSTGRES
void vely_pg_close();
num vely_pg_nfield();
vely_dbc *vely_pg_connect (num abort_if_bad);
num vely_pg_exec(char *s, num returns_tuple, char is_prep, void **prep, num paramcount, char **params);
num vely_pg_affected();
char *vely_pg_fieldname(num fnum);
void vely_pg_free();
num vely_pg_nrows();
void vely_pg_rows(char ***row, num num_fields, num nrow, unsigned long **lens);
char *vely_pg_error(char *s);
char *vely_pg_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er);
num vely_pg_checkc();
void vely_pg_close_stmt (void *st);
int vely_pg_escape(char *from, char *to, num *len);
#endif
#ifdef VV_INC_SQLITE
char *vely_lite_error(char *s, char is_prep);
void vely_lite_close ();
vely_dbc *vely_lite_connect (num abort_if_bad);
void vely_lite_insert_id(char *val, num sizeVal);
num vely_lite_affected(char is_prep);
num vely_lite_exec(char *s, char is_prep, void **prep, num paramcount, char **params);
int vely_lite_store(char is_prep);
int vely_lite_use(char is_prep);
num vely_lite_nfield();
char *vely_lite_fieldname();
void vely_lite_free(char is_prep);
num vely_lite_nrows();
int vely_lite_rows (char ***row, unsigned long **lens);
char *vely_lite_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er, char is_prep);
num vely_lite_checkc();
void vely_lite_close_stmt (void *st);
int vely_lite_escape(char *from, char *to, num *len);
#endif

#ifdef VV_INC_MARIADB
char *vely_maria_error(char *s, char is_prep);
void vely_maria_close ();
vely_dbc *vely_maria_connect (num abort_if_bad);
void vely_maria_insert_id(char *val, num sizeVal);
num vely_maria_affected(char is_prep);
num vely_maria_exec(char *s, char is_prep, void **prep, num paramcount, char **params);
int vely_maria_store(char is_prep);
int vely_maria_use(char is_prep);
num vely_maria_nfield();
char *vely_maria_fieldname();
void vely_maria_free();
num vely_maria_nrows(char is_prep);
int vely_maria_rows (char ***row, unsigned long **lens, char is_prep);
char *vely_maria_errm(char *errm, num errmsize, char *s, char *sname, num lnum, char *er, char is_prep);
num vely_maria_checkc();
void vely_maria_close_stmt (void *st);
int vely_maria_escape(char *from, char *to, num *len);
#endif

// 
// Globals
//
// Empty string. Its own value is the pointer itself, which is used
// as a differentiator between allocated and un-allocated strings.
// A string that points to 'VV_EMPTY_STRING' is not allocated, and cannot be
// realloc-ed, otherwise it can.
//
extern char *VV_EMPTY_STRING;
extern vely_config *vely_pc;
extern jmp_buf vely_jmp_buffer;
extern volatile num vely_done_setjmp;
extern jmp_buf vely_err_jmp_buffer;
extern volatile num vely_done_err_setjmp;
extern volatile num vely_in_fatal_exit;
extern char * vely_app_name;
extern char * vely_url_path;
extern char * vely_app_path;
extern num vely_max_upload;
extern num vely_is_trace;
extern int vely_errno;
extern int vely_stmt_cached;
extern bool vely_mem_os;


// DO not include velyapp.h for Vely itself, only for applications at source build time
#if VV_APPMAKE==1
// include generated Vely include file
#include "velyapp.h"
       
#endif

#endif





