// SPDX-License-Identifier: EPL-2.0
// If format is NULL, flush the output.
// Copyright 2019 DaSoftver LLC.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework. 

// 
// Main VELY processor. Takes input and output parameters from the command line
// and generates C file ready for compilation.
//

#include "vely.h"


// 
//
// Defines (most of them)
//
//

// Maximum number of SQL queries in a single file, otherwise unlimited in application
// Queries's code is generated based on the name of the query, and not the index into an array.
// This limit is for internal code generation purposes.
#define VV_MAX_QUERY 300
// level of queries to allow for nesting (i.e. query inside a query)
#define VV_MAX_QUERY_NESTED 10
// maximum # of input parameters, '%s' in a query, per query
#define VV_MAX_QUERY_INPUTS 200
// maximum length of input parameter, be it C code, or a string. This is the length of what goes in the 
// SQL query input, per input.
#define VV_MAX_QUERY_INPUT_LEN 250
// maximum length of input line in a  source code file.
#define VV_FILE_LINE_LEN 8096
// maximum length of file name
#define VV_FILE_NAME_LEN 200
// maximum space to write out all output columns of a query
#define VV_TOT_COLNAMES_LEN (VV_MAX_COLNAME_LEN * VV_MAX_QUERY_OUTPUTS)
// maximym length of query name, which is the name in run-query...=
#define VV_MAX_QUERYNAME_LEN 200
// keep track up to 10 levels if else-task other is repeated more than once
#define VV_MAX_OTHER_TASK 31 
// maximum length of any error this utility may produce
#define VV_MAX_ERR_LEN 12000
// various keywords used. What's recognized is a keyword. Only keywords that have something following them
// have a space afterwards. In is_opt_defined we check if there was a space before (there must be).
// No whitespace other than space can be used in commands!
#define VV_KEYSIZE0 "size"
#define VV_KEYSIZE "size "
#define VV_KEYWITHVALUE "with-value "
#define VV_KEYREPLACE "replace"
#define VV_KEYTOERROR "to-error"
#define VV_KEYTASKID "task-id"
#define VV_KEYPARAM "param "
#define VV_KEYEXIT "exit"
#define VV_KEYFROMREQUEST "from-request"
#define VV_KEYCREATE "create "
#define VV_KEYTYPE0 "type"
#define VV_KEYTYPE "type "
#define VV_KEYONERRORCONTINUE "on-error-continue"
#define VV_KEYONERROREXIT "on-error-exit"
#define VV_KEYON "on"
#define VV_KEYOFF "off"
#define VV_KEYERRNO "errno"
#define VV_KEYPREFIX "prefix "
#define VV_KEYNAME0 "name"
#define VV_KEYNAME "name "
#define VV_KEYPUT "put"
#define VV_KEYGET "get "
#define VV_KEYSET "set "
#define VV_KEYKEY "key "
#define VV_KEYVALUE "value "
#define VV_KEYREWIND "rewind "
#define VV_KEYPASSWORD "password "
#define VV_KEYINPUTLENGTH "input-length "
#define VV_KEYOUTPUTLENGTH "output-length "
#define VV_KEYITERATIONS "iterations "
#define VV_KEYSALT "salt "
#define VV_KEYSALTLENGTH "salt-length "
#define VV_KEYINITVECTOR "init-vector "
#define VV_KEYBINARY "binary"
#define VV_KEYHTML "html "
#define VV_KEYPOSITION "position "
#define VV_KEYUSE "use "
#define VV_KEYOTHER "other"
#define VV_KEYWITH "with "
#define VV_KEYDELIMITER "delimiter "
#define VV_KEYDELETE0 "delete"
#define VV_KEYDELETE "delete "
#define VV_KEYNOLOOP "no-loop"
#define VV_KEYOLDVALUE "old-value "
#define VV_KEYOLDKEY "old-key "
#define VV_KEYMAXHASHSIZE "max-hash-size "
#define VV_KEYCIPHER "cipher "
#define VV_KEYDIGEST "digest "
#define VV_KEYSKIPDATA "skip-data"
#define VV_KEYCRYPTO "crypto"
#define VV_KEYIN "in "
#define VV_KEYID "id "
#define VV_KEYTO "to "
#define VV_KEYSTATUS "status "
#define VV_KEYFROM "from "
#define VV_KEYFROMLENGTH "from-length "
#define VV_KEYTRAVERSE "traverse"
#define VV_KEYBEGIN "begin"
#define VV_KEYAPPEND "append"
#define VV_KEYLENGTH "length "
#define VV_KEYRESPONSE "response "
#define VV_KEYRESPONSECODE "response-code "
#define VV_KEYCERT "cert "
#define VV_KEYCOOKIEJAR "cookie-jar "
#define VV_KEYNUMBER "number"
// no-cert has NO space afterwards because it doesn't need anything to follow it
#define VV_KEYNOCERT "no-cert"
#define VV_KEYBUFFERED "buffered"
#define VV_KEYDEFAULT "default"
#define VV_KEYERROR "error "
#define VV_KEYERRORLENGTH "error-length "
#define VV_KEYERRORFILE "error-file "
#define VV_KEYERRORTEXT "error-text "
#define VV_KEYERRORPOSITION "error-position "
#define VV_KEYROWCOUNT "row-count "
#define VV_KEYAFFECTEDROWS "affected-rows "
#define VV_KEYNEWTRUNCATE "new-truncate"
#define VV_KEYDEFINED "define "
#define VV_KEYSECURE "secure"
#define VV_KEYEXPIRES "expires "
#define VV_KEYAPPPATH "app-path "
#define VV_KEYPATH "path "
#define VV_KEYPATH0 "path"
#define VV_KEYFILEID "file-id "
#define VV_KEYSAMESITE "same-site "
#define VV_KEYDIRECTORY "directory"
#define VV_KEYPROCESSDATA "process-data "
#define VV_KEYTRACEDIRECTORY "trace-directory"
#define VV_KEYFILEDIRECTORY "file-directory"
#define VV_KEYUPLOADSIZE "upload-size"
#define VV_KEYDBVENDOR "db-vendor "
#define VV_KEYTRACEFILE "trace-file"
#define VV_KEYNOHTTPONLY "no-http-only"
#define VV_KEYBLOCKSIZE "block-size "
#define VV_KEYINIT "init"
#define VV_KEYOUTPUT "output "
#define VV_KEYUNKNOWNOUTPUT "unknown-output"
#define VV_KEYMETHOD "method "
#define VV_KEYLOCATION "location "
#define VV_KEYARGS "args "
#define VV_KEYAVERAGEREADS "average-reads "
#define VV_KEYOUTPUTFILE "output-file "
#define VV_KEYINPUTFILE "input-file "
#define VV_KEYSUBJECT "subject "
#define VV_KEYHEADER "header "
#define VV_KEYHEADERS "headers "
#define VV_KEYRESPONSEHEADERS "response-headers "
#define VV_KEYREQUESTHEADERS "request-headers "
#define VV_KEYREQUESTPATH "request-path "
#define VV_KEYFILES "files "
#define VV_KEYFIELDS "fields "
#define VV_KEYREQUESTBODY "request-body "
#define VV_KEYCONTINUE "continue"
#define VV_KEYCONTENT "content "
#define VV_KEYCONTENTTYPE "content-type "
#define VV_KEYCONTENTLENGTH "content-length "
#define VV_KEYURLPAYLOAD "url-payload "
#define VV_KEYTHREADID "thread-id "
#define VV_KEYETAG "etag"
#define VV_KEYFILENAME "file-name "
#define VV_KEYNOCACHE "no-cache"
#define VV_KEYCACHECONTROL "cache-control "
#define VV_KEYCACHE "cache"
#define VV_KEYCLEARCACHE "clear-cache "
#define VV_KEYSTATUSID "status-id "
#define VV_KEYSTATUSTEXT "status-text "
#define VV_KEYCUSTOM "custom "
#define VV_KEYDOWNLOAD "download"
#define VV_KEYEQUAL "=="
#define VV_KEYEQUALSHORT "="
#define VV_KEYAT "@"
#define VV_KEYCOLON ":"
#define VV_KEYNOTEQUAL "!="
#define VV_KEYAND "and "
#define VV_KEYOR "or "
#define VV_KEYCOLUMNCOUNT "column-count "
#define VV_KEYCOLUMNNAMES "column-names "
#define VV_KEYCOLUMNDATA "column-data "
#define VV_KEYNOENCODE "noencode"
#define VV_KEYWEBENCODE "webencode"
#define VV_KEYURLENCODE "urlencode"
#define VV_KEYDATA "data "
#define VV_KEYDATALENGTH "data-length "
#define VV_KEYREQUESTSTATUS "request-status "
#define VV_KEYYEAR "year "
#define VV_KEYMONTH "month "
#define VV_KEYDAY "day "
#define VV_KEYHOUR "hour "
#define VV_KEYMIN "minute "
#define VV_KEYSEC "second "
#define VV_KEYTIMEOUT "timeout "
#define VV_KEYTIMEZONE "timezone "
#define VV_KEYFORMAT "format "
#define VV_KEYREPLACEWITH "replace-with "
#define VV_KEYRESULT "result "
#define VV_KEYBYTESWRITTEN "bytes-written "
#define VV_KEYNOTRIM "notrim"
#define VV_KEYCASEINSENSITIVE "case-insensitive"
#define VV_KEYUTF8 "utf8"
#define VV_KEYTEMPORARY "temporary"
#define VV_KEYSINGLEMATCH "single-match"
#define VV_KEYENVIRONMENT "environment "
#define VV_KEYWEBENVIRONMENT "web-environment "
#define VV_KEYOSNAME "os-name"
#define VV_KEYOSVERSION "os-version"
#define VV_KEYARGCOUNT "arg-count"
#define VV_KEYARGVALUE "arg-value "
#define VV_KEYINPUTCOUNT "input-count"
#define VV_KEYINPUTVALUE "input-value "
#define VV_KEYINPUTNAME "input-name "
#define VV_KEYREFERRINGURL "referring-url"
#define VV_KEYPROCESSID "process-id"
#define VV_KEYCOOKIECOUNT "cookie-count"
#define VV_KEYCOOKIE "cookie "
#define VV_KEYINPUT "input "
#define VV_KEYFINISHEDOKAY "finished-okay "
#define VV_KEYARRAYCOUNT "array-count "
#define VV_KEYSTARTED "started "
// Type for 'define' statements
#define VV_DEFSTRING 1
#define VV_DEFNUM 4
#define VV_DEFVOIDPTRPTR 6
#define VV_DEFBROKEN 8
#define VV_DEFJSON 9
#define VV_DEFHASH 10
#define VV_DEFDBL 11
#define VV_DEFFIFO 12
#define VV_DEFCHARPTRPTR 13
#define VV_DEFVOIDPTR 14
#define VV_DEFENCRYPT 15
#define VV_DEFFILE 16
#define VV_DEFFCGI 18
// maximum length of generated code line (in .c file, final line)
#define VV_MAX_CODE_LINE 4096
// error messages
#define VV_NAME_INVALID "Name [%s] is not valid, must be a valid C identifier because a variable is required in this context (such as with define subclause or statements like input-param)"
#define VV_MSG_NESTED_QRY "Qry ID [%lld] is nested too deep, maximum nesting of [%d]"
// vely ID stuff
#define TOOL "Vely"
#define TOOL_CMD "vely"
// maximum length of a column name in db select query result 
#define VV_MAX_COLNAME_LEN 64
// pre-processing status of a qry, describes the way it is used in the code
#define VV_QRY_UNUSED 0
#define VV_QRY_USED 1
#define VV_QRY_INACTIVE 0
#define VV_QRY_ACTIVE 1

//guard against if (..) vely_statement, causes if (..) char ...; which is illegal
#define VV_GUARD   oprintf("char _vely_statement_must_be_within_code_block_here_%lld; VV_UNUSED (_vely_statement_must_be_within_code_block_here_%lld);\n", vnone, vnone), vnone++;
// check if query returns a tuple, if not, cannot use row-count on it
#define VV_CHECK_TUPLE(k) if (gen_ctx->qry[k].returns_tuple == 0) _vely_report_error( "row-count cannot be used on query [%s] because it does not output any columns", gen_ctx->qry[k].name)
// current query id - 0 for the first, 1 for nested in it, 2 for nested further, then 0 again for the one next to the first etc.
#define query_id (gen_ctx->curr_qry_ptr-1)

// cipher+digest for encryption (default)
#define VV_DEF_DIGEST "\"sha256\""
#define VV_DEF_CIPHER "\"aes-256-cbc\""


//
//
// Type definitions
//
//

// 
// Query structure, describes run-query and all that it entails
//
typedef struct vely_qry_info_t
{
    char *text; // sql text of query
    char name[VV_MAX_QUERYNAME_LEN + 1]; // name of query, generated by Vely and used for naming variables and such

    num returns_tuple; // 0 if this doesn't return a tuple, like INSERT, but not INSERT..RETURNING(), 1 otherwise

    // 1 if unknown outputs
    char unknown_output;

    // number of, and qry outputs 
    num qry_total_outputs; 
    bool qry_out_defined; // true if all outputs defined automatically as string
    char *qry_outputs[VV_MAX_QUERY_OUTPUTS + 1];
    
    // input parameters from URL bound to each query
    char qry_inputs[VV_MAX_QUERY_INPUTS + 1][VV_MAX_QUERY_INPUT_LEN + 1];

    // number of query inputs actually found as opposed to the number of '%s'
    num qry_found_total_inputs; 

    // the database on which this index operates, used to switch back when end-query is encountered
    // note this is not spanning the code, but only set when needed. In other words, *every* db operation
    // has this set.
    num ind_current_db;
} qry_info;
// 
// Context data for VELY preprocessor. Used to hold information
// during the preprocessing.
//
typedef struct vely_gen_ctx_s
{
    // list of db queries - actual SQL with %s for input params
    // from URL
    qry_info qry[VV_MAX_QUERY + 1]; // used for generation code phase only

    num qry_used[VV_MAX_QUERY + 1]; // whether the query was used in the past or not
    num qry_active[VV_MAX_QUERY + 1]; // whether the query is active now or not
    num total_queries; // total number of queries that we have. 
    num total_write_string; // used to detect unclosed write-strings

    // when nesting, query IDs are indexes into qry_info.
    // curr_qry_ptr pointing to one just above the deepest. 
    num curr_qry_ptr;


} vely_gen_ctx;

//
// Database connections
//
typedef struct connections_s
{
    char *name;
} connections;

// For use with run-query
// pointers to parsed out statement parts, must be all NULL to begin with (meaning
// they are not found by default),
// what each has is self-explanatory, for instance eq is the data right after = 
// we make sure no other option can ever be
// on_error_action is VV_ON_ERROR_CONTINUE/EXIT for statement-specific on-error continue/exit or VV_OKAY if db-level on-error is in effect.
//
typedef struct s_vely_db_parse {
    char on_error_action;
    char *on_error_exit;
    char *on_error_cont;
    char *with_output;
    char *with_unknown_output;
    char *colon;
    char *query_result;
    char *eq;
    char *at;
    char *noloop;
    char *colcount;
    char *coldata;
    char *colnames;
    char *err;
    char *errtext;
    char *arows;
    char *rcount;
    char *name;
} vely_db_parse;


// 
//
// Global variables (and a few related macros)
//
//

bool other_task_done[VV_MAX_OTHER_TASK+1] = {false};
num vv_plain_diag = 0; // use color diagnostics
num vnone = 0; // used in detecting if () vely_statement
char line[VV_FILE_LINE_LEN + 1]; // buffer for each line from HTML file
num open_queries = 0; // number of queries currently open
num vely_is_trace = 0; // is tracing 0 or 1
num vely_max_upload = 25000000; // max upload default
char *app_path = ""; // app prefix not defined
char *vely_app_name=NULL;
char *vely_dbconf_dir = NULL;
char *vely_bld_dir = NULL;
num lnum = 0; // current line worked on 
char *src_file_name = NULL; // file_name of the source
num print_mode = 0; // 1 if @  to print out
num die_now = 0; // used in vely run-time only for now to gracefully exit program, not used here (not yet,
                // and maybe not ever).
FILE *outf = NULL; // vely output file (.c generated file)
num usedVELY = 0; // 1 if vely statement is used on the line
num vely_is_inline = 0; // if 1, the last statement just processed was inline
num within_inline = 0; // if statement is within <<...>>
num last_line_readline_closed = 0; // line number of the last read-line that has closed
num last_line_if_closed = 0; // line number of the last IF that has closed
// setup variables to be able to report the location of unclosed readline
#define check_next_readline {if (open_readline==0) last_line_readline_closed = lnum; open_readline++;}
// setup variables to be able to report the location of unclosed IF
num last_line_query_closed = 0; // line number of where the last query closed
// setup variables to report the location of unclosed query
#define check_next_query {if (open_queries==0) last_line_query_closed = lnum; open_queries++;}
num verbose = 0; // 1 if verbose output of preprocessing steps is displayed
num no_vely_line = 0; // if 1, no #line generated
num total_exec_programs=0; // enumerates instance of exec-program so generated argument array variables are unique
num total_body=0; // enumerates instance of call-web for body fields and files
num total_fcgi_arr=0; // enumerates instance of new-server for env
// Application name - we must know it in order to figure out database config file
// Database connections
vely_db_connections vely_dbs;
num totconn = 0;
//
// satisfy gcc, this is for signal handling for FCGI/standalone programs, no purpose in vv
num vely_in_request=0;
num volatile vely_done_err_setjmp=0;
num volatile vely_in_fatal_exit=0;
num volatile vely_done_setjmp=0;
char vely_query_id_str[VV_MAX_QUERYNAME_LEN + 1]; // query ID being worked on

//
//
// Function/macro declarations
//
//
void init_vely_gen_ctx (vely_gen_ctx *gen_ctx);
void vely_gen_c_code (vely_gen_ctx *gen_ctx, char *file_name);
void _vely_report_error (char *format, ...)  __attribute__ ((format (printf, 1, 2)));
num recog_statement (char *cinp, num pos, char *opt, char **mtext, num *msize, num isLast, num *is_inline);
num find_query (vely_gen_ctx *gen_ctx);
void new_query (vely_gen_ctx *gen_ctx, vely_db_parse *vp);
num get_col_ID (vely_gen_ctx *gen_ctx, num qry_name, char *column_out);
void oprintf (char *format, ...)  __attribute__ ((format (printf, 1, 2)));
void get_passed_whitespace (char **s);
void get_until_comma (char **s);
void get_until_whitespace (char **s);
void end_query (vely_gen_ctx *gen_ctx, num close_block);
void get_all_input_param (vely_gen_ctx *gen_ctx, char *iparams);
num terminal_width();
void out_verbose(num vely_line, char *format, ...);
void add_input_param (vely_gen_ctx *gen_ctx, char *inp_par);
void carve_statement (char **statement, char *statement_name, char *keyword, num is_mandatory, num has_data);
num define_statement (char **statement, num type);
// true if to emit line number in source file, do not do it if vely -x used, lnum>1 is so that the very first line
// isn't emitting #line twice
#define VV_EMIT_LINE (no_vely_line == 0 && lnum>1)
#define  VV_VERBOSE(lnum,...) out_verbose(lnum,  __VA_ARGS__)
void parse_param_list (char *parse_list, vely_fifo **params, num *tot);
void is_opt_defined (char **option, num *is_defined);
char *find_keyword(char *str, char *find, num has_spaces);
num find_connection(char *conn);
num add_connection(char *conn, num dbtype);
void get_db_config (char *dbname);
void get_query_output (vely_gen_ctx *gen_ctx, vely_db_parse *vp);
void statement_SQL (vely_gen_ctx *gen_ctx, char is_prep);
void start_loop_query (vely_gen_ctx *gen_ctx);
void generate_sql_code (vely_gen_ctx *gen_ctx, char is_prep);
void generate_query_result (vely_gen_ctx *gen_ctx, vely_db_parse *vp);
void query_execution (vely_gen_ctx *gen_ctx,const num run_query, const num query_result, char is_prep, vely_db_parse *vp);
void prepare_query (vely_gen_ctx *gen_ctx, vely_db_parse *vp);
void process_query (vely_gen_ctx *gen_ctx, const num query_result, const num run_query, char is_prep, vely_db_parse *vp);
char *vely_db_vendor(num dbconn);
void trimit(char *var);
void check_vely (char *c);
char *id_from_file (char *file);
num outargs(char *args, char *outname, char *type, num startwith, char pair);
void process_http_header (char *statement, char *header, char *temp_header, num http_header_count, char request, char **content_len, char **content_type);
char *make_default_header(int inittype, num http_header_count, char request);
void on_error_act (char *on_error_cont, char *on_error_exit, char *act);
void envsub ();
void vely_is_valid_app_path (char *name);
void query_result (vely_gen_ctx *gen_ctx, char *mtext);
char *opt_clause(char *clause, char *param, char *antiparam);
void name_query (vely_gen_ctx *gen_ctx, vely_db_parse *vp);
void free_query (char *qryname, bool skip_data);


//
//
// Implementation of functions used in VELY alone
//
//



//
// Returns code for optional clause. This is a clause where it may not have any data
// or it may. For example:
// set-cookie ... secure
// or
// set-cookie ... secure true|false
// If true|false, then it's either secure or not. Without true|false then it's secure.
// This helps write easier code.
// clause is "secure" in this case (the clause), "param" is what is generated if it applies
// and "antiparam" is what is generated if it does not apply which is if clause is NULL or
// it's there but false.
//
char *opt_clause(char *clause, char *param, char *antiparam)
{
    char *res = NULL;
    if (clause != NULL)
    {
        if (clause[0] != 0) // there is true|false
        {
            res = vely_malloc(strlen(clause)+strlen(param)+strlen(antiparam) + 30);
            sprintf (res, "((%s)?(%s):(%s))", clause, param, antiparam);
        }
        else // clause is present but no true|false
        {
            res = vely_strdup(param);
        }
    }
    else // clause is not present (NULL)
    {
        res = vely_strdup(antiparam);
    }
    return res;
}

//
// This processes query-result statement, see documentation
//
void query_result (vely_gen_ctx *gen_ctx, char *mtext)
{
    vely_db_parse vp;
    memset  (&vp, 0, sizeof(vely_db_parse)); // all are NULL now

    vp.query_result = vely_strdup (mtext);

    process_query (gen_ctx, 1, 0, 0, &vp);
}

//
// Checks if app-path is valid
// Valid paths are considered to have only alphanumeric characters and
// underscores, hyphens and forward slashes.
// Must end with "/appname"
// Returns 1 if name is valid, 0 if not.
//
void vely_is_valid_app_path (char *name)
{
    VV_TRACE ("");
    assert (name);

    num i = 0;
    while (name[i] != 0)
    {
        if (!isalnum(name[i]) && name[i] != '_' && name[i] != '-' && name[i] != '/') _vely_report_error ("Application path is not valid; it can contain only alphanumeric characters, underscores, hyphens and forward slashes");
        i++;
    }
    num l = (num)strlen(vely_app_name);
    // app patch must end with /<app name>
    if (i < l+1 || strcmp (name+i-l, vely_app_name) || name[i-l-1] != '/') _vely_report_error ("Application path [%s] must end with /%s", name, vely_app_name);
}


//
// Reads stdin, and substs $<curlybrace>XYZ<curlybrace> to the stdout. Max var len 250. Non-existent is empty. 
// Anything bad with $<curlybrace>XYZ<curlybrace> just
// copied to the stdout. XYZ can be alphanumeric or underscore.
// This does NOT and MUST NOT use vely_malloc. If it does, the calling of envsub in main() must be after the mem init.
//
void envsub () 
{
    char varname[256];
    num varlen = 0;
    while (1)
    {
        // read input until error or EOF
        int c = fgetc(stdin);
        if (c == EOF) break;
        // Treat dollar separately, anything else goes to stdout
        if (c == '$')
        {
            // Expecting open brace after $
            int c = fgetc(stdin);
            // If end of file, print $ out and exit
            if (c == EOF) { fputc('$', stdout); break; }
            // Start of variable $<curlybrace>...<curlybrace>
            if (c == '{')
            {
                // get variable name
                varlen = 0;
                while (1)
                {
                    // one by one char variable name
                    int c = fgetc(stdin);
                    // check if variable name ended with closed brace
                    if (c == '}')
                    {
                        if (varlen == 0)
                        {
                            // nothing in it, just print out weird construct
                            fputs ("${}", stdout);
                            break;
                        }
                        else
                        {
                            // found var name, get its value, print it out. Account for empty or non-existing.
                            varname[varlen] = 0;
                            char *ev = secure_getenv (varname);
                            if (ev == NULL) ev="";
                            fputs (ev, stdout);
                            break;
                        }
                    }
                    // If this is isn't a variable, print it all out. It could be the end of stream,
                    // variable name isn't alphanum or underscore, or too long. We just then ignore it and print it out.
                    if (c == EOF || !(isalnum(c) || c == '_') || varlen >= (num)sizeof(varname)-2)
                    {
                        fputs ("${", stdout);
                        if (c != EOF) varname[varlen++] = c; // include the offending char unless EOF
                        varname[varlen] = 0;
                        fputs (varname, stdout);
                        break;
                    }
                    // Add read-in char to variable name
                    varname[varlen++] = c;
                }
            } else 
            {
                // this dollar with something other than <curlybrace> following
                fputc ('$', stdout);
                fputc (c, stdout);
            }
        } else fputc(c, stdout); // not a dollar
    }
}


//
// Based on on-error statement on_error, determine if it's continue or exit (or error out if neither)
// on_error_cont/exit is carved out on-error-cont/exit value, act is either VV_ON_ERROR_CONTINUE/EXIT or VV_OKAY
//
void on_error_act (char *on_error_cont, char *on_error_exit, char *act)
{
    if (on_error_cont != NULL) *act = VV_ON_ERROR_CONTINUE;
    else if (on_error_exit != NULL) *act = VV_ON_ERROR_EXIT;
    else *act = VV_OKAY;

    // check for sanity
    if (on_error_cont != NULL && on_error_exit != NULL) _vely_report_error ("on-error can be either 'continue' or 'exit', but not both");
}

//
// Generate code to create and init default vely_header. 'inittype' is VV_HEADER_FILE for file serving, i.e.
// when there's caching; VV_HEADER_PAGE for page serving, i.e when there's no caching.
// http_header_count is the ID of the temp vely_header-type.
// Returns the name of the vely_header variable for which the code is generated.
// request is 0 if this is reply header, or 1 if request. Some http headers are just for reply and vice versa.
//
char *make_default_header(int inittype, num http_header_count, char request)
{
    static char temp_header[200];
    if (inittype != VV_HEADER_FILE && inittype != VV_HEADER_PAGE) _vely_report_error( "unsupported header type [%d] (internal)", inittype);
    snprintf (temp_header, sizeof(temp_header), "_vely_tmp_header_%lld", http_header_count); 
    oprintf ("vely_header %s;\n", temp_header);
    // by default it is to show the file (disposition is empty)
    // use disposition as "attachment" or "download" for download
    oprintf ("vely_init_header (&(%s), %d, %d) ;\n", temp_header, inittype, request);
    return temp_header;
}

//
// Generates code that constructs vely_header variable. User can specify content-type, download, etag and others.
// Also allows for "custom" header in the form of expr=expr,... making it easy to create custom headers.
// "statement" is the name of the statement (such as "send-file"). "header" is the actual text of headers clause in the
// statement. temp_header is the name of the vely_header var generated via make_default_header().
// http_header_count is the ID of the temp vely_header-type, the same as used in make_default_header().
// request is 0 if this is reply header, or 1 if request. Some http headers are just for reply and vice versa.
// content_type is content-type if set, NULL otherwise,
// content_len is content-length if set, NULL otherwise (originally vely_header was to be used to set the values, but
// there is an issue of types: here all types are char*, while at run-time, they can be different); can be NULL.
//
void process_http_header (char *statement, char *header, char *temp_header, num http_header_count, char request, char **content_len, char **content_type)
{
    if (content_len != NULL) *content_len = NULL;
    if (content_type != NULL) *content_type = NULL;
    if (header != NULL)
    {
        // process header clause
        char *ctype = NULL;
        char *clen = NULL;
        char *download = NULL;
        char *cachecontrol = NULL;
        char *nocache = NULL;
        char *etag = NULL;
        char *filename = NULL;
        char *statusid = NULL;
        char *statustext = NULL;
        char *custom = NULL;
        // get fixed fields
        char *mheader = vely_strdup (header);
        if (request == 0)
        {
            download = find_keyword (mheader, VV_KEYDOWNLOAD, 1);
            etag = find_keyword (mheader, VV_KEYETAG, 1);
            filename = find_keyword (mheader, VV_KEYFILENAME, 1);
            cachecontrol = find_keyword (mheader, VV_KEYCACHECONTROL, 1);
            nocache = find_keyword (mheader, VV_KEYNOCACHE, 1);
            statusid = find_keyword (mheader, VV_KEYSTATUSID, 1);
            statustext = find_keyword (mheader, VV_KEYSTATUSTEXT, 1);
        }
        if (request == 1) 
        {
            clen = find_keyword (mheader, VV_KEYCONTENTLENGTH, 1);
        }
        ctype = find_keyword (mheader, VV_KEYCONTENTTYPE, 1);
        custom = find_keyword (mheader, VV_KEYCUSTOM, 1);
        if (request == 0)
        {
            carve_statement (&download, statement, VV_KEYDOWNLOAD, 0, 2);
            carve_statement (&etag, statement, VV_KEYETAG, 0, 2);
            carve_statement (&filename, statement, VV_KEYFILENAME, 0, 1);
            carve_statement (&cachecontrol, statement, VV_KEYCACHECONTROL, 0, 1);
            carve_statement (&nocache, statement, VV_KEYNOCACHE, 0, 0);
            carve_statement (&statusid, statement, VV_KEYSTATUSID, 0, 1);
            carve_statement (&statustext, statement, VV_KEYSTATUSTEXT, 0, 1);
        }
        if (request == 1) 
        {
            carve_statement (&clen, statement, VV_KEYCONTENTLENGTH, 0, 1);
        }
        carve_statement (&ctype, statement, VV_KEYCONTENTTYPE, 0, 1);
        carve_statement (&custom, statement, VV_KEYCUSTOM, 0, 1);

        char *downloadc = opt_clause(download, "\"attachment\"", "NULL");
        char *etagc = opt_clause(etag, "1", "0");

        if (nocache != NULL && cachecontrol != NULL) _vely_report_error( "both no-cache and cache-control specified, only one can be used");
        // gen header
        if (ctype != NULL) { if (content_len != NULL) *content_type = ctype;  oprintf ("(%s).ctype = %s;\n", temp_header, ctype); }
        if (clen != NULL) {if (content_len != NULL) *content_len = clen; oprintf ("(%s).clen = %s;\n", temp_header, clen);}
        if (download != NULL) oprintf ("(%s).disp = %s;\n", temp_header, downloadc);
        if (etag != NULL) oprintf ("(%s).etag = %s;\n", temp_header, etagc);
        if (filename != NULL) oprintf ("(%s).file_name = %s;\n", temp_header, filename);
        if (statusid != NULL) oprintf ("(%s).status_id = %s;\n", temp_header, statusid);
        if (statustext != NULL) oprintf ("(%s).status_text = %s;\n", temp_header, statustext);
        if (cachecontrol != NULL) oprintf ("(%s).cache_control = %s;\n", temp_header, cachecontrol);
        if (nocache != NULL) oprintf ("(%s).cache_control = \"max-age=0, no-cache\";\n", temp_header);
        vely_free(downloadc);
        vely_free(etagc);
        // get custom fields
        if (custom != NULL)
        {
            char *mcustom = vely_strdup (custom);
            // store custom headers
            char chead_var[100];
            snprintf (chead_var, sizeof (chead_var), "_vv_chead_arr%lld", http_header_count);
            num tothead = outargs(mcustom, chead_var, "char *", 0, 1);
            num i;
            num j = 0;
            // This pointer copying, first to _vv_chead_arr, then to control/value will
            // be optimized away by gcc; no effect on performance
            for (i = 0; i < tothead; i+=2)
            {
                oprintf ("(%s).control[%lld]=_vv_chead_arr%lld[%lld];\n", temp_header, j, http_header_count, i);
                oprintf ("(%s).value[%lld]=_vv_chead_arr%lld[%lld+1];\n", temp_header, j, http_header_count, i);
                j++;
            }
        }
    }
}

//
// Generate code for 1) name=value,... statement, for example in call-web or 2) something,... statement
// for example in exec-program. args is the actual text of what's parsed in the statement.
// outname is the name of <type>* array where the results are stored in generated code. 
// startwith is 0 or 1, depending on whether to put the first element of the array
// in [0] or [1]. 'pair' is 0 for something,... or 1 for name=value,.... For name=value situation,
// array is filled so it has name,value,name,value,etc.
// 'something','name','value' can be quoted or in () in order to avoid conflicts.
// The last element in the generated array always has NULL in it.
// So type is the type of these elements.
// Returns the index of the last element in array, the one that has NULL in it (in the code generated for the array).
//
num outargs(char *args, char *outname, char *type, num startwith, char pair) 
{


    num exec_inputs=startwith; // we start with index 1 in order to fill in program name as arg[0] at run-time
                        // args[0] is filled with program name at run-time

    num tote;
    if (args != NULL)
    {
        num tot;
        // must duplicate string because we CHANGE curr_start in parse_param_list (because it removes
        // escaped quotes) and thus we change the 'line' (the current parsing line). As a result, we will
        // probably error out at the end of line because we will see null characters since the line has 
        // shifted to the left during removal of escaped quotes).
        char *curr_start = vely_strdup(args);

        // parse program arguments
        vely_fifo *params = NULL;
        parse_param_list (curr_start, &params, &tot);
        // define run-time list of program arguments, account for the last NULL
        // also account double when there's x=y pair, since that'll take 2 entries
        // add startwith if not zero to account for pre-filled elements
        oprintf("%s%s[%lld];\n", type,outname, tote=((pair == 1 ? 2:1)*tot+1+startwith));

        char *value;
        while (1)
        {
            vely_retrieve (params, NULL, (void*)&value);
            if (value==NULL) break;

            if (pair == 1) 
            {
                // find equal sign outside the quotes or parenthesis
                char *eq = find_keyword(value, "=", 0);
                if (eq == NULL) 
                {
                    _vely_report_error( "Name/value pair missing equal sign ('=') in [%s]", value);
                }
                *eq = 0;
                // generate code for name/value pair array (run time variable argument)
                oprintf("%s[%lld] = %s;\n", outname, exec_inputs, value);
                exec_inputs++;
                oprintf("%s[%lld] = %s;\n", outname, exec_inputs, eq+1);
                exec_inputs++;
            }
            else
            {
                // generate code for program arguments array (run time variable argument)
                oprintf("%s[%lld] = %s;\n", outname, exec_inputs, value);
                exec_inputs++;
            }
        } 
    } 
    else 
    {
        oprintf("%s%s[%lld];\n", type,outname, tote=startwith+1); // no list (otherthan prefilled
                                                                  // startwith, just one element for NULL
    }

    // final arg MUST be NULL
    oprintf("%s[%lld] = NULL;\n", outname, exec_inputs);
    if (exec_inputs >= tote) _vely_report_error( "Internal error in building a list [%lld, %lld]", exec_inputs, tote);

    return exec_inputs;
}

//
// Check if (maybe) command c looks like vely, and error out if it does
// (because it's likely misspelled vely command)
// Check for 'ab..-cd..<space|eol>' or 'ab..-cd..#'
//
void check_vely (char *c)
{
    // first find the space or #, or end-of-line
    num i = 0;
    while (!isspace(c[i]) && c[i] != '#' && c[i] != 0) i++;
    num j;
    num isvely = 1;
    num hasdash = 0;
    for (j = 0; j < i; j++)
    {
        if (c[j] == '-') hasdash = 1;
        if (!isalpha(c[j]) && c[j] != '-') {isvely = 0; break;}
    }
    if (hasdash == 1 && isvely == 1) 
    {
        _vely_report_error ("Unrecognized Vely statement [%s]", c);
    }
}

//
// Trim string var in-place
//
void trimit(char *var)
{
    num var_len = strlen (var);
    vely_trim (var, &var_len);
}

//
// Return name of database based on its index in the list of used dbs (dbconn)
// Return empty if not found or not recognized
//
char *vely_db_vendor(num dbconn)
{
    VV_TRACE("");
    if (dbconn < 0 || dbconn >= totconn ) return "";
    if (vely_get_config()->ctx.db->conn[dbconn].db_type == VV_DB_SQLITE) return "sqlite";
    if (vely_get_config()->ctx.db->conn[dbconn].db_type == VV_DB_MARIADB) return "mariadb";
    if (vely_get_config()->ctx.db->conn[dbconn].db_type == VV_DB_POSTGRES) return "postgres";
    return "";
}


//
// Create and return C variable name from file name 'file'
//
char *id_from_file (char *file)
{
    char *id = vely_strdup(file);
    num i;
    if (!isalpha(id[0])) id[0] = '_';
    for (i = 1; i < (num)strlen(id); i++)
    {
        if (!isalnum(id[i])) id[i] = '_';
    }
    return id;
}


// 
// Entire query processing is in here. All input params are here. 
// query_result, run_query  are the statements to process 
// vp contains all the pieces of any query statement parsed out
// is_prep is 1 if this is run-prepared-query, otherwise 0
// gen_ctx is execution context
//
void process_query (vely_gen_ctx *gen_ctx, const num query_result, const num run_query, char is_prep, vely_db_parse *vp)
{

    // query_id is the location in a stack-array of a query. First query is 0, the one inside it is 1, inside it 2 etc.
    // once the loop ends, goes back to 1, 0. It is -1 if no query is active.
    // It is calculated on the fly, when run-query is used, and just used for query-result and current-row or any other

    if (run_query == 1)
    {
        // create new query, query id
        prepare_query (gen_ctx, vp);

        // perform most of query code generation, loop generation, result processing
        // also update query_id to keep track of query loops
        query_execution (gen_ctx, run_query, query_result, is_prep, vp);
        return;

    }
    else if (query_result == 1)
    {
        // Now check for query-result
        if (query_id == -1)
        {
            _vely_report_error( "Query usage attempted, but no query present and active");
        }
        query_execution (gen_ctx, run_query, query_result, is_prep, vp);
        return;
    }
}


//
// Name a query. By default, a query is numbered (as in 0,1,2..), but if you give it a name, it must be a valid C name
// gen_ctx is a query context, vp is the context of a parsed run-query statement
//
void name_query (vely_gen_ctx *gen_ctx, vely_db_parse *vp)
{
    assert (vp);
    static vely_fifo *qnames = NULL; // fifo list of all names, cannot keep in context, since we
                                     // overwrite inactive queries with active ones
    if (qnames == NULL) vely_store_init(&qnames); // init list of names originally
    if (vp->name != NULL)
    {
        num n_len = strlen (vp->name);
        vely_trim (vp->name, &n_len);
        if (vely_is_valid_param_name (vp->name) != 1)
        {
            _vely_report_error(VV_NAME_INVALID, vp->name);
        }
        snprintf (gen_ctx->qry[query_id].name, sizeof (gen_ctx->qry[query_id].name), "_vv_qryname_%s", vp->name);
        vely_rewind(qnames); // rewind the list to search
        while (1)
        { // get query names one by one and check if duplicate
            char *one_name;
            vely_retrieve (qnames, NULL, (void**)&one_name);
            if (one_name==NULL) break;
            if (!strcmp (one_name, vp->name)) {
                _vely_report_error( "Query with the name [%s] already exists", vp->name);
            }
        } 
        vely_store(qnames, NULL, (void*)vely_strdup(vp->name)); // store name for future lookup
    }
    else snprintf (gen_ctx->qry[query_id].name, sizeof (gen_ctx->qry[query_id].name), "_vv_qryname_%lld", gen_ctx->total_queries);
}

//
// Examine proposed query statement, perform checks to its validity, and generate initial defining code for it.
// gen_ctx is execution context, vp has all clauses parsed out
//
void prepare_query (vely_gen_ctx *gen_ctx, vely_db_parse *vp)
{
    assert (vp->eq); // must be non NULL by parsing 

    // new_query either adds a query or uses existing one
    new_query (gen_ctx, vp);

    // get query output columns (or unknown)
    get_query_output (gen_ctx, vp); 

    //
    // Generate query variables etc
    //
    // generate variables used by query
    oprintf("num _vv_qry_on_error_%s = %d;\n", gen_ctx->qry[query_id].name, vp->on_error_action);
    oprintf("VV_UNUSED (_vv_qry_on_error_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("num _vv_qry_executed_%s = 0;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_qry_executed_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("char *%s = NULL;\n", gen_ctx->qry[query_id].name); 
    //
    // Generate the C code to allocate a query. gen_ctx is the contect, and query_id is the query id.
    // Depending on what kind of code we generate later, some of these may not be used, and we mark them
    // as unused to avoid compiler warning.
    //
    // affected rows, and then number of rows and columns retrieved
    // set to zero in case run-query doesn't run so that they don't use random values
    oprintf("num _vv_arow_%s = 0;\n", gen_ctx->qry[query_id].name);
    oprintf("num _vv_nrow_%s = 0;\n", gen_ctx->qry[query_id].name);
    oprintf("num _vv_ncol_%s = 0;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED(_vv_arow_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED(_vv_nrow_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED(_vv_ncol_%s);\n", gen_ctx->qry[query_id].name);
    // when we do 'run-query' with empty text followed by start-query, we don't
    // know if query is DML or not, so we have to specify all variables
    oprintf("char * _vv_errm_%s = VV_EMPTY_STRING;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_errm_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("char * _vv_err_%s = VV_EMPTY_STRING;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_err_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("char **_vv_data_%s = NULL;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_data_%s);\n", gen_ctx->qry[query_id].name);
    oprintf("char **_vv_col_names_%s = NULL;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_col_names_%s);\n", gen_ctx->qry[query_id].name);
    // allocate SQL buffer
    oprintf("static char *_vv_sql_buf_%s;\n", gen_ctx->qry[query_id].name);
    oprintf("char **_vv_sql_params_%s = NULL;\n", gen_ctx->qry[query_id].name);
    oprintf("num _vv_sql_paramcount_%s = 0;\n", gen_ctx->qry[query_id].name);
    // _vv_is_input_used specifies if there's an input parameter. 
    // There may be some zeroes in it, but normally it is a block of 1's corresponding to input parameters
    // see comments on vely_make_SQL() function for more on this.
    oprintf ("num _vv_is_input_used_%s[%d];\n", gen_ctx->qry[query_id].name, VV_MAX_QUERY_INPUTS + 1);
    oprintf ("memset (_vv_is_input_used_%s, 0, sizeof (num)*%d);\n", gen_ctx->qry[query_id].name, VV_MAX_QUERY_INPUTS + 1);
    //_dlen is length of data fields
    oprintf ("num *_vv_dlen_%s = NULL;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_dlen_%s);\n", gen_ctx->qry[query_id].name);
    // _iter is the iterator through output data loop (incrementing by 1 for each row)
    oprintf("num _vv_iter_%s;\n", gen_ctx->qry[query_id].name);
    oprintf("VV_UNUSED (_vv_iter_%s);\n", gen_ctx->qry[query_id].name);
    // set if prepared statement, if so, make the unique variable used for it
    // unique void * for prepared stmt, important to be NULL. It is only non-NULL when it has been prepared. 
    // It cannot be deallocated (when connection goes down) if we cannot distinguish whether it has been prepared or not
    oprintf("static void *_vv_sql_prep_%s = NULL;\nVV_UNUSED(_vv_sql_prep_%s);\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name);
}


//
// Free resources used by query
// gen_ctx is execution context, qryname is the name of the query
// skip_data is true if not to free data (but rather free everything else)
//
void free_query (char *qryname, bool skip_data)
{
    char qname[VV_MAX_QUERYNAME_LEN+1];
    snprintf (qname, sizeof (qname), "_vv_qryname_%s", qryname);
    // qname itself is just sql text
    // the error msg/cde are either VV_EMPTY_STRING or allocated, so they can be freed
    oprintf("vely_free(_vv_errm_%s);\n", qname);
    oprintf("vely_free(_vv_err_%s);\n", qname);
    // free data if there, also free each individual chunk
    oprintf("if (_vv_data_%s != NULL) { if (!%s) {num i; for (i=0;i<_vv_ncol_%s*_vv_nrow_%s;i++) vely_free(_vv_data_%s[i]);} vely_free(_vv_data_%s);}\n", qname, skip_data?"true":"false", qname, qname, qname, qname);
    // free names if there, also free each individual chunk
    oprintf("if (_vv_col_names_%s != NULL) {num i; for (i=0;i<_vv_ncol_%s;i++) vely_free(_vv_col_names_%s[i]); vely_free (_vv_col_names_%s);}\n", qname, qname, qname, qname);
    // free length of data if there
    oprintf ("if (_vv_dlen_%s != NULL) vely_free (_vv_dlen_%s);\n", qname, qname);
    // _vv_sql_buf_... is freed if it was allocated, it is a dynamic buffer for SQL text to be executed, search on it
    // _vv_sql_params_... is freed if it was allocated, it's dynamic params when statement is prepared
    // _vv_sql_prep is allocated once with strdup() and never freed unless re-prepared, just a unique pointer, search on it
}


//
// Do most of query code generation and processing of query result. Do all of query stack processing.
// query_result is 1 if this is query-result. run_query is 1 if this is run-query 
//
void query_execution (vely_gen_ctx *gen_ctx, const num run_query, const num query_result, char is_prep, vely_db_parse *vp)
{

    if (run_query == 1)
    {
        //
        // Check if there is : after which there must be input params
        //
        if (vp->colon != NULL) get_all_input_param (gen_ctx, vp->colon);


        // ready to generate query execution code in C
        generate_sql_code (gen_ctx, is_prep);

        //
        // get any information about query user requested
        // This must be done before opening the loop below (start_loop_*) b/c otherwise 
        // these would execute many times in a loop
        //
        if (vp->colnames != NULL)
        {
            if (gen_ctx->qry[query_id].returns_tuple == 0) _vely_report_error( "column-names cannot be used on query [%s] because it does not output any columns", gen_ctx->qry[query_id].name);
            if (gen_ctx->qry[query_id].unknown_output == 0) _vely_report_error( "column-names cannot be used on query [%s] because its columns are already defined (it can be used only with unknown-output)", gen_ctx->qry[query_id].name);
            oprintf("%s = _vv_col_names_%s;\n", vp->colnames, gen_ctx->qry[query_id].name);
        }
        if (vp->coldata != NULL)
        {
            if (gen_ctx->qry[query_id].returns_tuple == 0) _vely_report_error( "column-data cannot be used on query [%s] because it does not output any columns", gen_ctx->qry[query_id].name);
            oprintf("%s = _vv_data_%s;\n", vp->coldata, gen_ctx->qry[query_id].name);
        }
        if (vp->colcount != NULL)
        {
            if (gen_ctx->qry[query_id].returns_tuple == 0) _vely_report_error( "column-count cannot be used on query [%s] because it does not output any columns", gen_ctx->qry[query_id].name);
            oprintf("%s = _vv_ncol_%s;\n", vp->colcount, gen_ctx->qry[query_id].name);
        }
        if (vp->err != NULL)
        {
            oprintf("%s = _vv_err_%s;\n", vp->err, gen_ctx->qry[query_id].name);
        } 
        if (vp->errtext != NULL)
        {
            oprintf("%s = _vv_errm_%s;\n", vp->errtext, gen_ctx->qry[query_id].name);
        }
        if (vp->arows != NULL)
        {
            oprintf("%s = _vv_arow_%s;\n", vp->arows, gen_ctx->qry[query_id].name);
        }
        if (vp->rcount != NULL)
        {
            VV_CHECK_TUPLE(query_id);
            oprintf("%s = _vv_nrow_%s;\n", vp->rcount, gen_ctx->qry[query_id].name);
        }

        // start the loop (that ends with end-query) if requested
        if (vp->noloop == NULL) start_loop_query (gen_ctx); else end_query (gen_ctx, 0);

    }

    else if (query_result == 1)
    {
        // 
        // this is query-result statement
        //
        generate_query_result (gen_ctx, vp);
        return;
    }
    else
    {
       // should never happen
       _vely_report_error ("Syntax error in query");
    }
}

//
//  Processing of query-result.  vp->query_result is the name of column being asked to provide value for.
//
void generate_query_result (vely_gen_ctx *gen_ctx, vely_db_parse *vp)
{
    assert (vp->query_result); // must be present for query-result
    char *col_out = vp->query_result;

    // get column encoding when retrieving query column
    num no_encode = 0;
    num url_encode = 0;
    num web_encode = 0;
    char *noenc = find_keyword (col_out, VV_KEYNOENCODE, 1);
    if (noenc != NULL) no_encode = 1;
    char *webenc = find_keyword (col_out, VV_KEYWEBENCODE, 1);
    if (webenc != NULL) web_encode = 1;
    char *urlenc = find_keyword (col_out, VV_KEYURLENCODE, 1);
    if (urlenc != NULL) url_encode = 1;
    char *tovar = find_keyword (col_out, VV_KEYTO, 1);
    char *len = find_keyword (col_out, VV_KEYLENGTH, 1);


    carve_statement (&noenc, "query-result", VV_KEYNOENCODE, 0, 0);
    carve_statement (&webenc, "query-result", VV_KEYWEBENCODE, 0, 0);
    carve_statement (&urlenc, "query-result", VV_KEYURLENCODE, 0, 0);
    carve_statement (&len, "query-result", VV_KEYLENGTH, 0, 1);
    carve_statement (&tovar, "query-result", VV_KEYTO, 0, 1);
    define_statement (&len, VV_DEFNUM);
    define_statement (&tovar, VV_DEFSTRING);

    // we can change the mtext all we want, but we can't memmove, which is fine, that's why we use col_out here
    if (urlenc != NULL) *urlenc = 0;
    if (webenc != NULL) *webenc = 0;
    if (noenc != NULL) *noenc = 0;
    // remove any trailing spaces, so column comparison is always correct.
    num col_len = strlen (col_out);
    vely_trim (col_out, &col_len);
    if (col_len == 0)
    {
        _vely_report_error ("Column name cannot be empty");
    }

    if (no_encode+web_encode+url_encode > 1)
    {
        _vely_report_error ("Query output can be either noencode, webencode (default) or urlencode, but not any combination of these");
    }

    if (tovar != NULL)
    {
        // TO must be no encoding specified or "noencode" option if there is an encoding option
        // set it to no-encode if nothing specified
        if (no_encode+web_encode+url_encode == 1)
        {
            if (no_encode != 1) _vely_report_error ("Encoding cannot be used with 'to' keyword, the data is not encoded");
        } else no_encode = 1;
    }
    else 
    {
        // if no 'to' used, then if no encoding specified web encoding is the default
        if (no_encode+web_encode+url_encode == 0) web_encode = 1; // default webencode
    }


    // Check if query ID is active. If it is used, but not active, it means
    // we're using a query column outside the action block for that query
    if (gen_ctx->qry_active[query_id] != VV_QRY_ACTIVE)
    {
        _vely_report_error( "Query [%s] is not active", gen_ctx->qry[query_id].name);
    }

    //
    //
    // column_id is the id of the query column
    //
    //
    num column_id =  get_col_ID (gen_ctx, query_id, col_out);

    // we now have numerical positional ID for a column in the result set
    // which is column_id
    // for start-query we have made sure all start-query# queries have exact same columns
    // because the get_col_ID() call above will get the very last column definition in any case

    // we cannot have error, affected_rows here, because this is result set and it may be 0 rows,
    // so without rows, query-result is never reached, making it impossible to get the aforementioned fields.

    // if we added too many output columns, but query has not enough of them, the data will be undefined; prevent that
    oprintf ("if (_vv_ncol_%s <= %lld) vely_report_error (\"Column #%lld does not exist in the database result set\");\n", gen_ctx->qry[query_id].name,column_id, column_id);

    
    if (len != NULL) 
    {
        // this is to store lengths of columns query-result#a,b to define b length define l
        oprintf ("%s=_vv_dlen_%s[_vv_iter_%s*_vv_ncol_%s+%lld];\n", len, gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name, column_id);
    }


    // Print out actual column from db query, at runtime
    if (tovar == NULL)
    {
        oprintf("vely_puts (%s, _vv_data_%s[_vv_iter_%s*_vv_ncol_%s+%lld]);\n", web_encode == 1 ? "VV_WEB" : (url_encode == 1 ? "VV_URL":"VV_NOENC"), gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name, column_id);
    }
    else 
    {
        oprintf("%s = (_vv_data_%s[_vv_iter_%s*_vv_ncol_%s+%lld]);\n", tovar, gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, column_id);
    }
    //
    //
    // column_id is not used beyond this point. Very careful with it, because it can be negative!!!
    //
    //
}

//
// Generate SQL for SELECT and DMLs for start-query for query index query_id
//
void generate_sql_code (vely_gen_ctx *gen_ctx, char is_prep)
{

    oprintf("%s = %s;\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].text);

    oprintf("char *fname_loc_%s = \"%s\";\n",gen_ctx->qry[query_id].name, src_file_name); // to overcome constness
    oprintf("num lnum_%s = %lld;\n",gen_ctx->qry[query_id].name,lnum); 
    oprintf("vely_location (&fname_loc_%s, &lnum_%s, 1);\n",gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name);


    statement_SQL (gen_ctx, is_prep);

    if (gen_ctx->qry[query_id].returns_tuple == 1)
    {
        // generate select call for any SQL that returns tuple
        oprintf("vely_select_table (_vv_sql_buf_%s, &_vv_arow_%s, &_vv_nrow_%s, &_vv_ncol_%s, %s%s, &_vv_data_%s, &_vv_dlen_%s, (char**)&_vv_err_%s, (char**)&_vv_errm_%s, %d, &_vv_sql_prep_%s, _vv_sql_paramcount_%s, _vv_sql_params_%s, _vv_qry_on_error_%s);\n",
        gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name, 
        // get query columns names from db only if unknown output is used, otherwise not
        gen_ctx->qry[query_id].unknown_output == 1 ? "&_vv_col_names_": "NULL", 
        gen_ctx->qry[query_id].unknown_output == 1 ? gen_ctx->qry[query_id].name:"",
        gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, is_prep, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name);
        oprintf("vely_db_free_result (%d);\n", (int)is_prep);


    }
    else
    {
        // this is no-tuple call

        oprintf("vely_execute_SQL (_vv_sql_buf_%s, &_vv_arow_%s, (char**)&_vv_err_%s, (char**)&_vv_errm_%s, %lld, 1, %d, &_vv_sql_prep_%s, _vv_sql_paramcount_%s, _vv_sql_params_%s, _vv_qry_on_error_%s);\n",
            gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].returns_tuple, (int)is_prep, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name);
        // set nrow to 0
        oprintf("_vv_nrow_%s=0;\n", gen_ctx->qry[query_id].name);
        oprintf("vely_db_free_result (%d);\n", (int)is_prep);
    }

    // deallocate _vv_sql_buf_<query name> if not prepared and no input params (which is the condition for it to be allocated)
    if (gen_ctx->qry[query_id].qry_found_total_inputs > 0 && is_prep == 0) oprintf("vely_free (_vv_sql_buf_%s);\n", gen_ctx->qry[query_id].name);
    // deallocate SQL params used for prepared query
    oprintf("if (_vv_sql_params_%s != NULL) vely_free (_vv_sql_params_%s);\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name);
}

//
// open the loop query in generated code. query_id is the index number of query.
//
void start_loop_query (vely_gen_ctx *gen_ctx)
{

    oprintf("for (_vv_iter_%s = 0; _vv_iter_%s < _vv_nrow_%s; _vv_iter_%s++)\n",gen_ctx->qry[query_id].name, 
        gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name,gen_ctx->qry[query_id].name);
    oprintf("{\n");
    
    // if outputs defined with "output define col1, col2..." then define them and fill values
    if (gen_ctx->qry[query_id].qry_out_defined)
    {
        num i;
        for (i = 0; i < gen_ctx->qry[query_id].qry_total_outputs; i++) 
        {
            char *collist = gen_ctx->qry[query_id].qry_outputs[i];
            // define variable if asked, this is query-result <colname> to define <colname>
            // hence the sizing of qr, 50 is for "to define" or anything else
            char qr[2*VV_MAX_COLNAME_LEN+50];
            if (vely_is_valid_param_name(collist) != 1)
            {
                _vely_report_error(VV_NAME_INVALID, collist);
            }
            snprintf (qr, sizeof(qr), "%s to define %s", collist, collist);
            query_result (gen_ctx, qr);
        }
    }


    // done with loop query
}



//
// Statement SQL to be executed for run-query etc. This is after the analysis of the statement,
// so we have filled out structures based on query_id. The statement SQL is output to generated source file
//
void statement_SQL (vely_gen_ctx *gen_ctx, char is_prep)
{
    if (is_prep == 1)
    {
        // 
        // statement is evaluated only the first time for prepared queries, so we save a copy
        // of the query in case we need to reconnect. TODO: if query is a string literal, we can
        // avoid strdup below.
        //
        oprintf("static char _vv_sql_buf_set_%s=0;\n", gen_ctx->qry[query_id].name);
        //
        // _vv_sql_buf_<query name> is a copy of query text. It should stay and not be freed for the life of the process,
        // because if the db server goes down, it will be needed to prepare statement again.
        // So no vely_ alloc functions, as they are deallocated at the end of the request
        // Also set void** as prepared statement pointer, so it can be cleared when connection is lost (vely_db_prep() call)
        oprintf("if (_vv_sql_buf_set_%s==0) {if ((_vv_sql_buf_%s=strdup(%s))==NULL) vely_report_error(\"Cannot allocate query [%s] for a prepared statement\");_vv_sql_buf_set_%s=1; vely_db_prep(&_vv_sql_prep_%s); }\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name);
        oprintf("if (_vv_qry_executed_%s == 1) {vely_report_error(\"Query [%s] has executed the second time; if you want to execute the same query twice in a row without a run-query, use different queries with the same query text (or variable) if that is your intention, file [%s], line number [%lld] \");}\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, src_file_name, lnum);
    }

    // with dynamic queries, we cannot count how many '%s' in SQL text (i.e. inputs) there are. 
    // For dynamic, the number of inputs is known  only by
    // the number of actual input parameters in start-query (this is qry_found_total_inputs). Because
    // SQL statement (_vv_sql_buf...) is only known at run-time, we need to pass this number of input params to it
    // and verify we are not using bad memory or missing arguments.
    if (gen_ctx->qry[query_id].qry_found_total_inputs == 0)
    {
        //
        // If there are no parameters, this would look like:
        // vely_make_SQL (_vv_sql_buf_something, 32000, 0, something);
        // and that would cause gcc error 'format not a string literal and no format arguments [-Werror=format_security]
        // because gcc doesn't know that generated code will NOT make a mistake of including %s in 'something'
        // So make it a simple copy, as it's all that's needed.
        // is_prep can be 0 or 1 here, if 1, this is prepared statement without any input params
        //
        // for non-prepared statements without any input params, just use the query text
        if (is_prep == 0) oprintf("_vv_sql_buf_%s=%s;\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name);
    }
    else
    {
        if (is_prep == 0)
        {
            // _vv_sql_buf_%s may be NULL here, in general it is undefined. And it will be allocated in vely_make_SQL at run time
            // and it will be deallocated once the query executes.
            oprintf("vely_make_SQL (&_vv_sql_buf_%s, %lld, %s ",
                gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].qry_found_total_inputs,  gen_ctx->qry[query_id].name); 
            num z;
            for (z = 0; z < gen_ctx->qry[query_id].qry_found_total_inputs; z++)
            {
                oprintf(", _vv_is_input_used_%s[%lld]==1 ?  (%s) : NULL ", gen_ctx->qry[query_id].name, z, gen_ctx->qry[query_id].qry_inputs[z]);
            }
            oprintf(");\n");
        }
        else
        {
            // 
            // set params count and params themselves for prepared statement
            //
            oprintf("_vv_sql_paramcount_%s = %lld;\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].qry_found_total_inputs);
            oprintf("_vv_sql_params_%s = (char**)vely_calloc (%lld, sizeof(char*));\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].qry_found_total_inputs);
            num z;
            for (z = 0; z < gen_ctx->qry[query_id].qry_found_total_inputs; z++)
            {
                oprintf("_vv_sql_params_%s[%lld]=(_vv_is_input_used_%s[%lld]==1 ?  (%s) : NULL);\n", gen_ctx->qry[query_id].name, z, gen_ctx->qry[query_id].name, z, gen_ctx->qry[query_id].qry_inputs[z]);
            }
        }
    }

    // We execute the actual db query right at the beginning of the action block
    // We use 'query ID' decorated variables, so all our results are separate
    // file/line must be here because this is generated code and file name/line number are NOT available then unless 
    // we plug it in now
    oprintf("if (_vv_qry_executed_%s == 1) {vely_report_error(\"Query [%s] has executed the second time; if you want to execute the same query twice in a row without a run-query, use different queries with the same query text (or variable) if that is your intention, file [%s], line number [%lld] \");}\n", gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].name, src_file_name, lnum);
    oprintf("_vv_qry_executed_%s = 1;\n", gen_ctx->qry[query_id].name);
}


// 
// Process text of dynamic query. gen_ctx is execution context, vp is parsed statement clauses.
//
void get_query_output (vely_gen_ctx *gen_ctx,  vely_db_parse *vp)
{
    char *with_output = vp->with_output;
    char *with_unknown_output = vp->with_unknown_output;

    if (with_output != NULL && with_unknown_output != NULL)
    {
        _vely_report_error( "Cannot have both output and unknown-output clause");
    }

    if (with_output != NULL)
    {
        // first check if there's define
        size_t pref = strspn (with_output, " ");
        char *defcomp = "define ";
        int defcomp_l = strlen (defcomp);
        if (!strncmp (with_output + pref, defcomp, defcomp_l)) 
        {
            gen_ctx->qry[query_id].qry_out_defined = true; // define outputs as strings
            // get passed define to extract column names
            with_output += defcomp_l;
        }
        // get all the column names
        while (1)
        {
            num end_of_list = 0;
            char *collist = with_output;
            // now finding column list after output
            char *colend = collist;
            get_until_comma (&colend);
            if (*colend == 0) end_of_list = 1; // no more in comma-separated list, last item
            *colend = 0;
            num colLen = strlen (collist);
            vely_trim (collist, &colLen);
            gen_ctx->qry[query_id].qry_outputs[gen_ctx->qry[query_id].qry_total_outputs] = vely_strdup (collist);
            gen_ctx->qry[query_id].qry_total_outputs++;
            if (gen_ctx->qry[query_id].qry_total_outputs >= VV_MAX_QUERY_OUTPUTS)
            {
                _vely_report_error( "Too many query outputs [%lld]", gen_ctx->qry[query_id].qry_total_outputs);
            }
            with_output = colend + 1;
            if (end_of_list == 1) break;
        }
    }
    else if (with_unknown_output != NULL)
    {
        // 
        // This is SELECT with unknown outputs, such as 'SELECT * from '... - the output isn't really
        // unknown but we don't want to get data using column names, but rather column-data in run-query statement
        // So qry_total_outputs remains ZERO in this case.
        // Check there is nothing beyond unknown-output:
        //
        //
        // We set qry_total_output to 1 even though we don't know the number of columns at run-time. 
        // Any query would have at least one column so we can
        // safely call this one "first_column", which is rather arbitrary. The purpose of "unknown-output" is to NEVER get columns by
        // name but rather to use column-names, column-data, column-count in run-query
        //
        gen_ctx->qry[query_id].qry_outputs[0] = "first_column";
        gen_ctx->qry[query_id].qry_total_outputs = 1;
        gen_ctx->qry[query_id].unknown_output = 1;
    }
    else
    {
        // no output, this is not a SELECT, such a dynamic query is DML
        gen_ctx->qry[query_id].returns_tuple = 0;
    }
}


//
// Setup database config from db name, which is dbname
//
void get_db_config (char *dbname)
{
    // if db name not specified, that's valid if there is only one db in use
    if (dbname == NULL)
    {
        if (totconn == 1) dbname = vely_get_config()->ctx.db->conn[0].db_name;
        else if (totconn == 0) _vely_report_error ("No database is configured. Configure a database before attempting to use it");
        else _vely_report_error ("There is more than one database configured. Use '@' to specify the database used here");
    }

    num l = strlen (dbname);
    vely_trim (dbname, &l);

    // dbname is now pure db conn name
    if (vely_is_valid_param_name(dbname) != 1)
    {
        _vely_report_error( "Database name must have only alphanumeric characters and underscore");
    }

    // generate database switch to this database
    // no need for runtime to have dbtype info, each library already knows what it is
    oprintf ("vely_get_config()->ctx.db->ind_current_db=%lld;\n", find_connection (dbname));

}

//
// Build a list of connections, and return a fixed index that identifies the
// database connection we're adding. 'conn' is the connection name. It is assume no duplicates arrive here - this is
// fed from .dbvendors file generated by vely utility.
// dbtype is db vendor (such as VV_DB_MARIADB)
// There is no limit on number of db connections
//
num add_connection(char *conn, num dbtype)
{
    num i;
    static num conn_num = 0;
    if (vely_get_config()->ctx.db->conn == NULL)
    {
        conn_num = 1; // start with 1 db connection since that's what most programs will do.
        // calloc initializes to zero here - we rely on that to make sure members like g_con are NULL, etc.
        vely_get_config()->ctx.db->conn = vely_calloc (conn_num, sizeof(one_db));
    }
    for (i = 0; i < totconn; i++)
    {
        if (!strcmp (conn, vely_get_config()->ctx.db->conn[i].db_name))
        {
            _vely_report_error ("Database connection [%s] duplicated in the list of connections", conn);
            return i;
        }
    }
    totconn++;
    if (totconn > conn_num)
    {
        num old_conn_num = conn_num;
        conn_num+=3; // add three connections at a time
        vely_get_config()->ctx.db->conn = vely_realloc (vely_get_config()->ctx.db->conn, conn_num*sizeof(one_db));
        // set to zero so g_con is NULL etc.
        num j;
        for (j = old_conn_num; j < conn_num; j++) 
        {
            memset (&(vely_get_config()->ctx.db->conn[j]), 0, sizeof(one_db));
        }
    }
    vely_get_config()->ctx.db->conn[totconn-1].db_name=vely_strdup (conn);
    vely_get_config()->ctx.db->conn[totconn-1].db_type=dbtype; // the default (empty) is mariadb
    return totconn - 1;
}

//
// Build a list of connections, and return a fixed number between 0 and VV_MAX_DB_CONNECTIONS that identifies the
// database connection we're dealing with. 'conn' is the connection name.
//
num find_connection(char *conn)
{
    num i;
    // make sure no spaces around the db name, there usually are from parsing
    char *db = vely_strdup (conn);
    num dlen = strlen (db);
    vely_trim (db, &dlen);

    for (i = 0; i < totconn; i++)
    {
        if (!strcmp (db, vely_get_config()->ctx.db->conn[i].db_name))
        {
            return i;
        }
    }
    _vely_report_error ("Database connection [%s] not found. Every database connection must have a configuration file in [%s] directory", db, vely_dbconf_dir);
    return -1;
}

//
// See vely_find_keyword0
//
char *find_keyword(char *str, char *find, num has_spaces)
{
    return  vely_find_keyword0 (str, find, has_spaces, 1);
}


//
// Get value of option in statement. For example, if statement is 
// send-mail from "x@y.com" to "z@w.com" ...
// then you can get "x@y.com" by doing this:
// 
// char *from = find_keyword (...);
// carve_statement (&from, "send-mail", VV_KEYFROM, 1, 1);
//
// where variable 'from' will be "x@y.com", '1' as 'is_mandatory' parameter means this parameter MUST be present,
// indicate where we're at in the parsing process (file name and line number).
// 'send-mail' is the name of top statement, and VV_KEYFROM is "from", and we're parsing out the data after it.
// NOTE that 'from' MUST point to actual "x@y.com" within original send-mail string. This means ALL options must be first
// found with find_keyword() before calling this function for any of them.
// This function MUST be called for ALL statement options - if not then some statement values will contain other statement and will be 
// INCORRECT. 
// 'has_data' is 0 if the option is alone without data, for example 'no-cert'. Typically it's 1. It can also be 2, which is
// when there may be data after it, but it may also be missing.
//
void carve_statement (char **statement, char *statement_name, char *keyword, num is_mandatory, num has_data)
{
    //
    // *statement is the result of strstr(mtext, keyword) - and these MUST
    // be done for ALL options prior to calling this function
    //
    if (*statement != NULL) 
    {
        int ksize = strlen(keyword);
        // keyword that is not a 'human word' may have no space at the end, such as =, ==, != (hence isalpha() check)
        // also keyword may be " ", which is the very first space after the statement name, like in get-cookie
        // but keyword[ksize-1] being ' ' is when there's supposed to be data afterwards
        if (has_data > 0 && keyword[ksize-1]!=' ' && isalpha(keyword[0]))
        {
            // any keyword that has data must have trailing space
            // but if has_data is 2, data may be missing after keyword (see above)
            if (has_data != 2) _vely_report_error( "Internal: keyword [%s] is missing trailing space", keyword);
        }
        char *end_of_url = *statement;
        // advance past the keyword
        *(*statement = (*statement + ksize-1)) = 0;
        (*statement)++;
        if (has_data == 2)
        {
            // advance passed any white spaces, this is important when has_data is 2, so we know if there is something there or not
            // if what follows is a keyword, it will be cut off by the processing of that keyword and still be empty
            while (isspace(**statement)) (*statement)++;
        }
        *end_of_url = 0; // cut off keyword, so previous clause ends at the beginning of the next one

        if (has_data==0)
        {
            (*statement)[0] = 0; // no data for this option, we only care if present or not present.
        }

    }
    else if (is_mandatory==1)
    {
        _vely_report_error( "%s keyword is missing in %s", keyword, statement_name);
    }
}

//
// Call this if statement can have 'define', meaning it is a string that is the result of some operation
// This will generate code to define such string.
// 'statement' is the pointer to the pointer to data associated with statement name, for example in 'get-time to define res year 1'
// 'statement' could point to 'define res' and define it as a string
// 'type is VV_DEFSTRING('char *') or VV_DEFNUM or VV_DEFVOIDPTRPTR or VV_DEFCHARPTRPTR
// or VV_BROKEN or VV_DEFDBL or VV_DEFHASH or VV_DEFJSON or VV_DEFFIFO or VV_DEFVOID, or VV_DEFENCRYPT
// or VV_DEFFILE or VV_DEFFCGI
// Returns 1 if there is a "define", 0 otherwise.
//
// if the '*statement' has () around it, those are removed (see comments below)
//
num define_statement (char **statement, num type)
{
    if (*statement == NULL) return 0;
    //
    // This is if option can be of form 'define some_var'
    //
    num is_def_result = 0;
    is_opt_defined (statement, &is_def_result);

    // if the result has () around it, remove it. There's no need for it, other than to use the same name
    // for a variable or function as is a keyword. If really needed, use (()), so the inner ones remain
    int ls = strlen (*statement);
    if (ls >= 3 && (*statement)[0] == '(' && (*statement)[ls-1] == ')')
    {
        // remove leading ( and trailing )
        (*statement)[ls - 1] = 0;
        (*statement)++;
    }

    // make sure *statement is not empty - this happens when data to a clause is not supplied
    num lst = strlen (*statement);
    *statement = vely_trim_ptr(*statement,  &lst);
    if ((*statement)[0] == 0) 
    {
        _vely_report_error("Parameter missing in the statement");
    }


    if (is_def_result == 1 && vely_is_valid_param_name(*statement) != 1)
    {
        _vely_report_error(VV_NAME_INVALID, *statement);
    }
    if (is_def_result == 1)
    {
        if (type == VV_DEFSTRING) oprintf ("VV_DEFINE_STRING(%s);\n", *statement);
        else if (type == VV_DEFVOIDPTR) oprintf ("void *%s = NULL;\n", *statement);
        else if (type == VV_DEFVOIDPTRPTR) oprintf ("void **%s = NULL;\n", *statement);
        else if (type == VV_DEFCHARPTRPTR) oprintf ("char **%s = NULL;\n", *statement);
        else if (type == VV_DEFNUM) oprintf ("num %s = 0;\n", *statement);
        else if (type == VV_DEFDBL) oprintf ("dbl %s = 0;\n", *statement);
        else if (type == VV_DEFBROKEN) oprintf ("vely_split_str *%s = NULL;\n", *statement);
        else if (type == VV_DEFJSON) oprintf ("vely_json *%s = NULL;\n", *statement);
        else if (type == VV_DEFHASH) oprintf ("vely_hash *%s = NULL;\n", *statement);
        else if (type == VV_DEFFIFO) oprintf ("vely_fifo *%s = NULL;\n", *statement);
        else if (type == VV_DEFENCRYPT) oprintf ("EVP_CIPHER_CTX *%s = NULL;\n", *statement);
        else if (type == VV_DEFFILE) oprintf ("vely_file *%s = NULL;\n", *statement);
        else if (type == VV_DEFFCGI) oprintf ("vv_fc *%s = NULL;\n", *statement);
        else _vely_report_error( "Unknown define type (%lld)", type);
    }
    return is_def_result;
} 


//
// For a given option text (without the option keyword), find out if it has 'define' keyword
// 'option' is the option text (such as 'define xyz' that was part of for example 'program-output define xyz')
// and on output it would be just 'xyz' and is_defined would be 1. If it were just 'xyz', it would be still 
// 'xyz' and is_defined would be 0.
//
// We do not make a copy via vely_strdup of the option because option is capped to be a string on its own, 
// i.e when we manipulate the option string here, we won't be moving the memory for the entire line, passed
// the option itself.
//
void is_opt_defined (char **option, num *is_defined)
{
    num def_len = strlen (VV_KEYDEFINED);
    num l;
    *is_defined=0;

    // trim it so we can reliably check if it starts with 'define'
    l = strlen (*option);
    vely_trim (*option, &l);

    if (!strncmp (*option, VV_KEYDEFINED, def_len))
    {
        // if it starts with 'defined', get passed it
        *option = *option + strlen (VV_KEYDEFINED);
        *is_defined = 1; // set output flag to indicate it has 'define'
        // trim the rest of it
        l = strlen (*option);
        vely_trim (*option, &l);
    }

    if (option[0]==0)
    {
        _vely_report_error ("Markup option is empty");
    }
}

// 
// Display verbosely steps taken during the preprocessing
// vely_line is the line number is source, plus the printf-like output
//
void out_verbose(num vely_line, char *format, ...)
{
    if (verbose==0) return;
    // THIS FUNCTON MUST NEVER USE ANY FORM OF MALLOC OR VV_MALLOC
    // or it may fail when memory is out or not available (such as in vely_malloc)

    char trc[VV_TRACE_LEN + 1];

    va_list args;
    va_start (args, format);
    vsnprintf (trc, sizeof(trc) - 1, format, args);
    va_end (args);

    fprintf (stdout, "Line %lld: %s\n", 
        vely_line, trc);
    fflush (stdout);
}


// 
// Find terminal width for help display.
// Returns width in characters.
//
num terminal_width()
{
    static num twidth = 0;
    if (twidth == 0)
    {
        struct winsize w;
        ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);

        twidth = (num)(w.ws_col);
    }
    return twidth;
}

//
// Parse a list of parameters. Parameters are a list of strings or non-string parameters separated
// by commas. Parameter can have a comma if it is a string, or within ()
//
// String representing list of parameters is parse_list. VELY list structure is 'params', which will
// hold the parsed parameters. 
// parse_list is advanced to its end during processing.
// tot is the number of elements (single or x=y pairs encountered); it can be NULL.
//
// This is for a LIST of parameters separated by comma ONLY (i.e. start-query# and exec-program).
// On return, parse_list is one byte passed the end of last character in the list, which is null character.
//
void parse_param_list (char *parse_list, vely_fifo **params, num *tot)
{
    vely_store_init(params);
    if (tot != NULL) *tot = 0;

    while (1)
    {
        // input param can be anything separate by comma, which must be outside quotes
        // or parenthesis, i.e. free-comma.
        // so find free comma, or if none found returns pointer
        // to the ending null char of *parse_list string.
        char *milestone = find_keyword(parse_list, ",", 0);
        char *end_of_parse_list; // this is where parse_list should be after looking for parameter
        char _vv_inp_par[VV_MAX_QUERY_INPUT_LEN + 1];
        char *inp_par=_vv_inp_par; // because we will ++ this pointer, can't work otherwise

        if (milestone == NULL)
        {
            milestone = parse_list + strlen (parse_list); // points to 0 of its end, so grasping param works below w/par_len
            // we reached the end of string, this is the last parameter
            end_of_parse_list = milestone;
        }
        else
        {
            end_of_parse_list = milestone+1;// get one beyond comma
            // skip white space after comma
            while (isspace(*end_of_parse_list)) end_of_parse_list++;
        }

        // milestone now points to either the next parameter or is NULL (meaning end of param list)

        // length of input parameter
        num par_len = milestone - (parse_list);

        if (par_len > VV_MAX_QUERY_INPUT_LEN - 1)
        {
            _vely_report_error( "Parameter too long [%lld], parameter [%.100s]", par_len, parse_list);
        }

        memcpy (inp_par, parse_list, par_len);
        inp_par[par_len] = 0;

        vely_trim(inp_par, &par_len);
        vely_store(*params, NULL, (void*)vely_strdup(inp_par));

        parse_list = end_of_parse_list; // positioned to next parameter
                                         // this needs to be before break below
                                         // because caller relies on parse_list being exhausted
        if (tot != NULL) (*tot)++;

        // break if at the end
        if (*end_of_parse_list==0) break;

    } 
    vely_rewind(*params);
}

//
// Find next input parameter in SQL text. gen_ctx is the context of parsing, and 
// This parses part of statement such as start-query#xyz: a,b,c... i.e. it finds a, b, c etc. In this case query text uses '%s' 
// The parameter is a C expression or a quoted string (technically also a C expression, but treated differently).
// A parameter must be either a string, or not have comma, or be within ()
// So for example a parameter like func_call("x",2) would be (func_call("x",2))
//
// gen_ctx is the context. query_id is index into gen_ctx->qry[] array. iparams is the list of parameters to be parsed
//
void get_all_input_param (vely_gen_ctx *gen_ctx, char *iparams)
{

    // parse params. 
    vely_fifo *params = NULL;
    parse_param_list (iparams, &params, NULL);


    char *value;
    while (1)
    {
        vely_retrieve (params, NULL, (void**)&value);
        if (value==NULL) break;

        add_input_param (gen_ctx, value);
    } 
}

// 
// Add input parameter to a query.
// gen_ctx is the context. query_id is the index into gen_ctx->qry[] (i.e. queries). inp_par is the string
// representing input parameter 
//
void add_input_param (vely_gen_ctx *gen_ctx, char *inp_par)
{
    VV_TRACE("");

    vely_strncpy (gen_ctx->qry[query_id].qry_inputs[gen_ctx->qry[query_id].qry_found_total_inputs], inp_par, 
       VV_MAX_QUERY_INPUT_LEN - 1);

    oprintf("_vv_is_input_used_%s[%lld]=1;\n",  gen_ctx->qry[query_id].name, gen_ctx->qry[query_id].qry_found_total_inputs);
    gen_ctx->qry[query_id].qry_found_total_inputs++;

}

//
// Process end-query statement. Update relevant data structures such as query stack.
// gen_ctx is preprocessor context, close_block is 1 if this is run-query (where we generate for loop)
// and 0 for run-query  with no-loop
// Output: query_id will be set to either -1 if there is no active query once we end this query, or to 
// that query's id (for example in case of nested queries).
// Error out if there is no active query at the point in source code.
//
void end_query (vely_gen_ctx *gen_ctx, num close_block)
{
    // end of action block found , but there was not beginning
    if (query_id == -1)
    {
        _vely_report_error( "query ending found, but no matching beginning");
    }
    // Ending of query with ID of leaving ID
    num leaving_id = gen_ctx->curr_qry_ptr;
    assert (leaving_id != -1);

    // no longer active, and its variables are defined, in case we
    // need to use them again - but they won't be defined again 
    gen_ctx->qry_active[leaving_id] = VV_QRY_INACTIVE;

    // reset number of inputs found in <<qry XX, in case we have another 
    // same query later on, for which we need to calculate this 'actual'
    // number of found input URL params
    // This can be with start-query, where multiple start-queries can have
    // different kinds of inputs
    gen_ctx->qry[leaving_id].qry_found_total_inputs = 0;

    // go down one level
    gen_ctx->curr_qry_ptr --;
    assert (gen_ctx->curr_qry_ptr >= 0);

    if (close_block == 1)
    {
        oprintf("}\n"); // end of FOR loop we started with the QRY statement beginning
    }

    open_queries--;
}


// 
// Output generated C code. We remove empty lines (of which is generally plenty in order not to clutter
// HTML output, and to consistently output only non-empty lines. Works like printf.
// Error out if generated source code line too long.
// If format is NULL, flush the output.
//
void oprintf (char *format, ...)
{
    va_list args;
    va_start (args, format);

    static char *oline = NULL;
    static num oline_len = 0; // length of buffer
    static num oline_size = 0; // size in bytes written 
    static num oline_prev_line = 0; // has previous line ended?

    if (format == NULL)
    {
        if (oline != NULL)
        {
            // remove empty printouts
            num len = strlen (oline);
            vely_replace_string (oline, len+1, "vely_puts (VV_NOENC, \"\");\n", "", 1, NULL, 1); // remove idempotent printouts
                                                    // will always succeed because output is shorter 
            vely_replace_string (oline, len+1, "vely_puts (VV_NOENC, \"\");", "", 1, NULL, 1); // replace with or without new line

            // The output goes to outf, i.e. the global file description tied to generated C code,
            // OR it goes to stdout, which would be if this is a command line program
            if (outf != NULL)
            {
                fprintf (outf, "%s", oline);
                va_end (args);
            }
            else
            {
                printf ("%s", oline);
                va_end (args);
            }
            //start from the beginning since just flushed
            vely_free (oline);
            oline_len = 0;
            oline_size = 0;
            oline_prev_line = 0;
            return;
        }
        else
        {
            // nothing to flush
            va_end (args);
            return;
        }
    }

    if (oline == NULL) oline = vely_malloc (oline_size += 2*VV_MAX_CODE_LINE + 1);


    // add line number so all the code generated is the same line number
    // but only if the previous line had a new line (meaning this is the beginning of the new line of source code)
    // for example a single code line may be output by several oprintfs (this is oline_prev_line)
    if (oline_prev_line == 1 && VV_EMIT_LINE)
    {
        // make reported line as line*10000+generated_line, so each generated line is tied to original
        // line (final_line/10000), but it also has information about which generated line the diagnostic is about
        // this is used in error reporting
        static num old_lnum = -1;
        static num in_lnum = 0;
        if (old_lnum != lnum) { old_lnum = lnum; in_lnum = 0; } else in_lnum++;
        num tot_written = snprintf (oline + oline_len, oline_size - oline_len - 1, "#line %lld \"%s\"\n", vv_plain_diag == 1 ? lnum : lnum*10000+in_lnum, src_file_name);
        if (tot_written >= oline_size - 1)
        {
            _vely_report_error ("Source code line too long, exiting");
            exit (1);
        }
        oline_len += tot_written;
        oline_prev_line = 0;
    }

    // add the actual code generated
    num tot_written = vsnprintf (oline + oline_len, oline_size - oline_len - 1, format, args);
    if (tot_written >= oline_size - 1)
    {
        _vely_report_error ("Source code line too long, exiting");
        exit (1);
    }
    if (strchr (oline + oline_len, '\n') != NULL) oline_prev_line = 1;
    oline_len += tot_written;

    // expand if getting long, meaning what's left is less than VV_MAX_CODE_LINE(4k)
    if (oline_size-oline_len <= VV_MAX_CODE_LINE)
    {
        oline = vely_realloc (oline, oline_size += 2*VV_MAX_CODE_LINE + 1);
    }
    va_end (args);
}

#define VV_COLOR_BLUE "\033[34m"
#define VV_COLOR_NORMAL "\033[0;39m"
#define VV_COLOR_BOLD "\033[0;1m"
// 
// Output error to stderr. The error means error during the preprocessing with VELY.
// There's a maximum length for it, and if it's more than that, ignore the rest.
// If there is no dot at the end, put it there.
// After that, exit program.
//
void _vely_report_error (char *format, ...)
{
    char errtext[VV_MAX_ERR_LEN + 1];

    oprintf (NULL); //  flush output

    va_list args;
    va_start (args, format);
    vsnprintf (errtext, sizeof(errtext) - 1, format, args);
    va_end (args);
    fprintf (stderr, "%s%s%s", VV_COLOR_BOLD, VV_COLOR_BLUE, errtext);
    if (src_file_name != NULL)
    {
        fprintf (stderr, ", reading file [%s], line [%lld]", src_file_name, lnum);
    }
    if (errtext[0] != 0 && errtext[strlen (errtext) - 1] != '.')  fprintf(stderr, ".");
    fprintf (stderr, "%s\n",VV_COLOR_NORMAL);
    exit (1);
}

// 
// Get column ID for a query and output column name. gen_ctx is preprocessor context, 
// qry_name is the ID of a query
// (in the array of queries), column_out is the name of the column for which we seek the ID. The ID of a column
// is the array index in the list of output columns that matches the data array (the result set).
// Returns the ID of a column. For virtual columns (those that start with $), returns negative values that match them.
//
num get_col_ID (vely_gen_ctx *gen_ctx, num qry_name, char *column_out)
{
    assert (gen_ctx);
    assert (column_out);


    char all_columns[VV_TOT_COLNAMES_LEN + 1];
    // get column ID from column name
    num i;
    num j = 0;
    num found_col = -1;
    for (i = 0; i < gen_ctx->qry[qry_name].qry_total_outputs; i++)
    {
        j+=snprintf (all_columns + j, sizeof (all_columns)-j-1, "%s,", gen_ctx->qry[qry_name].qry_outputs[i]);
        if (!strcmp (column_out, gen_ctx->qry[qry_name].qry_outputs[i]))
        {
            //
            // Make sure column is not duplicated in the query output, leading to hard-to-find programming errors
            //
            if (found_col != -1) 
            {
                _vely_report_error( "Column [%s] is present more than once in the set of columns for query[%s], please make sure every column in query output has a unique name. List of columns separated by comma is [%s]", column_out, gen_ctx->qry[qry_name].text, all_columns);
            }
            found_col = i;
        }
    }
    if (found_col != -1) return found_col;
    if (j>0) all_columns[j-1] =0;
    _vely_report_error( "Column [%s] is not a part of the set of columns for query[%s], list of columns separated by comma is [%s]", column_out, gen_ctx->qry[qry_name].text, all_columns);

    return 0; // will never be reached, column not found
}


// 
// Add new query. gen_ctx is the context, vp is the query parsing context.
// Error messages are emitted if there are syntax errors.
//
void new_query (vely_gen_ctx *gen_ctx, vely_db_parse *vp)
{
    assert (gen_ctx);
    assert (vp);
    char *orig_qry = vp->eq; // orig_qry is the SQL text of query

    //
    // This creates new query, nested if needed, gen_ctx->curr_qry_ptr is the depth of it and 
    // gen_ctx->curr_qry_ptr-1 is also query_id
    //
    // this increases the number of open queries
    check_next_query
    // move the query stack pointer one up. At this location in the stack, there is
    // nothing, i.e. this pointer always points to the next (as of yet non-existent)
    // query ID
    gen_ctx->curr_qry_ptr ++;
    // check if we're nesting too much
    if (gen_ctx->curr_qry_ptr >= VV_MAX_QUERY_NESTED) _vely_report_error( VV_MSG_NESTED_QRY, query_id, VV_MAX_QUERY_NESTED);
    // now this query ID is active. We use it to prohibit nesting queries with the same ID
    gen_ctx->qry_active[query_id] = VV_QRY_ACTIVE;
    // this is start-query, so the query has now officially been used
    gen_ctx->qry_used[query_id] = VV_QRY_USED;
    //
    // End new query creation
    //

    char *qry=vely_strdup(orig_qry);
    num ql = strlen(qry);
    // 
    // We do not touch % signs, because we do NOT use snprintf like function at run time in vely_make_SQL
    // '%s' will be replaced with input parameters, others won't be touched
    //
    // Sanity check and error
    if (qry[0] != 0 && qry[ql - 1] == ';')
    {
        _vely_report_error( "Query cannot end with a semicolon");
    }
    // actually add final query into the array of queries, curr_query is query ID
    // 
    // Initialize query once the text of query has been processed 
    // Note that query already exists - it is under [query_id] in array of queries
    // Set status of query (UNUSED), initialize number of input parameters, set returns_tuple flag, increase total number of
    // queries (if needed) and such. Other things about the query, such as query input parameters, are already set.
    //
    gen_ctx->qry[query_id].text = qry;
    // create name for a query, it's an internal unique name. It is based on total # of queries so far,
    // since there can be many queries on the same level, say query (id 0), inside of it query (id 1), then
    // both end, so the next one is id 0 again, and one inside it id 1 again.
    name_query (gen_ctx, vp);
    gen_ctx->qry[query_id].returns_tuple = 1;
    gen_ctx->qry[query_id].qry_found_total_inputs = 0;
    gen_ctx->qry[query_id].unknown_output = 0;
    gen_ctx->qry[query_id].qry_total_outputs = 0;
    gen_ctx->qry[query_id].qry_out_defined = false;
    // only up the number of queries if this is a new query
    // query, once used, is always used. So mark it used only once it added the very first time
    gen_ctx->qry_used[query_id] = VV_QRY_UNUSED;
    gen_ctx->total_queries++;
    if (gen_ctx->total_queries >= VV_MAX_QUERY)
    {
        _vely_report_error("Too many queries specified");
    }
}

// 
// Find current query ID - all references to queries are implicit and refer to the current one. gen_ctx is the context.
// Returns query ID or -1 if none found..
//
num find_query (vely_gen_ctx *gen_ctx)
{
    assert (gen_ctx);
    return gen_ctx->curr_qry_ptr - 1;
}


// 
// Change pointer to string pointer s, so it advances until either 1) a comma is encountered or 2) the end of the string
// The pointer is now at either one
//
void get_until_comma (char **s)
{
    assert (*s != NULL);
    num i = 0;
    while (((*s)[i])!=',' && (*s)[i]!=0) i++;
    *s = *s + i;
}

// 
// Change pointer to string pointer s, so it advances until either 1) a whitespace is encountered or 2) the end of the string
// The pointer is now at whitespace
//
void get_until_whitespace (char **s)
{
    assert (*s != NULL);
    num i = 0;
    while (!isspace ((*s)[i])) i++;
    *s = *s + i;
}

// 
// Change pointer to string pointer s, so it advances until either 1) a non-whitespace is encountered or 2) the end of the string
// The pointer is now non-whitespace (presumably after skipping bunch (or zero) of whitespaces)
//
void get_passed_whitespace (char **s)
{
    assert (*s != NULL);
    num i = 0;
    while (isspace ((*s)[i])) i++;
    *s = *s + i;
}



//
// Recognition of a statement. cinp is the line read from source file, pos is the current byte position in it (from where to 
// search for a statement), opt is the statement itself (a string to search for). Outputs mtext (which is the pointer to just after
// the statement, minus leading spaces), msize is the length of it. 
// isLast is 1 if this statement should NOT have any data after it, 0 if it should.
// Returns byte position in cinp right after where statement is found, or 0 if opt statement not recognized or if recognized, but
// there is more immediately appended text (else but there could be else-if actually).
// mtext is null-terminated so it can be vely_trim()-ed.
// is_inline is 1 if this was the end (>>) of <<..>> inline statement
// Errors out if there is an unterminated string after statement, or if arguments aren't there (either end of string or >>)
//
num recog_statement (char *cinp, num pos, char *opt, char **mtext, num *msize, num isLast, num *is_inline)
{
    assert (cinp);
    assert (opt);
    assert(msize);


    *is_inline = 0; // by default this isn't closed by >>

    // do not assign mtext in case of not-found, because it could override the found one when
    // we check for multiple recog_statement() in the same if(), same for msize
    num opt_len = strlen (opt);
    num orig_position = pos;
    if (!memcmp (cinp+pos, opt, opt_len))
    {

        *mtext = cinp+pos+opt_len;
        // can NOT skip leading spaces, because there's at least one space, and it is considered a keyword
        // in carve_statement(), so it must be there
        pos += opt_len;
        if (isLast == 1)
        {
            //
            // Nothing is expected after this statement, i.e. no text
            //
            while (isspace(cinp[pos])) pos++;
            // we consider end of line to be the same as >> 
            // we already handle \ (line continuation) before we encounter the first recog_statement
            // (this is done during reading from the C file)
            num is_end_of_inline = !memcmp (cinp+pos, ">>", 2) ? 1: 0;
            if (is_end_of_inline == 1)
            {
                *is_inline = 1;
                if (within_inline != 1)  _vely_report_error ("End of inline detected (>>), but no beginning (<<)");
                within_inline = 0;
            }
            if (cinp[pos] == 0 || is_end_of_inline == 1)
            {
                usedVELY = 0;
                *msize = (cinp+pos)-*mtext;
                (*mtext)[*msize] = 0; // zero the >>
                VV_VERBOSE(lnum,"Markup [%s] found", opt);
                // vely line never has new line in it, so this should be fine
                oprintf("\n//%s\n", cinp+orig_position);
                return pos + 1; // cinp[pos+1] must be '>' in the last '>>', i.e. the last char
            }
            else
            {
                return 0; // there can be else versus else-if : do not say 'extra characters...'
            }
        }
        else
        {
            // find >> (if present), which would finish the statement, so we can null-bind mtext.
            // if not, then mtext ends at end-of-line. In both cases, unterminated strings and unbalanced
            // () are found and error raised.
            char *eofc = NULL;
            if ((eofc = find_keyword(cinp+pos, ">>", 0)) != NULL)
            {
                *is_inline = 1;
                if (within_inline != 1)  _vely_report_error ("End of inline detected (>>), but no beginning (<<)");
                within_inline = 0;
                *eofc = 0;
            }
            else
            {
               eofc = find_keyword(cinp+pos, "", 0);
            }
            pos += (eofc - (cinp+pos));
            //
            // After this statement, we expect more text
            // We expect space after statement keyword
            // We also don't look for statement ending (i.e. >>) if it is quoted, such as 
            // for example with query text.
            // We will look for \" inside a string (an escaped quote), because C code might have this, so an
            // escaped quote is NOT end of the string. This is useful primarily in C code (c statement).
            // We do NOT interpret \", it is specified
            // so in the string and it remains so (i.e. we don't switch \" for " or anything at all). 
            // This does not affect SQL queries (think injection) because this affects only the code
            // programmer writes, and we also use ANSI single quotes for all sessions.
            //
            *msize = (cinp+pos)-*mtext;
            // Check that statement doesn't end with semicolon. @ and ! are covered outside recog_statement()
            // and we check this isn't C code
            if (strcmp(opt, ".") && *msize>0 && (*mtext)[*msize-1] == ';') 
            {
                _vely_report_error( "Statement '%s' cannot end with semicolon. Only C code ends with semicolon", opt);
            }
            // if opt is empty, the whole vely marker is c code, just pass it back
            //
            // in case some statements are subsets of other (such as else versus else-if)
            // we expect a space after it (since it's not the last!)
            // The exception is '.' statement, which doesn't need a space but has
            // text after it.
            if (cinp[orig_position+opt_len] != ' ' && strcmp(opt,"."))
            {
                // check if there's nothing after keyword - something must be there
                // note that input is trimmed, so even if bunch of spaces after keyword, it will be as nothing.
                if (orig_position+opt_len == pos)
                {
                    // error out if only spaces (through the end of statement) follow the statement key word - this means
                    // else and else-if will NOT error out, but only legitimate issues
                    _vely_report_error( "Statement '%s' has no arguments, but it should", opt);
                }
                return 0; // must have space afterward
            }
            usedVELY = 0;
            VV_VERBOSE(lnum,"Markup [%s] found", opt);
            oprintf("//%s\n", cinp+orig_position);
            return pos + 1;
        }
    }
    return 0;
}


// 
// Initialize context gen_ctx. Initialize command mode, all queries, query stack, string writing balancer
// and database location.
//
void init_vely_gen_ctx (vely_gen_ctx *gen_ctx)
{
    VV_TRACE("");

    num i;
    num j;
    for (j = 0; j < VV_MAX_QUERY; j++) 
    {
        gen_ctx->qry[j].returns_tuple = 1;
        for (i = 0; i < VV_MAX_QUERY_INPUTS; i++)  
        {
            gen_ctx->qry[j].qry_inputs[i][0] = 0;
        }
    }

    gen_ctx->total_queries = 0;

    gen_ctx->curr_qry_ptr = 0;

    gen_ctx->total_write_string = 0;


}




// 
// Main code generation function. gen_ctx is the context, file_name is the file
// that is being processed.
// Errors out if cannot open file and for many other reasons (including syntax and 
// other errors in statement). 
// Generates the code which is output according to the options used (see the documentation).
//
void vely_gen_c_code (vely_gen_ctx *gen_ctx, char *file_name)
{
    FILE *f;

    f = vely_fopen (file_name, "r");
    VV_VERBOSE(0,"Starting");
    if (f == NULL)
    {
        _vely_report_error( "Error opening file [%s]", file_name);
    }

    //
    // Start reading line by line from the source file
    //
    char *res;

    VV_VERBOSE(0,"Opened your file [%s]",file_name);

    num json_id = 0;
    num open_readline = 0;
    num open_ifs = 0;
    num line_len = 0; // unlike 'len' below, used only for concatenation of lines

    num http_header_count = 0; // used for _vely_tmp_header_%lld 
    num encrypt_count=0; // used for encryption length var
    num file_count=0; // used for static file descriptors
    num trim_count=0; // used for trim length var
    num code_json = 0; // counter for status/errt vars
    num regex_cache = 0; // used to cache regex compilation


    bool ccomm_open = false; // true if C comment opened in some line prior
    num ccomm_line = -1; // line where /* C comment opened

    // 
    // Main loop in which lines are read from the source file
    //
    while (1)
    {
        res = fgets (line + line_len, sizeof (line) - line_len- 1, f);
        if (res == NULL) // either end or error
        {
            num err = ferror (f);
            if (err) // if error, quit
            {
                _vely_report_error( "Error [%s] reading file [%s]", strerror (errno), file_name);
            }
            break; // nothing read in, exit normally
        }
        lnum++;

        // 
        // Setup line number for error reporting. Print new line in front
        // so if we forget to finish a line with new line, this would ruin
        // C syntax.
        // For internal debugging, may be useful not to have it.
        //
        if (VV_EMIT_LINE)
        {
            oprintf("\n#line %lld \"%s\"\n", vv_plain_diag == 1 ? lnum : lnum*10000, file_name);
        }

        // 
        // if this is continuation, trim the continuation to avoid big gaps in between pieces
        // Lines can be concatenated
        //
        if (line_len > 0)
        {
            num cont_len = strlen (line+line_len);
            // check if continuation starts with space. If it doesn't, we cannot trim.
            if (cont_len > 1 && isspace(line[line_len])) 
            {
                // We used to leave a space when lines are concatenated with / but NO MORE. The space
                // should be made in the code, if needed. The reason is that space-sensitive parsing will NOT 
                // work if we arbitrarily leave a space in between lines. Concatenation means just that - concatenation.
                // form incorrect syntax because there would be no spaces at all in between them
                vely_trim (line+line_len, &cont_len);
            }
        }

        num i;
        num len = strlen (line);

        if (len >= (num)sizeof (line) - 2) // don't allow too big of a line
        {
            _vely_report_error( "Line too long");
        }

        vely_trim (line, &len);


        if (len > 0 && line[len - 1] == '\\')
        {
            // continuation of the line
            line[len - 1] = 0;
            line_len = len - 1;
            continue; // read more
        }
        line_len = 0;

        VV_VERBOSE(lnum,"Got [%s]", line);

        char *mtext = NULL;
        num msize = 0;
        num first_on_line = 1;
        num is_verbatim = 0; // 1 if #  to print out
        within_inline = 0; // if statement is within <<...>>
        usedVELY = 0;
        print_mode = 0; // if in @ or !

        // 
        //
        // Comments: both // and /**/ are handled here
        // Not trivial. Matches gcc behavior as best as possible. Generally /**/
        // takes precedence over //, unless /* is after //
        //
        //
        // Check for // comment, if found (and not in quotes), anything after that is ignored
        //
        // // comment: if not quoted, everyting after it is blanked out
        num op_i = 0;
        num ccomm_pos = -1; // position of /* that starts multiline comment
        while (isspace (line[op_i])) op_i++; // find beginning of line (ignore whitespace)
        // if this is @ or !, all comments are ignore, this is whole-line output statement
        if (memcmp(line+op_i, "@",1) && memcmp(line+op_i, "!",1)) 
        {
            // Check for /**/ comment
            // There can be multiple /**/ comments on a line, so this must be a loop
            bool skip_line=false;
            num lcheck = 0;
            bool closing_ccomm_found = false;// true if */ potentially found without matching /* on the same line
                                             // may be a false alarm if */ is after //
            num closing_ccomm_pos = -1;// position of closing */ per above bool
            while (1)
            {
                char *cl_comment = vely_find_keyword0 (line+lcheck, "/*", 0, 0);
                char *cr_comment = vely_find_keyword0 (line+lcheck, "*/", 0, 0);
                // calculate where /* and */ are, meaning the first ones found
                num l_pos = -1;
                num r_pos = -1;
                if (cl_comment != NULL) l_pos = cl_comment - line;
                if (cr_comment != NULL) r_pos = cr_comment - line;
                // If /* started at some point and not */ here, then ignore line
                if (ccomm_open)
                {
                    if (cr_comment == NULL) 
                    {
                        skip_line = true;
                        break; // this is under multiline /* */ comment
                    }
                    else
                    {
                        // this is ...*/... which must be closure of multiline /**/ comment
                        // So ccomm_open must be true. We don't care if there was /* before */, that's part of the comment
                        memmove (line, cr_comment + 2, len-r_pos-2+1); //+1 at the end is to include null byte
                        len = len - r_pos - 2; // length is smaller
                                             // we ignore all up to */ and continue afterwards
                        lcheck = 0; // continue from the beginning of code after /*
                        ccomm_open = false; // no longer multiline /**/ comment
                        continue;
                    }
                }
                if (l_pos == -1 && r_pos == -1) break; // no comments on line that's to be processed
                                                                     // since it's not in multiline /**/ comment
                if (l_pos != -1 && r_pos == -1) 
                {
                    // has /* but no closing */
                    len = cl_comment - line; // length is smaller once you cut out the rest of the line
                    line[len] = 0; // /* is not ending in this line, so cut it out
                    ccomm_open = true; // we're now in open-ended /* which will stretch more than one line
                    ccomm_line = lnum; // line where open-ended /* started
                    ccomm_pos = len; // where /* is
                    break; // because after /* without */, anything after that is a comment, so we got line
                } 
                else if (l_pos != -1 && r_pos != -1) 
                {
                    // we have left /* and right */ comment on the same line, so blank it out
                    // but which one comes first?
                    if (l_pos < r_pos) 
                    {
                        char *ci;
                        for (ci = cl_comment; ci != cr_comment+2; ci++) *ci=' ';
                        lcheck = r_pos + 2; // start looking again after this comment blanked out
                    } 
                    else
                    {
                        // we have already handled closure of multiline comment */ above, so this is an error
                        // to have */ to lead the line - this will be handled further out of this loop under cpp_comment!=NULL
                        // because if there's // prior to it, then it's a comment and not an error - in which case the rest of the 
                        // line doesn't matter, so break
                        closing_ccomm_found = true;
                        closing_ccomm_pos = r_pos;
                        break;
                    }
                } 
                else if (l_pos == -1 && r_pos != -1) 
                {
                    // this is ...*/... which must be closure of multiline /**/ comment
                    // But we already handled that, so this is an error,
                    // to have */ to lead the line - this will be handled further out of this loop under cpp_comment!=NULL
                    // because if there's // prior to it, then it's a comment and not an error - in which case the rest of the
                    // line doesn't matter, so break
                    closing_ccomm_found = true;
                    closing_ccomm_pos = r_pos;
                    break;
                }
                // cannot happen l_pos == -1 && r_pos == -1, as we checked for that above
            }
            // Look for // only after all /**/ have been processed. If // still here, then the only
            // case where it affect the outcome is if there's /* after //, which should be ignored
            char *cpp_comment = vely_find_keyword0 (line, "//", 0, 0);
            if (cpp_comment != NULL)
            {
                *cpp_comment = 0;
                len = cpp_comment - line; // after removing the comment, length is smaller
                // if /* for multiline is after //, then ignore it
                if (ccomm_pos !=-1 && ccomm_pos > len) ccomm_open = false;
                // if */ is found after //, then ignore it, unless it closes a multi-line /**/. If it doesn't
                // then it's ignored 
                if (closing_ccomm_found && closing_ccomm_pos < len) _vely_report_error( "Closing */ C comment found, but there was no opening /* comment");
            }
            if (skip_line) continue; // this line is a line where /* started before it, but no */ found yet, so ignore
        }
        //
        // END comment handling
        //

        // If in @, this is if >> was just processed (end of inline), so start printing again
        // as we have temporarily suspended printing when << was found (by doing vely_puts(<end of string>...)
        // reset vely_is_inline afterwards so any new checks (aside from recog_statement() which sets vely_is_inline)
        // do not get wrong value
#define VV_OPEN_PRINT  if (vely_is_inline == 1 && print_mode == 1) oprintf("vely_puts (VV_NOENC, \""); \
        vely_is_inline = 0; 

        //
        // In this loop, a line is examined char by char. However, once certain patterns are recognized, parsing
        // progresses more rapidly. Basically, recog_statement() is called to recognized the beginning of the VELY statement,
        // and from there the rest is parsed in a more rapid fashion. Each statement is checked, and once recognized and
        // processed, the next line is read in the outer loop.
        //
        for (i = 0; i < len; i++)
        {
            num newI = 0;
            num newI1 = 0;
            num newI2 = 0;
            num newI3 = 0;
            num newI4 = 0;
            num newI8 = 0;

            // since every statement, after it's recognized and consumed, does continue, it comes back here.
            // so if statement was inline, and within @, then we must start printing. This is first thing right 
            // after >>. We stopped printing at <<. So this continues printing; see the VV_OPEN_PRINT after this for loop
            VV_OPEN_PRINT

            if (is_verbatim == 0) 
            {

                if (!memcmp (line + i, "<<", 2)) // short directive
                {
                    i = i + 2; // past "<<"
                    within_inline = 1;
                    usedVELY = 1;
                    if (print_mode == 1)
                    {
                        // Print new line at the end, finish current portion of @ line, so that <<>> can do its thing
                        // if not in @, then just execute
                        oprintf("\");\n");
                    }
                }
                else
                {
                    ; // else? nothing. We consider new line start within VV_BEGIN/VV_END block to be the start
                    // of the <<vely statement>>, for simplicity of coding.
                }


                // checking for VELY statement without <<..>> can only work in those statements where they are the first on the 
                // line, OR if this is actual statement << ...>>
                if (first_on_line == 1 || usedVELY == 1)
                {
                    while (isspace (line[i])) i++;
                    first_on_line = 0;

                    //
                    // All the following statements use the same recipe, calling recog_statement()
                    // and advancing 'i' which points to the next segment to be parsed, while mtext/msize are the text of the 
                    // statement (after the initial statement keyword). The 1 or 0 after msize tells if this statement is alone or if it
                    // needs more data after the statement keyword. For example, end-query has nothing after the statement keyword 'end-query'
                    // and so it is alone on the line and argument following msize is 1.
                    //
                    // Parsing done typically involves finding keywords and then dividing the string into smaller pieces (based on
                    // the location of these keywords and the information contained between them or between such keywords and the end of string), typically
                    // a single piece of information is to be found in each piece. It's a
                    // parsing mechanism that's more suited for statement-like statements, where such statements are given through parameters
                    // which control what is being done, on what data, and how. Examples of statements given in the documentation present the 
                    // use scenarios. Markups are generally quite simple and more importantly are designed to be simple
                    // and to streamline commonly used functionality in an 80/20 fashion. The 20% functionality is typically done in pure C, and
                    // 80% functionality in simple statements. This tends to reduce not only the number of bugs, but also to force clearer and simpler
                    // programming (while the code generate is still C and thus typically faster than most anything else).
                    // 
                    //
                    if ((newI=recog_statement(line, i, "end-query", &mtext, &msize, 1, &vely_is_inline)) != 0) // is it end of query action block
                    {
                        VV_GUARD
                        i = newI;

                        end_query (gen_ctx,  1);

                        continue; // skip the statement and continue analyzing 
                    }
                    // Checking for non-mandatory (6th param is 1) MUST be done first.
                    else if (((newI=recog_statement(line, i, "rollback-transaction", &mtext, &msize, 1, &vely_is_inline)) != 0) 
                     || ((newI1=recog_statement(line, i, "rollback-transaction", &mtext, &msize, 0, &vely_is_inline)) != 0)) 
                    {
                        VV_GUARD
                        i = newI+newI1;
                        char *database = find_keyword (mtext, VV_KEYAT, 0);
                        char *oncont = find_keyword (mtext, VV_KEYONERRORCONTINUE, 1);
                        char *onexit = find_keyword (mtext, VV_KEYONERROREXIT, 1);
                        char *errt = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        char *err = find_keyword (mtext, VV_KEYERROR, 1);
                        carve_statement (&database, "rollback-transaction", VV_KEYAT, 0, 1);
                        carve_statement (&oncont, "rollback-transaction", VV_KEYONERRORCONTINUE, 0, 0);
                        carve_statement (&onexit, "rollback-transaction", VV_KEYONERROREXIT, 0, 0);
                        carve_statement (&errt, "rollback-transaction", VV_KEYERRORTEXT, 0, 1);
                        carve_statement (&err, "rollback-transaction", VV_KEYERROR, 0, 1);
                        define_statement (&errt, VV_DEFSTRING);
                        define_statement (&err, VV_DEFSTRING);
                        get_db_config (database);
                        //erract is VV_ON_ERROR_CONTINUE/EXIT for statement-specific on-error continue/exit or VV_OKAY if db-level on-error is in effect
                        oprintf("vely_rollback (\"%s\", %s, %s%s%s, %s%s%s);\n", mtext, oncont!=NULL?"VV_ON_ERROR_CONTINUE":(onexit!=NULL?"VV_ON_ERROR_EXIT":"VV_OKAY"),
                            err!=NULL?"&(":"", err !=NULL?err:"NULL", err!=NULL?")":"", 
                            errt!=NULL?"&(":"", errt !=NULL?errt:"NULL", errt!=NULL?")":""
                            );
                        continue;
                    }
                    else if (((newI=recog_statement(line, i, "on-error", &mtext, &msize, 0, &vely_is_inline)) != 0)) // commit transaction
                    {
                        VV_GUARD
                        i = newI;
                        char *database = find_keyword (mtext, VV_KEYAT, 0);
                        char *exit = find_keyword (mtext, VV_KEYEXIT, 1);
                        char *cont = find_keyword (mtext, VV_KEYCONTINUE, 1);
                        carve_statement (&database, "on-error", VV_KEYAT, 0, 1);
                        carve_statement (&exit, "on-error", VV_KEYEXIT, 0, 0);
                        carve_statement (&cont, "on-error", VV_KEYCONTINUE, 0, 0);

                        if (exit == NULL && cont == NULL) _vely_report_error( "Missing 'exit' or 'continue'");
                        if (exit != NULL && cont != NULL) _vely_report_error( "Cannot have both 'exit' or 'continue'");

                        get_db_config (database);
                        oprintf ("vely_get_config()->ctx.db->conn[vely_get_config()->ctx.db->ind_current_db].exit_on_error=%d;\n", exit != NULL ? 1 : 0);
                        continue;
                    }
                    // Checking for non-mandatory (6th param is 1) MUST be done first.
                    else if (((newI=recog_statement(line, i, "commit-transaction", &mtext, &msize, 1, &vely_is_inline)) != 0)
                     || ((newI1=recog_statement(line, i, "commit-transaction", &mtext, &msize, 0, &vely_is_inline)) != 0)) 
                    {
                        VV_GUARD
                        i = newI+newI1;
                        char *database = find_keyword (mtext, VV_KEYAT, 0);
                        char *oncont = find_keyword (mtext, VV_KEYONERRORCONTINUE, 1);
                        char *onexit = find_keyword (mtext, VV_KEYONERROREXIT, 1);
                        char *errt = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        char *err = find_keyword (mtext, VV_KEYERROR, 1);
                        carve_statement (&database, "commit-transaction", VV_KEYAT, 0, 1);
                        carve_statement (&oncont, "commit-transaction", VV_KEYONERRORCONTINUE, 0, 0);
                        carve_statement (&onexit, "commit-transaction", VV_KEYONERROREXIT, 0, 0);
                        carve_statement (&errt, "commit-transaction", VV_KEYERRORTEXT, 0, 1);
                        carve_statement (&err, "commit-transaction", VV_KEYERROR, 0, 1);
                        define_statement (&errt, VV_DEFSTRING);
                        define_statement (&err, VV_DEFSTRING);
                        get_db_config (database);
                        //erract is VV_ON_ERROR_CONTINUE/EXIT for statement-specific on-error continue/exit or VV_OKAY if db-level on-error is in effect
                        oprintf("vely_commit (\"%s\", %s, %s%s%s, %s%s%s);\n", mtext, oncont!=NULL?"VV_ON_ERROR_CONTINUE":(onexit!=NULL?"VV_ON_ERROR_EXIT":"VV_OKAY"),
                            err!=NULL?"&(":"", err !=NULL?err:"NULL", err!=NULL?")":"", 
                            errt!=NULL?"&(":"", errt !=NULL?errt:"NULL", errt!=NULL?")":""
                            );
                        continue;
                    }
                    // Checking for non-mandatory (6th param is 1) MUST be done first.
                    else if (((newI=recog_statement(line, i, "begin-transaction", &mtext, &msize, 1, &vely_is_inline)) != 0) 
                        || ((newI1=recog_statement(line, i, "begin-transaction", &mtext, &msize, 0, &vely_is_inline)) != 0))
                    {
                        VV_GUARD
                        i = newI+newI1;
                        char *database = find_keyword (mtext, VV_KEYAT, 0);
                        char *oncont = find_keyword (mtext, VV_KEYONERRORCONTINUE, 1);
                        char *onexit = find_keyword (mtext, VV_KEYONERROREXIT, 1);
                        char *errt = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        char *err = find_keyword (mtext, VV_KEYERROR, 1);
                        carve_statement (&database, "begin-transaction", VV_KEYAT, 0, 1);
                        carve_statement (&oncont, "begin-transaction", VV_KEYONERRORCONTINUE, 0, 0);
                        carve_statement (&onexit, "begin-transaction", VV_KEYONERROREXIT, 0, 0);
                        carve_statement (&errt, "begin-transaction", VV_KEYERRORTEXT, 0, 1);
                        carve_statement (&err, "begin-transaction", VV_KEYERROR, 0, 1);
                        define_statement (&errt, VV_DEFSTRING);
                        define_statement (&err, VV_DEFSTRING);
                        get_db_config (database);
                        //erract is VV_ON_ERROR_CONTINUE/EXIT for statement-specific on-error continue/exit or VV_OKAY if db-level on-error is in effect
                        oprintf("vely_begin_transaction (\"%s\", %s, %s%s%s, %s%s%s);\n", mtext, oncont!=NULL?"VV_ON_ERROR_CONTINUE":(onexit!=NULL?"VV_ON_ERROR_EXIT":"VV_OKAY"),
                            err!=NULL?"&(":"", err !=NULL?err:"NULL", err!=NULL?")":"", 
                            errt!=NULL?"&(":"", errt !=NULL?errt:"NULL", errt!=NULL?")":""
                            );
                        continue;
                    }
                    // Checking for non-mandatory (6th param is 1) MUST be done first.
                    else if (((newI1=recog_statement(line, i, "current-row", &mtext, &msize, 1, &vely_is_inline)) != 0)  // this is query row (number)
                         || (newI=recog_statement(line, i, "current-row", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI+newI1;
                        if (query_id == -1) _vely_report_error( "Query usage attempted, but no query present and active");

                        if (gen_ctx->qry[query_id].returns_tuple == 0)
                        {
                            _vely_report_error( "current-row cannot be used on query [%s] because it does not output any columns", gen_ctx->qry[query_id].name);
                        }
                        char *to = NULL;
                        if (newI != 0)
                        {
                            to = find_keyword (mtext, VV_KEYTO, 1);
                            carve_statement (&to, "current-row", VV_KEYTO, 1, 1);
                            define_statement (&to, VV_DEFNUM);
                        }

                        if (to != NULL)
                        {
                            // this happens right in the run-time, as vely_printf_noenc is going on
                            oprintf("%s = _vv_iter_%s+1;\n", to, gen_ctx->qry[query_id].name);
                        }
                        else
                        {
                            oprintf("vely_printf (false, VV_NOENC, \"%%lld\", _vv_iter_%s+1);\n", gen_ctx->qry[query_id].name); 
                        }
                        
                        continue;
                    }
                    else if ((newI1=recog_statement(line, i, "delete-query", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI1;
                        char *skipd = find_keyword (mtext, VV_KEYSKIPDATA, 0);
                        carve_statement (&skipd, "delete-query", VV_KEYSKIPDATA, 0, 0);

                        // trim query name, or generated code will be incorrect
                        char *qname = vely_strdup(mtext);
                        num lname = strlen (qname);
                        vely_trim (qname, &lname);
                        // free query generate code
                        free_query (qname, skipd!=NULL?true:false);

                        continue;
                    }
                    else if ((newI1=recog_statement(line, i, "query-result", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI1;

                        //
                        // find keyword must be first
                        //
                        // these are keywords needed to perform syntax highlighting, do NOT remove this comment!
                        // VV_KEYTO VV_KEYLENGTH VV_KEYURLENCODE VV_KEYWEBENCODE VV_KEYNOENCODE
                        query_result (gen_ctx, mtext);

                        continue;
                    }
                    else if (((newI4=recog_statement(line, i, "run-prepared-query", &mtext, &msize, 0, &vely_is_inline)) != 0)
                        || ((newI8=recog_statement(line, i, "run-query", &mtext, &msize, 0, &vely_is_inline)) != 0))
                    {
                        VV_GUARD
                        i = newI8+newI4;

                        //
                        // these are keywords needed to perform syntax highlighting, do NOT remove this comment!
                        // VV_KEYOUTPUT VV_KEYUNKNOWNOUTPUT VV_KEYNOLOOP VV_KEYAFFECTEDROWS VV_KEYROWCOUNT VV_KEYERROR VV_KEYCOLUMNNAMES VV_KEYCOLUMNCOUNT VV_KEYCOLUMNDATA VV_KEYERRORTEXT VV_KEYONERRORCONTINUE VV_KEYONERROREXIT

                        char is_prep = ((newI4 !=0) ? 1:0);

                        vely_db_parse vp;
                        memset  (&vp, 0, sizeof(vely_db_parse)); // all are NULL now

                        //
                        // find keyword must be first
                        //

                        // run-query 
                        vp.coldata = find_keyword (mtext, VV_KEYCOLUMNDATA, 1);
                        vp.colnames = find_keyword (mtext, VV_KEYCOLUMNNAMES, 1);
                        vp.colcount = find_keyword (mtext, VV_KEYCOLUMNCOUNT, 1);
                        vp.on_error_cont = find_keyword (mtext, VV_KEYONERRORCONTINUE, 1);
                        vp.on_error_exit = find_keyword (mtext, VV_KEYONERROREXIT, 1);
                        vp.with_output = find_keyword (mtext, VV_KEYOUTPUT, 1);
                        vp.with_unknown_output = find_keyword (mtext, VV_KEYUNKNOWNOUTPUT, 1);
                        vp.colon = find_keyword (mtext, VV_KEYCOLON, 0);
                        vp.eq = find_keyword (mtext, VV_KEYEQUALSHORT, 0);
                        vp.at = find_keyword (mtext, VV_KEYAT, 0);
                        vp.noloop = find_keyword (mtext, VV_KEYNOLOOP, 0);
                        vp.err = find_keyword (mtext, VV_KEYERROR, 1);
                        vp.errtext = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        vp.arows = find_keyword (mtext, VV_KEYAFFECTEDROWS, 1);
                        vp.rcount = find_keyword (mtext, VV_KEYROWCOUNT, 1);
                        vp.name = find_keyword (mtext, VV_KEYNAME, 1);

                        //
                        // Carving must be after find_keyword
                        //

                        // carve for run-query, match above defines
                        carve_statement (&(vp.coldata), "run-query", VV_KEYCOLUMNDATA, 0, 1);
                        carve_statement (&(vp.colnames), "run-query", VV_KEYCOLUMNNAMES, 0, 1);
                        carve_statement (&(vp.colcount), "run-query", VV_KEYCOLUMNCOUNT, 0, 1);
                        carve_statement (&(vp.on_error_cont), "run-query", VV_KEYONERRORCONTINUE, 0, 0);
                        carve_statement (&(vp.on_error_exit), "run-query", VV_KEYONERROREXIT, 0, 0);
                        carve_statement (&(vp.with_output), "run-query", VV_KEYOUTPUT, 0, 1);
                        carve_statement (&(vp.with_unknown_output), "run-query", VV_KEYUNKNOWNOUTPUT, 0, 0);
                        carve_statement (&(vp.colon), "run-query", VV_KEYCOLON, 0, 1);
                        carve_statement (&(vp.eq), "run-query", VV_KEYEQUALSHORT, 1, 1);
                        carve_statement (&(vp.at), "run-query", VV_KEYAT, 0, 1);
                        carve_statement (&(vp.noloop), "run-query", VV_KEYNOLOOP, 0, 0);
                        carve_statement (&(vp.err), "run-query", VV_KEYERROR, 0, 1);
                        carve_statement (&(vp.errtext), "run-query", VV_KEYERRORTEXT, 0, 1);
                        carve_statement (&(vp.arows), "run-query", VV_KEYAFFECTEDROWS, 0, 1);
                        carve_statement (&(vp.rcount), "run-query", VV_KEYROWCOUNT, 0, 1);
                        carve_statement (&(vp.name), "run-query", VV_KEYNAME, 0, 1);


                        //
                        // Define must be after carving
                        //

                        // run-query has output vars that can be defined
                        define_statement (&(vp.err), VV_DEFSTRING);
                        define_statement (&(vp.errtext), VV_DEFSTRING);
                        define_statement (&(vp.arows), VV_DEFNUM);
                        define_statement (&(vp.rcount), VV_DEFNUM);
                        define_statement (&(vp.colnames), VV_DEFCHARPTRPTR);
                        define_statement (&(vp.coldata), VV_DEFCHARPTRPTR);
                        define_statement (&(vp.colcount), VV_DEFNUM);

                        // check on-error: can be only continue or exit
                        on_error_act (vp.on_error_cont, vp.on_error_exit, &(vp.on_error_action));

                        get_db_config (vp.at);

                        // now @db has been cutoff with 0 char  and
                        // also vely_get_config()->ctx.db->conn[vely_get_config()->ctx.db->ind_current_db].db_name is now correct

                        process_query (gen_ctx, 0, 1, is_prep, &vp);

                        continue;
                    }

                    // web-output line
                    else if (!memcmp(line+i, "@",1) || !memcmp(line+i, "!",1))
                    {
                        if (within_inline == 1)
                        {
                            _vely_report_error("Cannot use output statements (such as @ or !) within an inline statement (<< ... >>)");
                        }
                        oprintf("//%s\n", line+i); // there can never be \n in line, so this should work
                        // @, !  is a synonym for an output line.  ! is verbatim, @ respects << ... >>
                        usedVELY = 0; // reset usedVELY because "@/!" isn't actual directive, it only
                                    // says what follows is just printed out
                        if (line[i+1] == 0)
                        {
                            // if empty, print empty line, and continue
                            oprintf("\n");

                        }
                        oprintf("vely_puts (VV_NOENC, \""); // open the text line for free-text unencoded output
                        print_mode = 1;
                        if (line[i] == '!') is_verbatim = 1;
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, ".", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        i = newI;
                        // C code
                        oprintf("%s\n", mtext); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "p-out", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_puts (VV_NOENC, %s);\n", mtext); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "p-dbl", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_printf (false, VV_NOENC, \"%%f\", %s);\n", mtext); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "p-path", &mtext, &msize, 1, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_puts (VV_WEB, vely_app_path);\n");

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "p-num", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_printf (false, VV_NOENC, \"%%lld\", %s);\n", mtext); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "p-url", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_puts (VV_URL, %s);\n", mtext); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "p-web", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_puts (VV_WEB, %s);\n", mtext); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "exit-code", &mtext, &msize, 0, &vely_is_inline)) != 0 ) 
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_get_config ()->ctx.req->ec=(%s);\n", mtext);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "silent-header", &mtext, &msize, 1, &vely_is_inline)) != 0 ) 
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("vely_get_config ()->ctx.req->silent=1;\n");

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "out-header", &mtext, &msize, 0, &vely_is_inline)) != 0 ) 
                    {
                        VV_GUARD
                        i = newI;
                        char *use_default = find_keyword (mtext, VV_KEYDEFAULT, 1);
                        char *use = find_keyword (mtext, VV_KEYUSE, 1);
                        carve_statement (&use, "out-header", VV_KEYUSE, 0, 1);
                        carve_statement (&use_default, "out-header", VV_KEYDEFAULT, 0, 0);

                        if (use == NULL && use_default == NULL) _vely_report_error( "Must use either 'use' or 'default' to specify header in out-header");
                        if (use != NULL && use_default != NULL) _vely_report_error( "Cannot use both 'use' or 'default' in out-header");
                        if (use != NULL) 
                        {
                            //VV_HEADER_PAGE because output from a page isn't cached, it's dynamic
                            char *temp_header = make_default_header(VV_HEADER_PAGE, http_header_count, 0);
                            // VV_KEY* must be specified, even as a header, for getvim to pick up keywords for highlighting!
                            process_http_header ("out-header", use, temp_header, http_header_count, 0, NULL, NULL); // VV_KEYCONTENTTYPE VV_KEYDOWNLOAD VV_KEYETAG VV_KEYFILENAME VV_KEYCACHECONTROL VV_KEYNOCACHE VV_KEYSTATUSID VV_KEYSTATUSTEXT VV_KEYCUSTOM
                            oprintf("vely_get_config ()->ctx.req->header=&(%s);\n", temp_header);
                            http_header_count++;
                        }
                        oprintf("vely_output_http_header(vely_get_config ()->ctx.req);\n");

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "exec-program", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        

                        // Get keywords (if used)
                        char *program_args = find_keyword (mtext, VV_KEYARGS, 1);
                        char *program_status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *program_error = find_keyword (mtext, VV_KEYERROR, 1);
                        char *program_output = find_keyword (mtext, VV_KEYOUTPUT, 1);
                        char *program_output_length = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *program_input = find_keyword (mtext, VV_KEYINPUT, 1);
                        char *program_input_length = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *out_file = find_keyword (mtext, VV_KEYOUTPUTFILE, 1);
                        char *err_file = find_keyword (mtext, VV_KEYERRORFILE, 1);
                        char *in_file = find_keyword (mtext, VV_KEYINPUTFILE, 1);

                        carve_statement (&program_args, "exec-program", VV_KEYARGS, 0, 1);
                        carve_statement (&program_status, "exec-program", VV_KEYSTATUS, 0, 1);
                        carve_statement (&program_output, "exec-program", VV_KEYOUTPUT, 0, 1);
                        carve_statement (&program_error, "exec-program", VV_KEYERROR, 0, 1);
                        carve_statement (&program_output_length, "exec-program", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&program_input, "exec-program", VV_KEYINPUT, 0, 1);
                        carve_statement (&program_input_length, "exec-program", VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&out_file, "exec-program", VV_KEYOUTPUTFILE, 0, 1);
                        carve_statement (&err_file, "exec-program", VV_KEYERRORFILE, 0, 1);
                        carve_statement (&in_file, "exec-program", VV_KEYINPUTFILE, 0, 1);

                        define_statement (&program_output, VV_DEFSTRING);
                        define_statement (&program_error, VV_DEFSTRING);
                        define_statement (&program_output_length, VV_DEFNUM);
                        define_statement (&program_status, VV_DEFNUM);


                        // cannot have output-* and output (to string) at the same time
                        if (out_file != NULL && program_output != NULL) _vely_report_error( "Specify either output-file or output in exec-program statement");
                        if (err_file != NULL && program_error != NULL) _vely_report_error( "Specify either error-file or error in exec-program statement");
                        if (in_file != NULL && program_input != NULL) _vely_report_error( "Specify either input-file or input in exec-program statement");
                        if (program_input == NULL && program_input_length != NULL) _vely_report_error( "Cannot use input-length without 'input' in exec-program statement");
                        if (program_output == NULL && program_output_length != NULL) _vely_report_error( "Cannot use output-length without 'output' in exec-program statement");

                        oprintf ("num _vv_prg_status%lld = 0;\n", total_exec_programs);
                        oprintf("VV_UNUSED (_vv_prg_status%lld);\n", total_exec_programs); // if status not used by the developer
                        oprintf ("FILE *_vv_prg_out_file%lld = NULL;\nVV_UNUSED(_vv_prg_out_file%lld);\n", total_exec_programs, total_exec_programs);
                        oprintf ("FILE *_vv_prg_err_file%lld = NULL;\nVV_UNUSED(_vv_prg_err_file%lld);\n", total_exec_programs, total_exec_programs);
                        if (err_file != NULL)
                        {
                            // this is error-file. 
                            oprintf ("_vv_prg_err_file%lld = vely_fopen ((%s), \"w+\");\n", total_exec_programs, err_file);
                            // we set status to -8 if cannot open for writing
                            //VERR is set in vely_fopen
                            oprintf ("if (_vv_prg_err_file%lld == NULL) {_vv_prg_status%lld=VV_ERR_WRITE;} else { \n", total_exec_programs, total_exec_programs);
                        }
                        if (out_file != NULL)
                        {
                            // this is output-file. We open file for writing, and we close it after we're done
                            oprintf ("_vv_prg_out_file%lld = vely_fopen ((%s), \"w+\");\n", total_exec_programs, out_file);
                            // we set status to -8 if cannot open for writing
                            //VERR is set in vely_fopen
                            oprintf ("if (_vv_prg_out_file%lld == NULL) {_vv_prg_status%lld=VV_ERR_WRITE;} else { \n", total_exec_programs, total_exec_programs);
                        }

                        oprintf ("FILE *_vv_prg_in_file%lld = NULL;\nVV_UNUSED(_vv_prg_in_file%lld);\n", total_exec_programs, total_exec_programs);
                        if (in_file != NULL)
                        {
                            // this is input-file. We open it for reading, and close after we're done
                            oprintf ("_vv_prg_in_file%lld = vely_fopen ((%s), \"r\");\n", total_exec_programs, in_file);
                            // for status, we set it to -9 if cannot read
                            //VERR is set in vely_fopen
                            oprintf ("if (_vv_prg_in_file%lld == NULL) {_vv_prg_status%lld=VV_ERR_READ;} else {\n", total_exec_programs, total_exec_programs);
                        }



                        // store exec-program args arguments in an array
                        char prog_var[100];
                        snprintf (prog_var, sizeof(prog_var), "_vv_prg_arr%lld", total_exec_programs);
                        num exec_inputs = outargs(program_args, prog_var, "char *", 1, 0);

                        // generate run-time call to execute program
                        // exec_inputs is always at least 1 (to account for args[0] being program name)
                        // this is output (i.e. string output)
                        oprintf ("_vv_prg_status%lld = vely_exec_program(%s, _vv_prg_arr%lld, %lld, _vv_prg_in_file%lld, &(_vv_prg_out_file%lld), &(_vv_prg_err_file%lld), %s, %s, %s%s%s, %s%s%s, %s%s%s);\n",
                        total_exec_programs, 
                        mtext, 
                        total_exec_programs, 
                        exec_inputs, 
                        total_exec_programs,
                        total_exec_programs,
                        total_exec_programs,
                        program_input==NULL ? "NULL":program_input, 
                        program_input_length==NULL ?"0":program_input_length, 
                        program_output==NULL ? "":"&(", 
                        program_output==NULL ? "NULL":program_output, 
                        program_output==NULL ? "":")", 
                        program_output_length==NULL ? "":"&(", 
                        program_output_length==NULL ? "NULL":program_output_length, 
                        program_output_length==NULL ? "":")",
                        program_error==NULL ? "":"&(", 
                        program_error==NULL ? "NULL":program_error, 
                        program_error==NULL ? "":")");
                        if (err_file != NULL) oprintf ("}\n");
                        if (out_file != NULL) oprintf ("}\n");
                        if (in_file != NULL) oprintf ("}\n");
                        if (program_status!=0) oprintf("%s=_vv_prg_status%lld;\n", program_status, total_exec_programs);

                        total_exec_programs++; // advance exec-program counter so generating specific variables is unique
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "send-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        //
                        // Look for each option and collect relevant info
                        //
                        char *header = find_keyword (mtext, VV_KEYHEADERS, 1);
                        carve_statement (&header, "send-file", VV_KEYHEADERS, 0, 1);

                        // if there's no header, by default:
                        char *temp_header = make_default_header(VV_HEADER_FILE, http_header_count, 0);
                        oprintf ("(%s).file_name = basename(%s);\n", temp_header, mtext);

                        // specify keywords used in function for syntax highlighting to work properly
                        process_http_header ("send-file", header, temp_header, http_header_count, 0, NULL, NULL); // VV_KEYCONTENTTYPE VV_KEYDOWNLOAD VV_KEYETAG VV_KEYFILENAME VV_KEYCACHECONTROL VV_KEYNOCACHE VV_KEYSTATUSID VV_KEYSTATUSTEXT VV_KEYCUSTOM
                        http_header_count++;


                        oprintf ("vely_out_file(%s, &(%s));\n", mtext, temp_header);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "lock-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // lock-file "/home/vely/files" id [define] x status [define] st
                        // later
                        // unlock-file x
                        char *id = find_keyword (mtext, VV_KEYID, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&id, "lock-file", VV_KEYID, 1, 1);
                        carve_statement (&status, "lock-file", VV_KEYSTATUS, 1, 1);

                        define_statement (&id, VV_DEFNUM);
                        define_statement (&status, VV_DEFNUM);

                        oprintf ("%s=vely_lockfile (%s, &(%s));\n", status, mtext, id);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "unlock-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // unlock-file id x
                        char *id = find_keyword (mtext, VV_KEYID, 1);

                        carve_statement (&id, "lock-file", VV_KEYID, 1, 1);

                        oprintf ("close(%s);\n", id);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "random-string", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *length = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *number = find_keyword (mtext, VV_KEYNUMBER, 1);
                        char *binary = find_keyword (mtext, VV_KEYBINARY, 1);

                        carve_statement (&to, "random-string", VV_KEYTO, 1, 1);
                        carve_statement (&length, "random-string", VV_KEYLENGTH, 0, 1);
                        carve_statement (&number, "random-string", VV_KEYNUMBER, 0, 0);
                        carve_statement (&binary, "random-string", VV_KEYBINARY, 0, 0);

                        char type;
                        if (number != NULL) type = VV_RANDOM_NUM;
                        else if (binary != NULL) type = VV_RANDOM_BIN;
                        else type = VV_RANDOM_STR;

                        if (number != NULL && binary != NULL) _vely_report_error( "cannot use both 'number' and 'binary' in random-string statement");

                        define_statement (&to, VV_DEFSTRING);
                        if (length == NULL)
                        {
                            length="20";
                        }

                        oprintf ("vely_make_random (&(%s), (%s)+1, %d, false);\n", to, length, type);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "random-crypto", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *length = find_keyword (mtext, VV_KEYLENGTH, 1);

                        carve_statement (&to, "random-string", VV_KEYTO, 1, 1);
                        carve_statement (&length, "random-string", VV_KEYLENGTH, 0, 1);

                        define_statement (&to, VV_DEFSTRING);
                        if (length == NULL)
                        {
                            length="20";
                        }

                        oprintf ("vely_make_random (&(%s), (%s)+1, VV_RANDOM_BIN, true);\n", to, length);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "upper-string", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // upper-string str

                        oprintf ("vely_upper (%s);\n", mtext);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "lower-string", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // lower-string str

                        oprintf ("vely_lower (%s);\n", mtext);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "count-substring", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *in = find_keyword (mtext, VV_KEYIN, 1);
                        char *case_insensitive = find_keyword (mtext, VV_KEYCASEINSENSITIVE, 1);

                        carve_statement (&to, "count-substring", VV_KEYTO, 1, 1);
                        carve_statement (&in, "count-substring", VV_KEYIN, 1, 1);
                        carve_statement (&case_insensitive, "count-substring", VV_KEYCASEINSENSITIVE, 0, 2);

                        char *case_insensitivec = opt_clause(case_insensitive, "0", "1");

                        define_statement (&to, VV_DEFNUM);

                        oprintf ("%s=vely_count_substring (%s, %s, %s);\n", to, in, mtext, case_insensitivec);
                        vely_free(case_insensitivec);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "derive-key", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *from = find_keyword (mtext, VV_KEYFROM, 1);
                        char *fromlen = find_keyword (mtext, VV_KEYFROMLENGTH, 1);
                        char *digest = find_keyword (mtext, VV_KEYDIGEST, 1);
                        char *salt = find_keyword (mtext, VV_KEYSALT, 1);
                        char *salt_len = find_keyword (mtext, VV_KEYSALTLENGTH, 1);
                        char *iterations = find_keyword (mtext, VV_KEYITERATIONS, 1);
                        char *length = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *binary = find_keyword (mtext, VV_KEYBINARY, 1);

                        carve_statement (&length, "derive-key", VV_KEYLENGTH, 1, 1);
                        carve_statement (&from, "derive-key", VV_KEYFROM, 1, 1);
                        carve_statement (&fromlen, "derive-key", VV_KEYFROMLENGTH, 0, 1);
                        carve_statement (&digest, "derive-key", VV_KEYDIGEST, 0, 1);
                        carve_statement (&salt, "derive-key", VV_KEYSALT, 0, 1);
                        carve_statement (&salt_len, "derive-key" , VV_KEYSALTLENGTH, 0, 1);
                        carve_statement (&iterations, "derive-key", VV_KEYITERATIONS, 0, 1);
                        carve_statement (&binary, "derive-key", VV_KEYBINARY, 0, 2);

                        define_statement (&mtext, VV_DEFSTRING);

                        char *binaryc = opt_clause(binary, "true", "false");
                        oprintf ("%s=vely_derive_key( %s, %s, %s, %s, %s, %s, %s, %s );\n", mtext, from, fromlen==NULL?"-1":fromlen, digest==NULL?VV_DEF_DIGEST:digest, iterations==NULL?"-1":iterations, salt==NULL?"NULL":salt, salt_len==NULL?"0":salt_len, length, binaryc);
                        vely_free(binaryc);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "hash-string", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *digest = find_keyword (mtext, VV_KEYDIGEST, 1);
                        char *binary = find_keyword (mtext, VV_KEYBINARY, 1);
                        char *olen = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);

                        carve_statement (&to, "hash-string", VV_KEYTO, 1, 1);
                        carve_statement (&digest, "hash-string", VV_KEYDIGEST, 0, 1);
                        carve_statement (&binary, "hash-string", VV_KEYBINARY, 0, 2);
                        carve_statement (&olen, "hash-string", VV_KEYOUTPUTLENGTH, 0, 1);

                        define_statement (&to, VV_DEFSTRING);
                        define_statement (&olen, VV_DEFNUM);

                        char *binaryc = opt_clause(binary, "true", "false");
                        oprintf ("%s=vely_hash_data (%s, %s, %s, %s%s%s);\n", to, mtext, digest==NULL?VV_DEF_DIGEST:digest, binaryc, olen==NULL?"NULL":"&(",olen==NULL?"":olen,olen==NULL?"":")");
                        vely_free(binaryc);



                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "decode-base64", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *olen = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *ilen = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        carve_statement (&olen, "decode-base64", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&ilen, "decode-base64", VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&to, "decode-base64", VV_KEYTO, 1, 1);

                        define_statement (&olen, VV_DEFNUM);
                        define_statement (&to, VV_DEFSTRING);

                        oprintf ("vely_b64_decode ((char*)(%s), %s, &((%s)), %s%s%s);\n", mtext, ilen==NULL?"-1":ilen,to, olen==NULL?"NULL":"&(",olen==NULL?"":olen,olen==NULL?"":")");



                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "encode-base64", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *olen = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *ilen = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        carve_statement (&olen, "encode-base64", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&ilen, "encode-base64", VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&to, "encode-base64", VV_KEYTO, 1, 1);

                        define_statement (&olen, VV_DEFNUM);
                        define_statement (&to, VV_DEFSTRING);

                        oprintf ("vely_b64_encode ((char*)(%s), %s, &(%s), %s%s%s);\n", mtext, ilen==NULL?"-1":ilen,to, olen==NULL?"NULL":"&(",olen==NULL?"":olen,olen==NULL?"":")");



                        continue;
                    }

                    else if ((newI=recog_statement(line, i, "encode-hex", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *olen = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *ilen = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *pref = find_keyword (mtext, VV_KEYPREFIX, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        carve_statement (&olen, "encode-hex", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&ilen, "encode-hex", VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&pref, "encode-hex", VV_KEYPREFIX, 0, 1);
                        carve_statement (&to, "encode-hex", VV_KEYTO, 1, 1);

                        define_statement (&olen, VV_DEFNUM);
                        define_statement (&to, VV_DEFSTRING);

                        oprintf ("vely_bin2hex((char*)(%s), &(%s), %s, %s%s%s, %s);\n", mtext, to, ilen==NULL?"-1":ilen, olen==NULL?"NULL":"&(",olen==NULL?"":olen,olen==NULL?"":")", pref==NULL?"NULL":pref);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "decode-hex", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *olen = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *ilen = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        carve_statement (&olen, "decode-hex", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&ilen, "decode-hex", VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&to, "decode-hex", VV_KEYTO, 1, 1);

                        define_statement (&olen, VV_DEFNUM);
                        define_statement (&to, VV_DEFSTRING);

                        oprintf ("vely_hex2bin ((char*)(%s), &(%s), %s, %s%s%s);\n", mtext, to, ilen==NULL?"-1":ilen, olen==NULL?"NULL":"&(",olen==NULL?"":olen,olen==NULL?"":")");

                        continue;
                    }

                    else if ((newI=recog_statement(line, i, "decode-url", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
                     (newI1=recog_statement(line, i, "decode-web", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI+newI1;

                        num is_web=(newI1 != 0 ? 1:0);
                        char *outlength = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *inlength = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);

                        carve_statement (&outlength, is_web==1?"decode-web":"decode-url", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&inlength, is_web==1?"decode-web":"decode-url", VV_KEYINPUTLENGTH, 0, 1);

                        define_statement (&outlength, VV_DEFNUM);
                        if (inlength == NULL) inlength="-1";

                        oprintf ("%s%svely_decode (%s, %s, %s);\n", outlength==NULL?"":outlength, outlength==NULL?"":"=", is_web==1?"VV_WEB":"VV_URL", mtext, inlength);



                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "encode-url", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
                     (newI1=recog_statement(line, i, "encode-web", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI+newI1;

                        num is_web=(newI1 != 0 ? 1:0);
                        char *outlength = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *inlength = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        carve_statement (&inlength, is_web==1?"encode-web":"encode-url", VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&outlength, is_web==1?"encode-web":"encode-url", VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&to, is_web==1?"encode-web":"encode-url", VV_KEYTO, 1, 1);

                        define_statement (&outlength, VV_DEFNUM);
                        define_statement (&to, VV_DEFSTRING);

                        if (inlength == NULL) inlength = "-1";

                        oprintf ("%s%svely_encode (%s, %s, %s, &(%s));\n", outlength==NULL?"":outlength, outlength==NULL?"":"=", is_web==1?"VV_WEB":"VV_URL", mtext, inlength, to);



                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "trim-string", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // trim-string str [length [define] len]
                        char *length = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *result = find_keyword (mtext, VV_KEYRESULT, 1);

                        carve_statement (&length, "trim-string", VV_KEYLENGTH, 0, 1);
                        carve_statement (&result, "trim-string", VV_KEYRESULT, 0, 1);

                        define_statement (&length, VV_DEFNUM);
                        define_statement (&result, VV_DEFSTRING);

                        oprintf ("num _vv_trim_len%lld=strlen (%s);\n", trim_count, mtext);
                        if (result == NULL) oprintf ("vely_trim (%s, &_vv_trim_len%lld);\n", mtext, trim_count);
                        else oprintf ("%s=vely_trim_ptr (%s, &_vv_trim_len%lld);\n", result, mtext, trim_count);
                        if (length != NULL)
                        {
                            oprintf ("%s=_vv_trim_len%lld;\n", length, trim_count);
                        }

                        trim_count++;


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "encrypt-data", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
                        (newI1=recog_statement(line, i, "decrypt-data", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI+newI1;
                        bool enc = (newI!=0); // is it encrypt (otherwise decrypt)

                        // Example:
                        // encrypt-data data [input-length l] output-length [define] ol password p [salt s] to [define] res [binary]

                        char *password = find_keyword (mtext, VV_KEYPASSWORD, 1);
                        char *inlength = find_keyword (mtext, VV_KEYINPUTLENGTH, 1);
                        char *outlength = find_keyword (mtext, VV_KEYOUTPUTLENGTH, 1);
                        char *salt = find_keyword (mtext, VV_KEYSALT, 1);
                        char *salt_len = find_keyword (mtext, VV_KEYSALTLENGTH, 1);
                        char *iv = find_keyword (mtext, VV_KEYINITVECTOR, 1);
                        char *iterations = find_keyword (mtext, VV_KEYITERATIONS, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *binary = find_keyword (mtext, VV_KEYBINARY, 1);
                        char *cipher = find_keyword (mtext, VV_KEYCIPHER, 1);
                        char *digest = find_keyword (mtext, VV_KEYDIGEST, 1);
                        char *cache = find_keyword (mtext, VV_KEYCACHE, 1);
                        char *ccache = find_keyword (mtext, VV_KEYCLEARCACHE, 1);

                        carve_statement (&password, enc?"encrypt-data":"decrypt-data", VV_KEYPASSWORD, 1, 1);
                        carve_statement (&inlength, enc?"encrypt-data":"decrypt-data" , VV_KEYINPUTLENGTH, 0, 1);
                        carve_statement (&outlength, enc?"encrypt-data":"decrypt-data" , VV_KEYOUTPUTLENGTH, 0, 1);
                        carve_statement (&iv, enc?"encrypt-data":"decrypt-data", VV_KEYINITVECTOR, 0, 1);
                        carve_statement (&salt, enc?"encrypt-data":"decrypt-data", VV_KEYSALT, 0, 1);
                        carve_statement (&salt_len, enc?"encrypt-data":"decrypt-data" , VV_KEYSALTLENGTH, 0, 1);
                        carve_statement (&iterations, enc?"encrypt-data":"decrypt-data", VV_KEYITERATIONS, 0, 1);
                        carve_statement (&to, enc?"encrypt-data":"decrypt-data" , VV_KEYTO, 1, 1);
                        carve_statement (&binary, enc?"encrypt-data":"decrypt-data", VV_KEYBINARY, 0, 2);
                        carve_statement (&cipher, enc?"encrypt-data":"decrypt-data", VV_KEYCIPHER, 0, 1);
                        carve_statement (&digest, enc?"encrypt-data":"decrypt-data", VV_KEYDIGEST, 0, 1);
                        carve_statement (&cache, enc?"encrypt-data":"decrypt-data", VV_KEYCACHE, 0, 0);
                        carve_statement (&ccache, enc?"encrypt-data":"decrypt-data", VV_KEYCLEARCACHE, 0, 1);

                        define_statement (&to, VV_DEFSTRING);
                        define_statement (&outlength, VV_DEFNUM);

                        char *binaryc = opt_clause(binary, "1", "0");
                        if (ccache != NULL && cache == NULL) _vely_report_error( "clear-cache cannot be used without cache in encrypt/decrypt-data statement");

                        if (iv == NULL)
                        {
                            iv="NULL";
                            if (cache != NULL) _vely_report_error( "cache cannot be used if init-vector is omitted in encrypt/decrypt-data statement");
                        } 
                        if (salt == NULL)
                        {
                            if (salt_len != NULL) _vely_report_error( "cannot specify salt-len without salt in encrypt/decrypt-data statement");
                            salt="NULL";
                        }

                        char *to_crypt = mtext;
                        
                        oprintf ("%sEVP_CIPHER_CTX *_vv_e_ctx%lld = NULL;\n", cache!=NULL?"static ":"", encrypt_count);
                        if (ccache != NULL) 
                        {
                            // if clear-cache is true, then first check if there's existing encryption context, if so, clear it
                            // then set context to NULL so it's created again.
                            oprintf ("if ((%s)) {if (_vv_e_ctx%lld != NULL) EVP_CIPHER_CTX_free(_vv_e_ctx%lld); _vv_e_ctx%lld = NULL;}\n", ccache, encrypt_count, encrypt_count, encrypt_count);
                        }
                        oprintf ("if (_vv_e_ctx%lld == NULL) { _vv_e_ctx%lld = EVP_CIPHER_CTX_new(); vely_get_enc_key(%s, %s, %s, %s, %s?_vv_e_ctx%lld:NULL, %s?_vv_e_ctx%lld:NULL, %s, %s);}\n", encrypt_count, encrypt_count, password, salt, salt_len == NULL ? "0":salt_len, iterations==NULL?"-1":iterations, enc?"true":"false", encrypt_count, (!enc)?"true":"false", encrypt_count, cipher==NULL?VV_DEF_CIPHER:cipher, digest==NULL?VV_DEF_DIGEST:digest);
                        // regardless of whether input-length is used or not, we have it set
                        if (inlength != NULL)
                        {
                            oprintf ("num _vv_encrypt_count%lld = %s;\n", encrypt_count, inlength);
                        }
                        else
                        {
                            oprintf ("num _vv_encrypt_count%lld = strlen(%s);\n", encrypt_count, to_crypt);
                        }
                        oprintf ("%s = %s(_vv_e_ctx%lld, (unsigned char*)(%s), &_vv_encrypt_count%lld, %s, (unsigned char*)(%s));\n", to, enc?"vely_encrypt":"vely_decrypt", encrypt_count, to_crypt, encrypt_count, binaryc, iv==NULL?"NULL":iv);
                        // regardless of whether output-length is used or not, we have it set
                        if (outlength != NULL)
                        {
                            oprintf ("%s=_vv_encrypt_count%lld;\n", outlength, encrypt_count);
                        }
                        if (cache == NULL) oprintf ("EVP_CIPHER_CTX_free(_vv_e_ctx%lld);\n", encrypt_count);
                        encrypt_count++;
                        vely_free(binaryc);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "stat-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *path = find_keyword (mtext, VV_KEYPATH0, 1);
                        char *basename = find_keyword (mtext, VV_KEYNAME0, 1);
                        char *type = find_keyword (mtext, VV_KEYTYPE0, 1);
                        char *size = find_keyword (mtext, VV_KEYSIZE0, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        carve_statement (&to, "stat-file", VV_KEYTO, 1, 1);
                        carve_statement (&path, "stat-file", VV_KEYPATH0, 0, 0);
                        carve_statement (&basename, "stat-file", VV_KEYNAME0, 0, 0);
                        carve_statement (&type, "stat-file", VV_KEYTYPE0, 0, 0);
                        carve_statement (&size, "stat-file", VV_KEYSIZE0, 0, 0);

                        num tot_opt = (path!=NULL?1:0)+(basename!=NULL?1:0)+(type!=NULL?1:0)+(size!=NULL?1:0);
                        if (tot_opt != 1)
                        {
                            _vely_report_error( "Exactly one option must be in stat-file statement");
                        }

                        // result can be defined
                        if (size != NULL || type != NULL ) 
                            define_statement (&to, VV_DEFNUM);
                        else 
                            define_statement (&to, VV_DEFSTRING);


                        if (size != NULL) oprintf ("%s=vely_get_file_size (%s);\n", to, mtext);
                        if (type != NULL) oprintf ("%s=vely_file_type (%s);\n", to, mtext);
                        // basename and path must use strdup on argument because it may be altered
                        // also strdup the result because it may be overwritten by subsequent calls
                        if (basename != NULL) oprintf ("%s=vely_basename (%s);\n", to, mtext);
                        if (path != NULL) oprintf ("%s=vely_realpath (%s);\n", to, mtext);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "read-json", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *val = find_keyword (mtext, VV_KEYVALUE, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *key = find_keyword (mtext, VV_KEYKEY, 1);
                        char *type = find_keyword (mtext, VV_KEYTYPE, 1);
                        char *trav = find_keyword (mtext, VV_KEYTRAVERSE, 1);
                        char *beg = NULL;
                        if (trav != NULL)
                        {
                            beg = find_keyword (mtext, VV_KEYBEGIN, 1);
                        }

                        if (trav != NULL)
                        {
                            carve_statement (&key, "read-json", VV_KEYKEY, 0, 1);
                            carve_statement (&val, "read-json", VV_KEYVALUE, 0, 1);
                            carve_statement (&beg, "read-json", VV_KEYBEGIN, 0, 0);
                            carve_statement (&trav, "read-json", VV_KEYTRAVERSE, 0, 0);
                        }
                        else
                        {
                            carve_statement (&key, "read-json", VV_KEYKEY, 1, 1);
                            carve_statement (&val, "read-json", VV_KEYVALUE, 1, 1);
                        }
                        carve_statement (&type, "read-json", VV_KEYTYPE, 0, 1);
                        carve_statement (&status, "read-json", VV_KEYSTATUS, 0, 1);


                        if (trav != NULL) define_statement (&key, VV_DEFSTRING); 
                        define_statement (&status, VV_DEFNUM);
                        define_statement (&type, VV_DEFNUM);
                        define_statement (&val, VV_DEFSTRING);

                        if (trav == NULL) 
                        {
                            // if key is a "".""..., i.e. string literal, treat all of it as string
                            char *fkey = NULL;
                            // trim key, must be on both sides to get the right result ".." 
                            num lkey = strlen (key);
                            key = vely_trim_ptr(key,  &lkey);
                            // if string, make it a proper string with escaped "
                            if (key[0] == '"')
                            {
                                num fsize = 2*strlen(key)+1+2; // enough for many quotes inside
                                fkey = vely_malloc(fsize);
                                strcpy (fkey+1, key);
                                vely_replace_string (fkey+1, fsize-1, "\"", "\\\"", 1, NULL, 0);
                                fkey[0] = '"';
                                strcpy (fkey+strlen(fkey), "\"");
                            } else fkey = key;
                            oprintf ("%s%svely_read_json (%s, %s, &(%s), %s%s%s);\n", status == NULL ? "":status, status == NULL ? "":"=", mtext, fkey, val, type==NULL?"":"&(", type==NULL?"NULL":type, type==NULL?"":")");
                        }
                        else
                        {
                            if (beg != NULL) 
                            {
                                oprintf ("vely_begin_json (%s);\n", mtext);
                            }
                            if ((key == NULL && val != NULL) || (key != NULL && val == NULL)) _vely_report_error( "'key' and 'value' must both be in read-json statement");
                            if (key != NULL) oprintf ("%s%svely_next_json (%s, &(%s), &(%s), %s%s%s);\n", status == NULL ? "":status, status == NULL ? "":"=", mtext, key, val, type==NULL?"":"&(", type==NULL?"NULL":type, type==NULL?"":")");
                        }

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "utf8-json", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *len = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *errt = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        carve_statement (&len, "utf8-json", VV_KEYLENGTH, 0, 1);
                        carve_statement (&status, "utf8-json", VV_KEYSTATUS, 0, 1);
                        carve_statement (&errt, "utf8-json", VV_KEYERRORTEXT, 0, 1);
                        carve_statement (&to, "utf8-json", VV_KEYTO, 0, 1);
                        define_statement (&status, VV_DEFNUM);
                        define_statement (&errt, VV_DEFSTRING);
                        define_statement (&to, VV_DEFSTRING);

                        oprintf ("char *_vv_errt%lld = NULL;\n", code_json);
                        oprintf ("num _vv_res%lld = vely_utf8_to_json (%s, %s, &(%s), &_vv_errt%lld);\n", code_json, mtext, len == NULL ? "-1":len, to, code_json);
                        if (status != NULL) oprintf ("if (_vv_res%lld == -1) {VERR0;%s=VV_ERR_UTF8;} else %s=_vv_res%lld;\n", code_json, status, status, code_json); else oprintf("VV_UNUSED(_vv_res%lld);\n", code_json);
                        if (errt != NULL) oprintf ("%s = _vv_errt%lld;\n", errt, code_json); 

                        code_json++;

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "json-utf8", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *errt = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        carve_statement (&status, "json-utf8", VV_KEYSTATUS, 0, 1);
                        carve_statement (&errt, "json-utf8", VV_KEYERRORTEXT, 0, 1);
                        define_statement (&status, VV_DEFNUM);
                        define_statement (&errt, VV_DEFSTRING);

                        oprintf ("char *_vv_errt%lld = NULL;\n", code_json);
                        oprintf ("char *_vv_res%lld = vely_json_to_utf8 (%s, 0, &_vv_errt%lld, 1);\n", code_json, mtext, code_json);
                        if (status != NULL) oprintf ("if (_vv_res%lld != NULL) %s=VV_OKAY; else {VERR0;%s=VV_ERR_UTF8;}\n", code_json, status, status); else oprintf("VV_UNUSED(_vv_res%lld);\n", code_json);
                        if (errt != NULL) oprintf ("%s = _vv_errt%lld;\n", errt, code_json);


                        code_json++;

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "delete-json", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        oprintf ("vely_del_json (%s);\n", mtext);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "new-json", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *nodec = find_keyword (mtext, VV_KEYNOENCODE, 1);
                        char *maxhash = find_keyword (mtext, VV_KEYMAXHASHSIZE, 1);
                        char *from = find_keyword (mtext, VV_KEYFROM, 1);
                        char *errp = find_keyword (mtext, VV_KEYERRORPOSITION, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *errt = find_keyword (mtext, VV_KEYERRORTEXT, 1);
                        char *len = find_keyword (mtext, VV_KEYLENGTH, 1);

                        carve_statement (&nodec, "new-json", VV_KEYNOENCODE, 0, 0);
                        carve_statement (&status, "new-json", VV_KEYSTATUS, 0, 1);
                        carve_statement (&errp, "new-json", VV_KEYERRORPOSITION, 0, 1);
                        carve_statement (&errt, "new-json", VV_KEYERRORTEXT, 0, 1);
                        carve_statement (&len, "new-json", VV_KEYLENGTH, 0, 1);
                        carve_statement (&from, "new-json", VV_KEYFROM, 1, 1);
                        carve_statement (&maxhash, "new-json", VV_KEYMAXHASHSIZE, 0, 1);

                        define_statement (&errt, VV_DEFSTRING);
                        define_statement (&status, VV_DEFNUM);
                        define_statement (&errp, VV_DEFNUM);
                        define_statement (&mtext, VV_DEFJSON);

                        //
                        // Look for each option and collect relevant info
                        //
                        oprintf ("vely_set_json (&(%s), %s);\n", mtext, maxhash == NULL ? "-1" : maxhash);
                        oprintf ("num vely_json_status_%lld = %s%svely_json_new (%s, NULL, (%s), %s);\n", json_id, errp == NULL ? "":errp, errp == NULL ? "":"=", from, len == NULL ? "-1" : len, nodec == NULL?"1":"0");
                        if (status != NULL)
                        {
                            oprintf ("VERR0; %s = (vely_json_status_%lld == -1 ? VV_OKAY : VV_ERR_JSON);\n", status, json_id);
                        }
                        else
                        {
                            oprintf ("VV_UNUSED(vely_json_status_%lld);\n", json_id);
                        }
                        if (errt != NULL)
                        {
                            oprintf ("%s = vely_json_err();\n", errt);
                        }
                        json_id++;

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "rename-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // rename-file "/home/vely/files/somefile.jpg" to "file..." status st
                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&status, "rename-file", VV_KEYSTATUS, 0, 1);
                        carve_statement (&to, "rename-file", VV_KEYTO, 1, 1);

                        define_statement (&status, VV_DEFNUM);

                        //
                        // Look for each option and collect relevant info
                        //
                        if (status == NULL)
                        {
                            oprintf ("rename ((%s), (%s));VERR;\n", mtext, to);
                        }
                        else
                        {
                            oprintf ("if (((%s) = rename ((%s), (%s))) != 0) {VERR;VV_TRACE (\"Cannot rename file [%%s] to [%%s], error [%%s]\",%s,%s,strerror(errno));%s=VV_ERR_RENAME;} else %s=VV_OKAY;\n", status, mtext, to, mtext,to, status, status);
                        }


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "delete-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // delete-file "/home/vely/files/somefile.jpg" status st
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&status, "delete-file", VV_KEYSTATUS, 0, 1);

                        define_statement (&status, VV_DEFNUM);

                        //
                        // Look for each option and collect relevant info
                        //
                        if (status == NULL)
                        {
                            oprintf ("unlink (%s);VERR;\n", mtext);
                        }
                        else
                        {
                            oprintf ("if (unlink (%s) != 0  && errno != ENOENT) {VERR;VV_TRACE (\"Cannot delete file [%%s], error [%%s]\",%s, strerror(errno)); %s=VV_ERR_DELETE;} else %s=VV_OKAY;\n", mtext,mtext, status, status);
                        }


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "call-web", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        //
                        // Look for each option and collect relevant info
                        //
                        char *resp = find_keyword (mtext, VV_KEYRESPONSE, 1);
                        char *head = find_keyword (mtext, VV_KEYRESPONSEHEADERS, 1);
                        char *rhead = find_keyword (mtext, VV_KEYREQUESTHEADERS, 1);
                        char *resp_code = find_keyword (mtext, VV_KEYRESPONSECODE, 1);
                        char *len = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *err = find_keyword (mtext, VV_KEYERROR, 1);
                        char *cert = find_keyword (mtext, VV_KEYCERT, 1);
                        char *nocert = find_keyword (mtext, VV_KEYNOCERT, 1);
                        char *cookiejar = find_keyword (mtext, VV_KEYCOOKIEJAR, 1);
                        char *timeout = find_keyword (mtext, VV_KEYTIMEOUT, 1);
                        char *body = find_keyword (mtext, VV_KEYREQUESTBODY, 1);
                        char *method = find_keyword (mtext, VV_KEYMETHOD, 1);

                        carve_statement (&body, "call-web", VV_KEYREQUESTBODY, 0, 1);
                        carve_statement (&timeout, "call-web", VV_KEYTIMEOUT, 0, 1);
                        carve_statement (&method, "call-web", VV_KEYMETHOD, 0, 1);
                        carve_statement (&resp, "call-web", VV_KEYRESPONSE, 1, 1);
                        carve_statement (&head, "call-web", VV_KEYRESPONSEHEADERS, 0, 1);
                        carve_statement (&rhead, "call-web", VV_KEYREQUESTHEADERS, 0, 1);
                        carve_statement (&resp_code, "call-web", VV_KEYRESPONSECODE, 0, 1);
                        carve_statement (&len, "call-web", VV_KEYSTATUS, 0, 1);
                        carve_statement (&err, "call-web", VV_KEYERROR, 0, 1);
                        carve_statement (&cert, "call-web", VV_KEYCERT, 0, 1);
                        carve_statement (&nocert, "call-web", VV_KEYNOCERT, 0, 0);
                        carve_statement (&cookiejar, "call-web", VV_KEYCOOKIEJAR, 0, 1);

                        // process body clause after ward because it is done within a copy of carved out 'body' above
                        char *files = NULL;
                        char *fields = NULL;
                        char *content = NULL;
                        if (body != NULL)
                        {
                            char *mbody = vely_strdup (body);
                            content = find_keyword (mbody, VV_KEYCONTENT, 1);
                            files = find_keyword (mbody, VV_KEYFILES, 1);
                            fields = find_keyword (mbody, VV_KEYFIELDS, 1);
                            carve_statement (&content, "call-web", VV_KEYCONTENT, 0, 1);
                            carve_statement (&files, "call-web", VV_KEYFILES, 0, 1);
                            carve_statement (&fields, "call-web", VV_KEYFIELDS, 0, 1);
                        }

                        //defines
                        define_statement (&len, VV_DEFNUM);
                        define_statement (&resp, VV_DEFSTRING);
                        define_statement (&head, VV_DEFSTRING);
                        define_statement (&resp_code, VV_DEFNUM);
                        define_statement (&err, VV_DEFSTRING);

                        char *req_header = NULL;
                        char req_header_ptr[200];
                        char* clen = NULL;
                        char* ctype = NULL;
                        if (rhead != NULL)
                        {
                            // VV_HEADER_FILE/PAGE doesn't matter, cache is not used with request header
                            req_header = make_default_header(VV_HEADER_FILE, http_header_count, 1);
                            snprintf (req_header_ptr, sizeof(req_header_ptr), "&%s", req_header);
                            process_http_header ("call-web", rhead, req_header, http_header_count, 1, &clen, &ctype); // VV_KEYCONTENTTYPE VV_KEYCUSTOM VV_KEYCONTENTLENGTH
                            http_header_count++;
                        }
                        if (content != NULL && ctype == NULL) 
                        {
                            _vely_report_error( "'content-type' must be specified with 'content' subclause in 'request-body' in call-web statement");
                        }
                        if (content == NULL && clen != NULL && body != NULL) 
                        {
                            _vely_report_error( "'content-length' cannot be specified without 'content' subclause in 'request-body' in call-web statement");
                        }
                        if (content == NULL && ctype != NULL && body != NULL) 
                        {
                            _vely_report_error( "'content-type' cannot be specified without 'content' subclause in 'request-body' in call-web statement");
                        }
                        if ((files == NULL && fields == NULL && content == NULL ) && body != NULL) 
                        {
                            _vely_report_error( "either 'content' or 'files'/'fields' must be specified with 'body' clause in  call-web statement");
                        }
                        if ((files != NULL || fields != NULL)) 
                        {
                            if (ctype == NULL) ctype = "multipart/form-data"; // when files or forms used, always this
                        }
                        if ((files != NULL || fields != NULL) && (content != NULL)) 
                        {
                            _vely_report_error( "you can specify either 'content' or 'files'/'fields', but not both in  call-web statement");
                        }
                        if (cert != NULL && nocert != NULL)
                        {
                            _vely_report_error( "cert and no-cert cannot coexist in the same call-web statement");
                        }

                        // cert cannot be defined, must exist and be filled with the location of .ca file!

                        // store fields and files (with body) arrays
                        char fields_var[100];
                        char files_var[100];
                        if (fields != NULL)
                        {
                            snprintf (fields_var, sizeof (fields_var), "_vv_fields_arr%lld", total_body);
                            outargs(fields, fields_var, "char *", 0, 1);
                        }
                        if (files != NULL)
                        {
                            snprintf (files_var, sizeof (files_var), "_vv_files_arr%lld", total_body);
                            outargs(files, files_var, "char *", 0, 1);
                        }

                        total_body++;

                        oprintf ("%s%svely_post_url_with_response(%s, &(%s), %s%s%s, %s%s%s, %s, %s, %s%s%s, %s, %s, %s, %s, %s, %s, %s, %s);\n", len==NULL?"":len,len==NULL?"":"=",
                            mtext, resp, head==NULL ? "":"&(",head==NULL ? "NULL":head, head==NULL ? "":")", err==NULL ? "":"&(",err==NULL ? "NULL":err, err==NULL ? "":")", nocert != NULL ? "NULL" : (cert != NULL ? cert : "\"\""), cookiejar == NULL ? "NULL":cookiejar, resp_code==NULL ? "":"&(",resp_code==NULL ? "NULL":resp_code, resp_code==NULL ? "":")", timeout==NULL ? "120":timeout, body == NULL ? "0":"1", fields == NULL ? "NULL":fields_var, files == NULL ? "NULL":files_var, req_header == NULL ? "NULL":req_header_ptr, method == NULL ? "NULL" :method, content==NULL?"NULL":content, clen==NULL?"-1":clen);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "new-server", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        //
                        // Look for each option and collect relevant info
                        //
                        char *server = find_keyword (mtext, VV_KEYLOCATION, 1);
                        char *method = find_keyword (mtext, VV_KEYMETHOD, 1);
                        char *apath = find_keyword (mtext, VV_KEYAPPPATH, 1);
                        char *req = find_keyword (mtext, VV_KEYREQUESTPATH, 1);
                        char *body = find_keyword (mtext, VV_KEYREQUESTBODY, 1);
                        char *upay = find_keyword (mtext, VV_KEYURLPAYLOAD, 1);
                        char *timeout = find_keyword (mtext, VV_KEYTIMEOUT, 1);
                        char *env = find_keyword (mtext, VV_KEYENVIRONMENT, 1);

                        carve_statement (&server, "new-server", VV_KEYLOCATION, 1, 1);
                        carve_statement (&method, "new-server", VV_KEYMETHOD, 1, 1);
                        carve_statement (&apath, "new-server", VV_KEYAPPPATH, 1, 1);
                        carve_statement (&req, "new-server", VV_KEYREQUESTPATH, 1, 1);
                        carve_statement (&body, "new-server", VV_KEYREQUESTBODY, 0, 1);
                        carve_statement (&upay, "new-server", VV_KEYURLPAYLOAD, 0, 1);
                        carve_statement (&timeout, "new-server", VV_KEYTIMEOUT, 0, 1);
                        carve_statement (&env, "new-server", VV_KEYENVIRONMENT, 0, 1);

                        // process body clause after ward because it is done within a copy of carved out 'body' above
                        char *ctype = NULL;
                        char *content = NULL;
                        char *clen = NULL;
                        char *mbody = NULL;
                        if (body != NULL)
                        {
                            mbody = vely_strdup (body);
                            content = find_keyword (mbody, VV_KEYCONTENT, 1);
                            clen = find_keyword (mbody, VV_KEYCONTENTLENGTH, 1);
                            ctype = find_keyword (mbody, VV_KEYCONTENTTYPE, 1);
                            carve_statement (&content, "new-server", VV_KEYCONTENT, 0, 1);
                            carve_statement (&clen, "new-server", VV_KEYCONTENTLENGTH, 0, 1);
                            carve_statement (&ctype, "new-server", VV_KEYCONTENTTYPE, 0, 1);
                        }

                        if (server == NULL) _vely_report_error( "'server' must be specified in new-server statement");
                        if (method == NULL) _vely_report_error( "'request-method' must be specified in new-server statement");
                        if (apath == NULL) _vely_report_error( "'app-path' must be specified in new-server statement");
                        if (req == NULL) _vely_report_error( "'request-path' must be specified in new-server statement");
                        if (body == NULL && ctype != NULL) _vely_report_error( "'content-type' cannot be specified without 'request-body' in new-server statement");
                        if (body == NULL && clen != NULL) _vely_report_error( "'content-length' cannot be specified without 'request-body' in new-server statement");
                        if (body != NULL && ctype == NULL) _vely_report_error( "'content-type' must be specified for 'request-body' in new-server statement");
                        if (body != NULL && content == NULL) _vely_report_error( "'content' must be specified for 'request-body' in new-server statement");


                        char env_var[100];
                        snprintf (env_var, sizeof (env_var), "_vv_fc_env_arr%lld", total_fcgi_arr);
                        if (env != NULL) outargs(env, env_var, "char *", 0, 1);

                        // define for fcgi thread
                        define_statement (&mtext, VV_DEFFCGI);
                        oprintf ("vv_set_fcgi (&(%s), %s, %s, %s, %s, %s, %s, %s, %s, %s, %s);\n", mtext, server, method, apath, req, upay==NULL?"NULL":upay,ctype==NULL?"NULL":ctype, body==NULL?"NULL":content, clen==NULL?"0":clen, timeout==NULL?"0":timeout, env==NULL?"NULL":env_var);

                        total_fcgi_arr++;
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "read-server", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *data = find_keyword (mtext, VV_KEYDATA, 1);
                        char *error = find_keyword (mtext, VV_KEYERROR, 1);
                        char *ldata = find_keyword (mtext, VV_KEYDATALENGTH, 1);
                        char *lerror = find_keyword (mtext, VV_KEYERRORLENGTH, 1);
                        char *rcode = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *rmsg = find_keyword (mtext, VV_KEYSTATUSTEXT, 1);
                        char *rstatus = find_keyword (mtext, VV_KEYREQUESTSTATUS, 1);

                        carve_statement (&data, "read-server", VV_KEYDATA, 0, 1);
                        carve_statement (&error, "read-server", VV_KEYERROR, 0, 1);
                        carve_statement (&ldata, "read-server", VV_KEYDATALENGTH, 0, 1);
                        carve_statement (&lerror, "read-server", VV_KEYERRORLENGTH, 0, 1);
                        carve_statement (&rcode, "read-server", VV_KEYSTATUS, 0, 1);
                        carve_statement (&rmsg, "read-server", VV_KEYSTATUSTEXT, 0, 1);
                        carve_statement (&rstatus, "read-server", VV_KEYREQUESTSTATUS, 0, 1);
                        
                        define_statement (&data, VV_DEFSTRING);
                        define_statement (&error, VV_DEFSTRING);
                        define_statement (&ldata, VV_DEFNUM);
                        define_statement (&lerror, VV_DEFNUM);
                        define_statement (&rcode, VV_DEFNUM);
                        define_statement (&rstatus, VV_DEFNUM);
                        define_statement (&rmsg, VV_DEFSTRING);

                        if (data == NULL && error == NULL && ldata == NULL && lerror == NULL && rcode == NULL && rstatus == NULL) _vely_report_error( "at least one clause must be specified in new-server statement");
                        if (data != NULL) oprintf ("%s = vv_fc_data(%s);\n", data, mtext);
                        if (error != NULL) oprintf ("%s = vv_fc_error(%s);\n", error, mtext);
                        if (ldata != NULL) oprintf ("%s = (%s)->data_len;\n", ldata, mtext);
                        if (lerror != NULL) oprintf ("%s = (%s)->error_len;\n", lerror, mtext);
                        if (rcode != NULL) oprintf ("%s = (%s)->return_code;\n", rcode, mtext);
                        if (rmsg != NULL) oprintf ("%s = (%s)->errm;\n", rmsg, mtext);
                        if (rstatus != NULL) oprintf ("%s = (%s)->req_status;\n", rstatus, mtext);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "delete-server", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;


                        oprintf ("vv_fc_delete (%s);\n", mtext);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "call-server", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *st = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *finok = find_keyword (mtext, VV_KEYFINISHEDOKAY, 1);
                        char *count = find_keyword (mtext, VV_KEYARRAYCOUNT, 1);
                        char *started = find_keyword (mtext, VV_KEYSTARTED, 1);

                        carve_statement (&st, "call-server", VV_KEYSTATUS, 0, 1);
                        carve_statement (&finok, "call-server", VV_KEYFINISHEDOKAY, 0, 1);
                        carve_statement (&count, "call-server", VV_KEYARRAYCOUNT, 0, 1);
                        carve_statement (&started, "call-server", VV_KEYSTARTED, 0, 1);

                        define_statement (&st, VV_DEFNUM);
                        define_statement (&finok, VV_DEFNUM);
                        define_statement (&started, VV_DEFNUM);

                        char req_var[100];
                        char totreq_s[30];
                        if (count == NULL)
                        {
                            // this is a list of fcgi calls, otherwise var is mtext, count is the length of array
                            snprintf (req_var, sizeof (req_var), "_vv_fc_req_arr%lld", total_fcgi_arr);
                            num totreq = outargs(mtext, req_var, "vv_fc *", 0, 0);
                            snprintf (totreq_s, sizeof(totreq_s), "%lld", totreq);
                        }

                        oprintf ("%s%svv_call_fcgi (%s, %s, %s%s%s, %s%s%s);\n", st==NULL?"":st, st==NULL?"":"=",count!=NULL?mtext:req_var, count != NULL?count:totreq_s, finok==NULL?"":"&(", finok==NULL?"NULL":finok, finok==NULL?"":")", started==NULL?"":"&(", started==NULL?"NULL":started, started==NULL?"":")" );

                        total_fcgi_arr++;
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "uniq-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *temp = find_keyword (mtext, VV_KEYTEMPORARY, 1);

                        //
                        // After all options positions have been found, we must get the options 
                        // for ALL of them
                        //
                        carve_statement (&temp, "uniq-file", VV_KEYTEMPORARY, 0, 0);

                        //defines
                        define_statement (&mtext, VV_DEFSTRING);

                        //generate code
                        oprintf ("{FILE *f = vely_make_document (&(%s), %d); fclose (f);}\n", mtext, temp!=NULL?1:0);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "get-time", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // get-time to [define] res timezone "GMT" [year +/-3] [month +/-3] [day +/-3] [hour +/-3] [min +/-3] [sec +/-3] [format "%a..."]
                        // get system time that's current, unless one of the options are used which go into the future/past as given

                        //
                        // Look for each option and collect relevant info
                        // First we MUST get each options position
                        //
                        char *res = find_keyword (mtext, VV_KEYTO, 1);
                        char *timezone = find_keyword (mtext, VV_KEYTIMEZONE, 1);
                        char *year = find_keyword (mtext, VV_KEYYEAR, 1);
                        char *month = find_keyword (mtext, VV_KEYMONTH, 1);
                        char *day = find_keyword (mtext, VV_KEYDAY, 1);
                        char *hour = find_keyword (mtext, VV_KEYHOUR, 1);
                        char *min = find_keyword (mtext, VV_KEYMIN, 1);
                        char *sec = find_keyword (mtext, VV_KEYSEC, 1);
                        char *format = find_keyword (mtext, VV_KEYFORMAT, 1);

                        //
                        // After all options positions have been found, we must get the options 
                        // for ALL of them
                        //
                        carve_statement (&res, "get-time", VV_KEYTO, 1, 1);
                        carve_statement (&timezone, "get-time", VV_KEYTIMEZONE, 0, 1);
                        carve_statement (&year, "get-time", VV_KEYYEAR, 0, 1);
                        carve_statement (&month, "get-time", VV_KEYMONTH, 0, 1);
                        carve_statement (&day, "get-time", VV_KEYDAY, 0, 1);
                        carve_statement (&hour, "get-time", VV_KEYHOUR, 0, 1);
                        carve_statement (&min, "get-time", VV_KEYMIN, 0, 1);
                        carve_statement (&sec, "get-time", VV_KEYSEC, 0, 1);
                        carve_statement (&format, "get-time", VV_KEYFORMAT, 0, 1);

                        // handle any 'define'd variables (that can be define'd)
                        // res is mandatory, but even if not, define_statement will skip if NULL
                        define_statement (&res, VV_DEFSTRING);

                        // No need to check if res!=NULL (i.e. it's mandatory), the above carve_statement checks that

                        //
                        // If there is data right after statement (i.e. 'get-time') and it has no option (such as call-web https://...)
                        // then mtext is this option (in this case https://...). In this particular case, we don't have such an option -
                        // every option has a keyword preceding it, including the first one.
                        //

                        oprintf ("%s=vely_time(%s, %s, %s, %s, %s, %s, %s, %s);\n", res, timezone==NULL ? "\"GMT\"":timezone, format==NULL ? "NULL":format, year==NULL ? "0":year, month==NULL ? "0":month, 
                            day==NULL ? "0":day, hour==NULL ? "0":hour, min==NULL ? "0":min, sec==NULL ? "0":sec);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "match-regex", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *pattern = mtext;
                        char *cache = find_keyword (mtext, VV_KEYCACHE, 1);
                        char *ccache = find_keyword (mtext, VV_KEYCLEARCACHE, 1);
                        char *in = find_keyword (mtext, VV_KEYIN, 1);
                        char *replace_with = find_keyword (mtext, VV_KEYREPLACEWITH, 1);
                        char *result = find_keyword (mtext, VV_KEYRESULT, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *case_insensitive = find_keyword (mtext, VV_KEYCASEINSENSITIVE, 1);
                        char *utf8 = find_keyword (mtext, VV_KEYUTF8, 1);
                        char *single_match = find_keyword (mtext, VV_KEYSINGLEMATCH, 1);

                        //
                        // After all options positions have been found, we must get the options 
                        // for ALL of them
                        //
                        carve_statement (&cache, "match-regex", VV_KEYCACHE, 0, 0);
                        carve_statement (&ccache, "match-regex", VV_KEYCLEARCACHE, 0, 1);
                        carve_statement (&in, "match-regex", VV_KEYIN, 1, 1);
                        carve_statement (&replace_with, "match-regex", VV_KEYREPLACEWITH, 0, 1);
                        carve_statement (&result, "match-regex", VV_KEYRESULT, 0, 1);
                        carve_statement (&status, "match-regex", VV_KEYSTATUS, 0, 1);
                        carve_statement (&case_insensitive, "match-regex", VV_KEYCASEINSENSITIVE, 0, 2);
                        carve_statement (&utf8, "match-regex", VV_KEYUTF8, 0, 2);
                        carve_statement (&single_match, "match-regex", VV_KEYSINGLEMATCH, 0, 2);

                        char *case_insensitivec = opt_clause(case_insensitive, "1", "0");
                        char *utf8c = opt_clause(utf8, "1", "0");
                        char *single_matchc = opt_clause(single_match, "1", "0");

                        if (result != NULL && replace_with == NULL)
                        {
                            _vely_report_error( "replace-with must be specified if result is given in match-regex statement");
                        }
                        if (replace_with != NULL && result == NULL)
                        {
                            _vely_report_error( "result must be specified if replace-with is given in match-regex statement");
                        }
                        if (result == NULL && status == NULL)
                        {
                            _vely_report_error( "status must be specified in this match-regex statement");
                        }

                        // result can be defined
                        define_statement (&result, VV_DEFSTRING);
                        define_statement (&status, VV_DEFNUM);

                        //
                        // If there is data right after statement (i.e. 'match') and it has no option (such as call-web https://...)
                        // then mtext is this option (in this case https://...). 
                        //

                        // if no cache, then we pass NULL pointer, if there is cache we pass address of (originally) NULL pointer
                        char regname[100];
                        snprintf (regname, sizeof(regname), "_vv_regex%lld", regex_cache);
                        oprintf ("static regex_t *%s = NULL;\nVV_UNUSED(%s);\n", regname, regname);
                        if (ccache != NULL) 
                        {
                            // if clear-cache is true, then first check if there's existing regex context, if so, clear it
                            // then set context to NULL so it's created again.
                            oprintf ("if ((%s)) {if (%s != NULL) vely_regfree(%s); %s = NULL;}\n", ccache, regname, regname, regname);
                        }

                        if (replace_with == NULL) oprintf ("%s%svely_regex(%s, %s, %s, %s, %s, %s, %s, %s%s);\n", status==NULL?"":status,status==NULL?"":"=", in, pattern, "NULL", "NULL", utf8c, case_insensitivec, single_matchc, cache==NULL?"":"&",cache==NULL?"NULL":regname);
                        else oprintf ("%s%svely_regex(%s, %s, %s, &(%s), %s, %s, %s, %s%s);\n", status==NULL?"":status,status==NULL?"":"=", in, pattern, replace_with, result, utf8c, case_insensitivec, single_matchc, cache==NULL?"":"&",cache==NULL?"NULL":regname);
                        vely_free(case_insensitivec);
                        vely_free(single_matchc);
                        vely_free(utf8c);

                        regex_cache++;
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "get-sys", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        //
                        // Look for each option and collect relevant info
                        // First we MUST get each options position
                        //
                        char *env = find_keyword (mtext, VV_KEYENVIRONMENT, 1);
                        char *webenv = find_keyword (mtext, VV_KEYWEBENVIRONMENT, 1);
                        char *osname = find_keyword (mtext, VV_KEYOSNAME, 1);
                        char *osver = find_keyword (mtext, VV_KEYOSVERSION, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        //
                        // After all options positions have been found, we must get the options 
                        // for ALL of them
                        //
                        carve_statement (&env, "get-sys", VV_KEYENVIRONMENT, 0, 1);
                        carve_statement (&webenv, "get-sys", VV_KEYWEBENVIRONMENT, 0, 1);
                        carve_statement (&osname, "get-sys", VV_KEYOSNAME, 0, 0);
                        carve_statement (&osver, "get-sys", VV_KEYOSVERSION, 0, 0);
                        carve_statement (&to, "get-sys", VV_KEYTO, 1, 1);

                        // result can be defined
                        define_statement (&to, VV_DEFSTRING);

                        num tot_opt = (webenv!=NULL?1:0)+(env!=NULL?1:0)+(osname!=NULL?1:0)+(osver!=NULL?1:0);
                        if (tot_opt != 1)
                        {
                            _vely_report_error( "Exactly one option must be in get-sys statement");
                        }

                        if (env !=NULL) oprintf ("%s=vely_getenv_os(%s);\n", to,env);
                        if (webenv !=NULL) oprintf ("%s=vely_getenv(%s);\n", to,webenv);
                        if (osname !=NULL) oprintf ("%s=vely_os_name();\n", to);
                        if (osver !=NULL) oprintf ("%s=vely_os_version();\n", to);


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "get-app", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        // Example:
                        // get-app name | version | directory  to [define] var

                        //
                        // Look for each option and collect relevant info
                        // First we MUST get each options position
                        //
                        char *appdir = find_keyword (mtext, VV_KEYDIRECTORY, 1);
                        char *name = find_keyword (mtext, VV_KEYNAME0, 1);
                        char *tracedir = find_keyword (mtext, VV_KEYTRACEDIRECTORY, 1);
                        char *filedir = find_keyword (mtext, VV_KEYFILEDIRECTORY, 1);
                        char *uploadsize = find_keyword (mtext, VV_KEYUPLOADSIZE, 1);
                        char *dbconfname = find_keyword (mtext, VV_KEYDBVENDOR, 1);
                        char *apath = find_keyword (mtext, VV_KEYPATH, 1);
                        char *pdata = find_keyword (mtext, VV_KEYPROCESSDATA, 1);

                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        //
                        // After all options positions have been found, we must get the options
                        // for ALL of them
                        //
                        carve_statement (&name, "get-app", VV_KEYNAME0, 0, 0);
                        carve_statement (&appdir, "get-app", VV_KEYDIRECTORY, 0, 0);
                        carve_statement (&tracedir, "get-app", VV_KEYTRACEDIRECTORY, 0, 0);
                        carve_statement (&filedir, "get-app", VV_KEYFILEDIRECTORY, 0, 0);
                        carve_statement (&uploadsize, "get-app", VV_KEYUPLOADSIZE, 0, 0);
                        carve_statement (&dbconfname, "get-app", VV_KEYDBVENDOR, 0, 1);
                        carve_statement (&apath, "get-app", VV_KEYPATH, 0, 0);
                        carve_statement (&pdata, "get-app", VV_KEYPROCESSDATA, 0, 0);

                        carve_statement (&to, "get-app", VV_KEYTO, 1, 1);

                        // result can be defined
                        if (uploadsize !=NULL) define_statement (&to, VV_DEFNUM);
                        else if (pdata !=NULL) define_statement (&to, VV_DEFVOIDPTR);
                        else define_statement (&to, VV_DEFSTRING);

                        num tot_opt = (appdir!=NULL?1:0)+(tracedir!=NULL?1:0)+(filedir!=NULL?1:0)+(apath!=NULL?1:0)+(uploadsize!=NULL?1:0)+(name!=NULL?1:0)+(dbconfname!=NULL?1:0)+(pdata!=NULL?1:0);
                        if (tot_opt != 1)
                        {
                            _vely_report_error( "Exactly one option must be in get-app statement");
                        }

                        if (appdir !=NULL) oprintf ("%s=vely_get_config()->app.home_dir;\n", to); // not alloced
                        if (tracedir !=NULL) oprintf ("%s=vely_get_config()->app.trace_dir;\n", to); // not alloced
                        if (filedir !=NULL) oprintf ("%s=vely_get_config()->app.file_dir;\n", to); // not alloced
                        if (uploadsize !=NULL) oprintf ("%s=vely_get_config()->app.max_upload_size;\n", to); // not alloced
                        if (name !=NULL) oprintf ("%s=vely_app_name;\n", to); // not alloced
                        if (pdata !=NULL) oprintf ("%s=vely_get_config()->ctx.data;\n", to); // not alloced
                        if (dbconfname !=NULL) 
                        {
                            num dbi = find_connection (dbconfname);
                            oprintf ("%s=\"%s\";\n", to, vely_db_vendor(dbi));  // not alloced
                        }
                        if (apath !=NULL) oprintf ("%s=vely_app_path;\n", to); // not alloced

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "get-req", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        //
                        // Look for each option and collect relevant info
                        // First we MUST get each options position
                        //
                        char *name = find_keyword (mtext, VV_KEYNAME0, 1);
                        char *method = find_keyword (mtext, VV_KEYMETHOD, 1);
                        char *ctype = find_keyword (mtext, VV_KEYCONTENTTYPE, 1);
                        char *data = find_keyword (mtext, VV_KEYDATA, 1);
                        char *eno = find_keyword (mtext, VV_KEYERRNO, 1);
                        char *err = find_keyword (mtext, VV_KEYERROR, 1);
                        char *process_id = find_keyword (mtext, VV_KEYPROCESSID, 1);
                        char *tracefile = find_keyword (mtext, VV_KEYTRACEFILE, 1);
                        char *argnum = find_keyword (mtext, VV_KEYARGCOUNT, 1);
                        char *argval = find_keyword (mtext, VV_KEYARGVALUE, 1);
                        char *numinput = find_keyword (mtext, VV_KEYINPUTCOUNT, 1);
                        char *inputval = find_keyword (mtext, VV_KEYINPUTVALUE, 1);
                        char *inputname = find_keyword (mtext, VV_KEYINPUTNAME, 1);
                        char *ref = find_keyword (mtext, VV_KEYREFERRINGURL, 1);
                        char *numcookie = find_keyword (mtext, VV_KEYCOOKIECOUNT, 1);
                        char *cookie = find_keyword (mtext, VV_KEYCOOKIE, 1);
                        char *header = find_keyword (mtext, VV_KEYHEADER, 1);
                        char *to = find_keyword (mtext, VV_KEYTO, 1);

                        //
                        // After all options positions have been found, we must get the options 
                        // for ALL of them
                        //
                        carve_statement (&name, "get-req", VV_KEYNAME0, 0, 0);
                        carve_statement (&method, "get-req", VV_KEYMETHOD, 0, 0);
                        carve_statement (&ctype, "get-req", VV_KEYCONTENTTYPE, 0, 0);
                        carve_statement (&data, "get-req", VV_KEYDATA, 0, 0);
                        carve_statement (&tracefile, "get-req", VV_KEYTRACEFILE, 0, 0);
                        carve_statement (&argnum, "get-req", VV_KEYARGCOUNT, 0, 0);
                        carve_statement (&argval, "get-req", VV_KEYARGVALUE, 0, 1);
                        carve_statement (&numinput, "get-req", VV_KEYINPUTCOUNT, 0, 0);
                        carve_statement (&inputval, "get-req", VV_KEYINPUTVALUE, 0, 1);
                        carve_statement (&inputname, "get-req", VV_KEYINPUTNAME, 0, 1);
                        carve_statement (&ref, "get-req", VV_KEYREFERRINGURL, 0, 0);
                        carve_statement (&numcookie, "get-req", VV_KEYCOOKIECOUNT, 0, 0);
                        carve_statement (&cookie, "get-req", VV_KEYCOOKIE, 0, 1);
                        carve_statement (&header, "get-req", VV_KEYHEADER, 0, 1);
                        carve_statement (&process_id, "get-req", VV_KEYPROCESSID, 0, 0);
                        carve_statement (&eno, "get-req", VV_KEYERRNO, 0, 0);
                        carve_statement (&err, "get-req", VV_KEYERROR, 0, 0);

                        carve_statement (&to, "get-req", VV_KEYTO, 1, 1);

                        // result can be defined
                        if (numinput != NULL || numcookie != NULL || argnum != NULL || method != NULL ) define_statement (&to, VV_DEFNUM);
                        else if (ctype != NULL || inputval != NULL || inputname != NULL || argval != NULL || cookie != NULL || header != NULL || name != NULL ) define_statement (&to, VV_DEFSTRING);
                        else if (data != NULL) define_statement (&to, VV_DEFSTRING);
                        else if (eno != NULL) define_statement (&to, VV_DEFNUM);
                        else if (process_id != NULL) define_statement (&to, VV_DEFNUM);
                        else define_statement (&to, VV_DEFSTRING);

                        num tot_opt = (tracefile!=NULL?1:0)+(numinput!=NULL?1:0)+(argval!=NULL?1:0)+(argnum!=NULL?1:0)+(inputval!=NULL?1:0)+(inputname!=NULL?1:0)+(ref!=NULL?1:0)+(numcookie!=NULL?1:0)+(cookie!=NULL?1:0)+(header!=NULL?1:0)+(process_id!=NULL?1:0)+(eno!=NULL?1:0)+(err!=NULL?1:0)+(data!=NULL?1:0)+(method!=NULL?1:0)+(ctype!=NULL?1:0)+(name!=NULL?1:0);
                        if (tot_opt != 1)
                        {
                            _vely_report_error( "Exactly one option must be in get-req statement (%lld)", tot_opt);
                        }

                        if (data !=NULL) oprintf ("%s=vely_get_config()->ctx.req->data;\n", to); // not alloced
                        if (tracefile !=NULL) oprintf ("%s=vely_get_config()->trace.fname;\n", to); // not alloced
                        if (numinput !=NULL) oprintf ("%s=vely_get_config()->ctx.req->ip.num_of_input_params;\n", to); // not alloced
                        if (argval !=NULL) oprintf ("%s=vely_get_config()->ctx.req->args.args[%s];\n", to, argval); //not alloced
                        if (argnum !=NULL) oprintf ("%s=vely_get_config()->ctx.req->args.num_of_args;\n", to); // not alloced
                        if (inputval !=NULL) oprintf ("%s=vely_get_config()->ctx.req->ip.values[%s];\n", to, inputval); // not alloced
                        if (inputname !=NULL) oprintf ("%s=vely_get_config()->ctx.req->ip.names[%s];\n", to, inputname); // not alloced
                        if (ref !=NULL) oprintf ("%s=vely_get_config()->ctx.req->referring_url;\n", to); // not alloced
                        if (numcookie !=NULL) oprintf ("%s=vely_get_config()->ctx.req->num_of_cookies;\n", to); // not alloced
                        if (cookie !=NULL) oprintf ("%s=vely_get_config()->ctx.req->cookies[%s].data;\n", to, cookie); //not alloced
                        if (process_id !=NULL) oprintf ("%s=getpid();\n", to); // not alloced
                        if (eno !=NULL) oprintf ("%s=vely_errno;\n", to); // not alloced
                        if (err !=NULL) oprintf ("%s=vely_strdup(strerror(vely_errno));\n", to); // alloced
                        if (header !=NULL) oprintf ("%s=vely_getheader(%s);\n", to, header); // not alloced (vely_getenv)
                        if (method !=NULL) oprintf ("%s=vely_get_config()->ctx.req->method;\n", to); // not alloced
                        if (name !=NULL) oprintf ("%s=vely_get_config()->ctx.req->name;\n", to); // not alloced
                        if (ctype !=NULL) oprintf ("%s=vely_getenv(\"CONTENT_TYPE\");\n", to); // not alloced


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "flush-output", &mtext, &msize, 1, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        oprintf ("vely_flush_out();\n");

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "set-app", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *data = find_keyword (mtext, VV_KEYPROCESSDATA, 1);

                        carve_statement (&data, "set-app", VV_KEYPROCESSDATA, 0, 1);

                        num tot_opt = (data!=NULL?1:0);
                        if (tot_opt != 1)
                        {
                            _vely_report_error( "Exactly one option must be in set-app statement (%lld)", tot_opt);
                        }

                        if (data != NULL) oprintf ("vely_get_config()->ctx.data = (void*)(%s);\n", data);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "set-req", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *data = find_keyword (mtext, VV_KEYDATA, 1);

                        carve_statement (&data, "set-req", VV_KEYDATA, 0, 1);

                        num tot_opt = (data!=NULL?1:0);
                        if (tot_opt != 1)
                        {
                            _vely_report_error( "Exactly one option must be in set-req statement (%lld)", tot_opt);
                        }

                        if (data != NULL) oprintf ("vely_get_config()->ctx.req->data = (void*)(%s);\n", data);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "pf-out", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
                            (newI1=recog_statement(line, i, "pf-web", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
                            (newI2=recog_statement(line, i, "pf-url", &mtext, &msize, 0, &vely_is_inline)) != 0
                        )  
                    {
                        VV_GUARD
                        i = newI+newI1+newI2;

                        // printf to stdout for standalone app, or to web/string otherwise 

                        num c_printf = (newI != 0 ? 1:0);
                        num c_printf_web = (newI1 != 0 ? 1:0);
                        //num c_printf_url = (newI2 != 0 ? 1:0);

                        // Example:
                        // printf "%s %lld", str, int
                        // this is no-enc printout

                        //printf statements must have a comma, so instead of printf-url "xxx" it must be printf-url "%s",xxx
                        //to avoid hard to track bugs
                        //look also for format (starting with unquoted quote), to know where the printf part starts
                        char *comma = find_keyword (mtext, ",", 0);
                        char *format = find_keyword (mtext, "\"", 0);
                        char *written = find_keyword (mtext, VV_KEYBYTESWRITTEN, 0);
                        char *toerr = find_keyword (mtext, VV_KEYTOERROR, 0);

                        carve_statement (&toerr, "pf-*", VV_KEYTOERROR, 0, 0);
                        //carve out bytes-written to get its value 
                        carve_statement (&written, "pf-*", VV_KEYBYTESWRITTEN, 0, 1);


                        if (format == NULL)
                        {
                            _vely_report_error( "format must be specified in pf-* statement");
                        }
                        else
                        {
                            *(format-1)=0; // to carve out written clause
                        }


                        if (comma == NULL || (comma-mtext) < (format-mtext))
                        {
                            _vely_report_error( "format must be followed by a comma and a list of arguments in pf-* statement");
                        }
                        if (written != NULL && (written-mtext > comma-mtext))
                        {
                            _vely_report_error( "bytes-written must be before format in pf-* statement");
                        }
                        // bytes-written may be define'd
                        define_statement (&written, VV_DEFNUM);

                        //
                        // Look for each option and collect relevant info
                        // First we MUST get each options position
                        //
                        oprintf ("%s%svely_printf(%s, %s, %s);\n", written==NULL?"":written,written==NULL?"":"=", toerr!=NULL?"true":"false",c_printf!=0?"VV_NOENC":(c_printf_web!=0?"VV_WEB":"VV_URL"),format);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "trace-run", &mtext, &msize, 1, &vely_is_inline)) != 0  ||
                        (newI1=recog_statement(line, i, "trace-run", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI+newI1;


                        // Example:
                        // trace-run "%s %lld", str, int
                        // this is no-enc printout

                        if (newI != 0)
                        {
                            // this is just trace-run and is useful for keeping track where we are
                            oprintf ("VV_TRACE(\"\");\n");
                        }
                        else
                        {
                            //trace statements must have a comma, so instead of trace "xxx" it must be trace "%s",xxx
                            //to avoid hard to track bugs
                            //look also for format (starting with unquoted quote), to know where the printf part starts
                            char *comma = find_keyword (mtext, ",", 0);
                            char *format = find_keyword (mtext, "\"", 0);
                            if (format == NULL)
                            {
                                _vely_report_error( "format must be specified in trace statement");
                            }

                            if (comma == NULL)
                            {
                                _vely_report_error( "format must be followed by a comma and a list of arguments in trace statement");
                            }
                            oprintf ("VV_TRACE(%s);\n", format);
                        }
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "copy-string", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;

                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *len = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *bwritten = find_keyword (mtext, VV_KEYBYTESWRITTEN, 1);
                        carve_statement (&to, "copy-string", VV_KEYTO, 1, 1);
                        carve_statement (&len, "copy-string", VV_KEYLENGTH, 0, 1);
                        carve_statement (&bwritten, "copy-string", VV_KEYBYTESWRITTEN, 0, 1);
                        define_statement (&to, VV_DEFSTRING);
                        define_statement (&len, VV_DEFNUM);
                        define_statement (&bwritten, VV_DEFNUM);
                        if (len != NULL && bwritten != 0) _vely_report_error("Cannot use length and bytes-written together in copy-string");

                        // must assign NULL, or it would assume cval is vely_string, which it may not be
                        if (len == NULL) oprintf ("VV_STRLDUP(%s,%s,%s%s%s);\n", to, mtext, bwritten == NULL ? "":"&(",bwritten == NULL ?"(num*)NULL":bwritten, bwritten == NULL ? "":")"); 
                        else
                        {
                            oprintf ("(((%s)=memcpy (vely_malloc (%s+1), (%s), (%s)))[%s]) = 0;\n",
                                to, len, mtext, len, len);
                        }


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "report-error", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *comma = find_keyword (mtext, ",", 0);
                        char *format = find_keyword (mtext, "\"", 0);
                        if (format == NULL)
                        {
                            _vely_report_error( "format must be specified in report-error statement");
                        }
                        else
                        {
                            *(format-1)=0; // to carve out possible written bytes clause in the future
                        }
                        if (comma == NULL || (comma-mtext) < (format-mtext))
                        {
                            _vely_report_error( "format must be followed by a comma and a list of arguments in report-error statement");
                        }
                        oprintf("vely_report_error(%s);\n", format);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "set-cookie", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *exp = find_keyword (mtext, VV_KEYEXPIRES, 1);
                        char *path = find_keyword (mtext, VV_KEYPATH, 1);
                        char *samesite = find_keyword (mtext, VV_KEYSAMESITE, 1);
                        char *nohttponly = find_keyword (mtext, VV_KEYNOHTTPONLY, 1);
                        char *secure = find_keyword (mtext, VV_KEYSECURE, 1);
                        char *eq = find_keyword (mtext, VV_KEYEQUALSHORT, 0);

                        carve_statement (&exp, "set-cookie", VV_KEYEXPIRES, 0, 1);
                        carve_statement (&path, "set-cookie", VV_KEYPATH, 0, 1);
                        carve_statement (&samesite, "set-cookie", VV_KEYSAMESITE, 0, 1);
                        carve_statement (&nohttponly, "set-cookie", VV_KEYNOHTTPONLY, 0, 2);
                        carve_statement (&secure, "set-cookie", VV_KEYSECURE, 0, 2); // may have data
                        carve_statement (&eq, "set-cookie", VV_KEYEQUALSHORT, 1, 1);

                        char *secc = opt_clause(secure, "\"Secure; \"", "\"\"");
                        char *httpc = opt_clause(nohttponly, "\"\"", "\"HttpOnly; \"");

                        // enforce that Strict is the default for SameSite and HttpOnly is the default
                        oprintf("vely_set_cookie (vely_get_config()->ctx.req, %s, %s, %s, %s, %s, %s, %s);\n", mtext, eq,
                            path == NULL ? "NULL" : path, exp == NULL ? "NULL" : exp, samesite == NULL ? "\"Strict\"" : samesite,
                            httpc, secc);

                        vely_free(secc);
                        vely_free(httpc);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "get-cookie", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *eq = find_keyword (mtext, VV_KEYEQUALSHORT, 0);
                        if (eq != NULL) carve_statement (&eq, "get-cookie", VV_KEYEQUALSHORT, 1, 1);
                        // this is for option being the very first after statement name, so keyword is " " (space
                        // between the statement and the data
                        char *cval = mtext;
                        carve_statement (&cval, "get-cookie", " ", 1, 1);
                        define_statement (&cval, VV_DEFSTRING);

                        oprintf("%s = vely_find_cookie (vely_get_config()->ctx.req, %s, NULL, NULL, NULL);\n", cval, eq); 

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "delete-cookie", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *path = find_keyword (mtext, VV_KEYPATH, 1);
                        char *st = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *secure = find_keyword (mtext, VV_KEYSECURE, 1);

                        carve_statement (&secure, "delete-cookie", VV_KEYSECURE, 0, 2);
                        carve_statement (&path, "delete-cookie", VV_KEYPATH, 0, 1);
                        carve_statement (&st, "delete-cookie", VV_KEYSTATUS, 0, 1);
                        define_statement (&st, VV_DEFNUM);

                        char *secc = opt_clause(secure, "\"Secure; \"", "\"\"");

                        oprintf("%s%svely_delete_cookie (vely_get_config()->ctx.req, %s, %s, %s);\n", st!=NULL ? st:"",st!=NULL ?"=":"", mtext, path==NULL?"NULL":path, secc ); 
                        vely_free(secc);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "request-body", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *len = find_keyword (mtext, VV_KEYLENGTH, 1);
                        carve_statement (&len, "request-body", VV_KEYLENGTH, 0, 1);
                        define_statement (&len, VV_DEFNUM);

                        char *var = NULL;
                        VV_STRDUP (var, mtext); // must have a copy because vely_trim could ruin further parsing, 
                                        // since we have 'i' up above already set to point in line
                        num var_len = strlen (var);
                        vely_trim (var, &var_len);
                        if (vely_is_valid_param_name(var) != 1)
                        {
                            _vely_report_error(VV_NAME_INVALID, var);
                        }
                        oprintf("char *%.*s = vely_get_config()->ctx.req->body;\n", (int)var_len, var);
                        if (len != NULL) oprintf("%s = vely_get_config()->ctx.req->body_len;\n", len);

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "set-input", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *eq = find_keyword (mtext, VV_KEYEQUALSHORT, 0);

                        carve_statement (&eq, "set-input", VV_KEYEQUALSHORT, 1, 1);

                        oprintf("vely_set_input (vely_get_config()->ctx.req, %s, %s);\n", mtext, eq);

                        continue;
                    }
                    else if (((newI=recog_statement (line, i, "if-task", &mtext, &msize, 0, &vely_is_inline)) != 0)
                        || ((newI1=recog_statement (line, i, "else-task", &mtext, &msize, 0, &vely_is_inline)) != 0))
                    {
                        VV_GUARD
                        i = newI+newI1;
                        bool else_if = false;
                        bool is_if = false;
                        if (newI1 != 0 ) else_if = true; else is_if = true;

                        // if if-task, then open_ifs is a +1 from previous if-task (or 0 if the first), so open_ifs is correct
                        if (is_if) other_task_done[open_ifs] = false; 

                        char *other = find_keyword (mtext, VV_KEYOTHER, 0);
                        carve_statement (&other, "if-task", VV_KEYOTHER, 0, 0);
                        if (other != NULL && !else_if)  _vely_report_error( "'other' clause can only be used with else-task");

                        if (else_if)
                        {
                            // if else-task, then open_ifs is +1 from when if-task was done, so open_ifs-1 is correct
                            if (open_ifs <= 0) _vely_report_error( "else-task found without an open if-task");
                            open_ifs--;
                            open_ifs++;
                            if (other_task_done[open_ifs-1] && other == NULL) _vely_report_error( "else-task with 'other' must be the last one in if-task statement");
                        }
                        else
                        {
                            if (open_ifs==0) last_line_if_closed = lnum; 
                            open_ifs++; // this is the only place where open_ifs increases
                            if (open_ifs >= VV_MAX_OTHER_TASK) _vely_report_error( "too many subtasks nested with if-task");
                        }

                        // At this point, open_ifs is 1 greater than the level, so on level 0 (original if-task), open_ifs is 1,
                        // on level 1 (one-nested if-task), open_ifs is 2 etc. That's because if-task will do +1, and for else-task
                        // it remains so. By the time code reaches here, it's +1
                        if (other != NULL) 
                        {
                            oprintf("} else {\n");
                            // here checking is done with open_ifs-1, because open_ifs is now +1 of what the level really is
                            if (other_task_done[open_ifs-1]) _vely_report_error( "'other' clause can only be used once with else-task");
                            other_task_done[open_ifs-1] = true;
                        }
                        else 
                        {
                            oprintf("%sif (", else_if == 1 ? "} else ":"");
                            char *curr = mtext;
                            oprintf("!strcmp ((%s), vely_get_config()->ctx.req->task == -1 ? \"\":vely_get_config()->ctx.req->ip.values[vely_get_config()->ctx.req->task])", curr);
                            oprintf(") {\n");
                        }

                        continue;
                    }
                    else if ((newI=recog_statement (line, i, "end-task", &mtext, &msize, 1, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("}\n");
                        if (open_ifs <= 0) _vely_report_error( "end-task found without an open if-task");
                        open_ifs--; // this is the only place where open_ifs decreases
                        // check done on real open_ifs, after --
                        other_task_done[open_ifs] = false;
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "input-param", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
                        (newI1=recog_statement(line, i, "task-param", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI + newI1;
                        bool task = (newI1 != 0);

                        char *var = NULL;
                        VV_STRDUP (var, mtext); // must have a copy because vely_trim could ruin further parsing, 
                                        // since we have 'i' up above already set to point in line
                        num var_len = strlen (var);
                        vely_trim (var, &var_len);
                        if (vely_is_valid_param_name(var) != 1)
                        {
                            _vely_report_error(VV_NAME_INVALID, var);
                        }
                        oprintf("char *%.*s = vely_get_input_param (vely_get_config()->ctx.req, \"%.*s\", %s);\n", (int)var_len, var, (int)var_len, var, task?"true":"false"); 
                        if (task) oprintf("VV_UNUSED(%s);\n",  var); // task input param can be unused (the var)

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "split-string", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *with = find_keyword (mtext, VV_KEYWITH, 1);
                        char *del = find_keyword (mtext, VV_KEYDELETE, 1);

                        if (del != NULL && (to != NULL || with != NULL)) _vely_report_error ("Only one of 'to'/'with' or 'delete' can be used in split-string");
                        if (del == NULL && to == NULL) _vely_report_error ("At least one of 'to'/'with' or 'delete' must be used in split-string");

                        if (to != NULL) 
                        {
                            carve_statement (&to, "split-string", VV_KEYTO, 1, 1);
                            carve_statement (&with, "split-string", VV_KEYWITH, 1, 1);
                        }
                        if (del != NULL) carve_statement (&del, "split-string", VV_KEYDELETE, 0, 1);

                        if (to != NULL) define_statement (&to, VV_DEFBROKEN);

                        if (to != NULL) oprintf("vely_break_down (%s, %s, &(%s));\n", mtext, with, to);
                        if (del != NULL) oprintf ("vely_delete_break_down (&(%s));\n", del);

                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "unused-var", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        i = newI;


                        oprintf("VV_UNUSED (%s);\n", mtext);
                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "new-fifo", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        define_statement (&mtext, VV_DEFFIFO);
                        oprintf ("vely_store_init (&(%s));\n", mtext);

                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "write-fifo", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *key = find_keyword (mtext, VV_KEYKEY, 1);
                        char *value = find_keyword (mtext, VV_KEYVALUE, 1);

                        carve_statement (&key, "fifo-list", VV_KEYKEY, 1, 1);
                        carve_statement (&value, "fifo-list", VV_KEYVALUE, 1, 1);


                        oprintf ("vely_store (%s, %s, (void*)%s);\n", mtext, key, value);

                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "rewind-fifo", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        oprintf ("vely_rewind (%s);\n", mtext);

                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "purge-fifo", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *del = find_keyword (mtext, VV_KEYDELETE0, 1);
                        carve_statement (&del, "purge-fifo", VV_KEYDELETE0, 0, 0);
                        oprintf ("vely_purge (&(%s), %s);\n", mtext, del!=NULL?"0":"1");

                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "read-fifo", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *key = find_keyword (mtext, VV_KEYKEY, 1);
                        char *value = find_keyword (mtext, VV_KEYVALUE, 1);

                        carve_statement (&key, "fifo-list", VV_KEYKEY, 1, 1);
                        carve_statement (&value, "fifo-list", VV_KEYVALUE, 1, 1);

                        define_statement (&key, VV_DEFSTRING);
                        define_statement (&value, VV_DEFSTRING);

                        oprintf ("vely_retrieve (%s, &(%s), (void**)&(%s));\n", mtext, key, value);

                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "write-file", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *from = find_keyword (mtext, VV_KEYFROM, 1);
                        char *append = find_keyword (mtext, VV_KEYAPPEND, 1);
                        char *length = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *pos = find_keyword (mtext, VV_KEYPOSITION, 1);
                        char *fileid = find_keyword (mtext, VV_KEYFILEID, 1);

                        carve_statement (&from, "write-file", VV_KEYFROM, 1, 1);
                        carve_statement (&append, "write-file", VV_KEYAPPEND, 0, 2);
                        carve_statement (&length, "write-file", VV_KEYLENGTH, 0, 1);
                        carve_statement (&status, "write-file", VV_KEYSTATUS, 0, 1);
                        carve_statement (&pos, "write-file", VV_KEYPOSITION, 0, 1);
                        carve_statement (&fileid, "write-file", VV_KEYFILEID, 0, 1);

                        define_statement (&status, VV_DEFNUM);

                        char *appendc = opt_clause(append, "1", "0");

                        // can trim mtext here because all is already carved, and it's just a ptr trim
                        num lm = strlen (mtext);
                        mtext = vely_trim_ptr(mtext,  &lm);
                        if (mtext[0] != 0 && fileid!=NULL) _vely_report_error( "you can specify either file name or file-id but not both in write-file statement");

                        if (append != NULL && pos!=NULL) _vely_report_error( "'append' and 'position' cannot both be in write-file statement");

                        if (length==NULL) length="0";

                        if (fileid != NULL) oprintf("%s%svely_write_file_id (*((%s)->f), %s, %s, %s, %s, %s);\n", status!=NULL ? status:"",status!=NULL ?"=":"",fileid, from, length, appendc, pos == NULL ? "0":pos, pos==NULL?"0":"1");
                        else oprintf("%s%svely_write_file (%s, %s, %s, %s, %s, %s);\n", status!=NULL ? status:"",status!=NULL ?"=":"",mtext, from, length, appendc, pos == NULL ? "0":pos, pos==NULL?"0":"1");
                        vely_free(appendc);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "read-file", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *read_from = mtext;
                        char *read_to = find_keyword (mtext, VV_KEYTO, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        char *pos = find_keyword (mtext, VV_KEYPOSITION, 1);
                        char *length = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *fileid = find_keyword (mtext, VV_KEYFILEID, 1);

                        carve_statement (&read_to, "read-file", VV_KEYTO, 1, 1);
                        carve_statement (&status, "read-file", VV_KEYSTATUS, 0, 1);
                        carve_statement (&pos, "read-file", VV_KEYPOSITION, 0, 1);
                        carve_statement (&length, "read-file", VV_KEYLENGTH, 0, 1);
                        carve_statement (&fileid, "read-file", VV_KEYFILEID, 0, 1);

                        define_statement (&status, VV_DEFNUM);
                        define_statement (&read_to, VV_DEFSTRING);

                        if (length == NULL) length="0";

                        if (fileid != NULL) oprintf("%s%svely_read_file_id (*((%s)->f), &(%s), %s, %s, %s);\n", status!=NULL ? status:"",status!=NULL ?"=":"",fileid, read_to, pos==NULL?"0":pos, length, pos!=NULL?"true":"false");
                        else oprintf("%s%svely_read_file (%s, &(%s), %s, %s);\n", status!=NULL ? status:"",status!=NULL ?"=":"",read_from, read_to, pos==NULL?"0":pos, length);
                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "get-hash", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *len = find_keyword (mtext, VV_KEYLENGTH, 1);
                        char *size = find_keyword (mtext, VV_KEYSIZE, 1);
                        char *reads = find_keyword (mtext, VV_KEYAVERAGEREADS, 1);


                        carve_statement (&size, "get-hash", VV_KEYSIZE, 0, 1);
                        carve_statement (&len, "get-hash", VV_KEYLENGTH, 0, 1);
                        carve_statement (&reads, "get-hash", VV_KEYAVERAGEREADS, 0, 1);
                        define_statement (&len, VV_DEFNUM);
                        define_statement (&size, VV_DEFNUM);
                        define_statement (&reads, VV_DEFDBL);

                        if (size == NULL && len == NULL && reads == NULL) _vely_report_error( "one of 'length', 'size' or 'average-reads' must be in get-hash statement");

                        if (len !=NULL) oprintf("%s=vely_total_hash (%s);\n", len, mtext);
                        if (size !=NULL) oprintf("%s=vely_hash_size (%s);\n", size, mtext);
                        if (reads !=NULL) oprintf("%s=vely_hash_reads (%s);\n", reads, mtext);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "purge-hash", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *del = find_keyword (mtext, VV_KEYDELETE0, 1);
                        carve_statement (&del, "purge-hash", VV_KEYDELETE0, 0, 0);
                        oprintf("vely_delete_hash (&(%s),%s);\n", mtext, del!=NULL?"0":"1");
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "resize-hash", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *size = find_keyword (mtext, VV_KEYSIZE, 1);

                        carve_statement (&size, "resize-hash", VV_KEYSIZE, 1, 1);

                        oprintf("vely_resize_hash (&(%s), %s);\n", mtext, size);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "new-hash", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *size = find_keyword (mtext, VV_KEYSIZE, 1);

                        carve_statement (&size, "new-hash", VV_KEYSIZE, 1, 1);
                        define_statement (&mtext, VV_DEFHASH);

                        oprintf("vely_create_hash (&(%s), %s);\n", mtext, size);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "write-hash", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *key = find_keyword (mtext, VV_KEYKEY, 1);
                        char *val = find_keyword (mtext, VV_KEYVALUE, 1);
                        char *oldd = find_keyword (mtext, VV_KEYOLDVALUE, 1);
                        char *oldk = find_keyword (mtext, VV_KEYOLDKEY, 1);
                        char *st = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&key, "write-hash", VV_KEYKEY, 1, 1);
                        carve_statement (&val, "write-hash", VV_KEYVALUE, 1, 1);
                        carve_statement (&oldd, "write-hash", VV_KEYOLDVALUE, 0, 1);
                        carve_statement (&oldk, "write-hash", VV_KEYOLDKEY, 0, 1);
                        carve_statement (&st, "write-hash", VV_KEYSTATUS, 0, 1);
                        define_statement (&oldd, VV_DEFSTRING);
                        define_statement (&oldk, VV_DEFSTRING);
                        define_statement (&st, VV_DEFNUM);

                        oprintf("%s%svely_add_hash (%s, %s, %s, %s%s%s, %s%s%s);\n", oldd==NULL?"":oldd, oldd==NULL?"":"=",mtext, key, val, st==NULL?"":"&(", st==NULL?"NULL":st, st==NULL?"":")", oldk==NULL?"":"&(", oldk==NULL?"NULL":oldk, oldk==NULL?"":")");
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "read-hash", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *key = find_keyword (mtext, VV_KEYKEY, 1);
                        char *oldk = find_keyword (mtext, VV_KEYOLDKEY, 1);
                        char *val = find_keyword (mtext, VV_KEYVALUE, 1);
                        char *del = NULL;
                        char *st = NULL;
                        char *trav = find_keyword (mtext, VV_KEYTRAVERSE, 1);
                        char *beg = NULL;

                        if (trav != NULL)
                        {
                            beg = find_keyword (mtext, VV_KEYBEGIN, 1);
                        }
                        else
                        {
                            del = find_keyword (mtext, VV_KEYDELETE0, 1);
                            st = find_keyword (mtext, VV_KEYSTATUS, 1);
                        }


                        //
                        // Carving must be after find_keyword
                        //
                        if (trav != NULL)
                        {
                            carve_statement (&beg, "read-hash", VV_KEYBEGIN, 0, 0);
                            carve_statement (&key, "read-hash", VV_KEYKEY, 0, 1);
                            carve_statement (&val, "read-hash", VV_KEYVALUE, 0, 1);
                            carve_statement (&trav, "read-hash", VV_KEYTRAVERSE, 0, 0);
                        }
                        else
                        {
                            carve_statement (&key, "read-hash", VV_KEYKEY, 1, 1);
                            carve_statement (&oldk, "read-hash", VV_KEYOLDKEY, 0, 1);
                            carve_statement (&del, "read-hash", VV_KEYDELETE0, 0, 2);
                            carve_statement (&st, "read-hash", VV_KEYSTATUS, 0, 1);
                            carve_statement (&val, "read-hash", VV_KEYVALUE, 1, 1);
                        }

                        char *delc = opt_clause(del, "1", "0");

                        define_statement (&val, VV_DEFSTRING);
                        if (trav != NULL)
                        {
                            define_statement (&key, VV_DEFSTRING);
                        }
                        else
                        {
                            define_statement (&st, VV_DEFNUM);
                            define_statement (&oldk, VV_DEFSTRING);
                        }


                        if (trav == NULL)
                        {
                            oprintf("%s=vely_find_hash (%s, %s, %s, %s%s%s, %s%s%s);\n", val,mtext, key, delc, st==NULL?"":"&(", st==NULL?"NULL":st, st==NULL?"":")", oldk==NULL?"":"&(", oldk==NULL?"NULL":oldk, oldk==NULL?"":")");
                        }
                        else
                        {
                            if ((key == NULL && val != NULL) || (key != NULL && val == NULL)) _vely_report_error( "'key' and 'value' must both be in read-hash statement");
                            if (beg != NULL) oprintf ("vely_rewind_hash(%s);\n", mtext);
                            if (key != NULL) oprintf("%s=vely_next_hash (%s, (void*)&(%s));\n", key, mtext, val);
                        }
                        vely_free(delc);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "manage-memory", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *on = find_keyword (mtext, VV_KEYON, 0);
                        char *off = find_keyword (mtext, VV_KEYOFF, 0);
                        carve_statement (&on, "manage-memory", VV_KEYON, 0,0);
                        carve_statement (&off, "manage-memory", VV_KEYOFF, 0,0);
                        // After carving out on/off, see if there is an expression
                        char *exp = NULL;
                        VV_STRDUP (exp, mtext); // must have a copy because vely_trim could ruin further parsing, 
                                        // since we have 'i' up above already set to point in line
                        num exp_len = strlen (exp);
                        vely_trim (exp, &exp_len);


                        if (on == NULL && off == NULL && exp[0] == 0) _vely_report_error( "must specify either 'on', 'off' or boolean expression in manage-memory statement");
                        if (on != NULL && off != NULL) _vely_report_error( "cannot specify both 'on' and 'off' in manage-memory statement");
                        if ((on != NULL || off != NULL) && exp[0] != 0)_vely_report_error( "cannot specify both a boolean expression and  'on'/'off' in manage-memory statement");
                        if (on != NULL) oprintf("vely_mem_os = false;\n");
                        else if (off != NULL) oprintf("vely_mem_os = true;\n");
                        else if (exp[0] != 0) oprintf("vely_mem_os = (%s);\n", exp);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "delete-mem", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        carve_statement (&status, "delete-mem", VV_KEYSTATUS, 0, 1);
                        define_statement (&status, VV_DEFNUM);
                        oprintf("%s%svely_safe_free ((void*)(%s));\n", status==NULL?"":status, status==NULL?"":"=",mtext);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "new-mem", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *size = find_keyword (mtext, VV_KEYSIZE, 1);
                        char *bsize = find_keyword (mtext, VV_KEYBLOCKSIZE, 1);
                        char *init = find_keyword (mtext, VV_KEYINIT, 1);
                        carve_statement (&size, "new-mem", VV_KEYSIZE, 1, 1);
                        carve_statement (&bsize, "new-mem", VV_KEYBLOCKSIZE, 0, 1);
                        carve_statement (&init, "new-mem", VV_KEYINIT, 0, 0);
                        define_statement (&mtext, VV_DEFVOIDPTR);

                        if (bsize == NULL) 
                        {
                            if (init == NULL) oprintf("%s=(void *)vely_malloc (%s);\n", mtext, size);
                            else oprintf("%s=(void *)vely_calloc ((%s), 1);\n", mtext, size);
                        }
                        else 
                        {
                            if (init == NULL) oprintf("%s=(void *)vely_malloc ((%s)*(%s));\n", mtext, size, bsize);
                            else oprintf("%s=(void *)vely_calloc ((%s), (%s));\n", mtext, size, bsize);
                        }
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "resize-mem", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *size = find_keyword (mtext, VV_KEYSIZE, 1);
                        carve_statement (&size, "resize-mem", VV_KEYSIZE, 1, 1);

                        oprintf("%s=(void*)_vely_realloc (%s, %s, 1);\n", mtext, mtext, size);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "copy-file", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        char *copy_to = find_keyword (mtext, VV_KEYTO, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);
                        carve_statement (&copy_to, "copy-file", VV_KEYTO, 1, 1);
                        carve_statement (&status, "copy-file", VV_KEYSTATUS, 0, 1);
                        define_statement (&status, VV_DEFNUM);
                        char *copy_from = mtext;
                        oprintf("%s%svely_copy_file (%s, %s);\n", status!=NULL ? status:"",status!=NULL ?"=":"",copy_from, copy_to);
                        continue;

                    }
                    else if ((newI=recog_statement(line, i, "finish-output", &mtext, &msize, 1, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        oprintf("vely_FCGI_Finish();\n");
                        oprintf("vely_disable_output();\n");
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "exit-request", &mtext, &msize, 1, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        // return 1 to sigsetjmp so it differs from the first time around
                        // exit-request will have no effect in _startup because sigsetjmp not done yet
                        oprintf("vely_exit_request(1);\n");
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "file-position", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *fileid = find_keyword (mtext, VV_KEYFILEID, 1);
                        char *get = find_keyword (mtext, VV_KEYGET, 1);
                        char *set = find_keyword (mtext, VV_KEYSET, 1);
                        char *status = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&fileid, "file-position", VV_KEYFILEID, 1, 1);
                        carve_statement (&get, "file-position", VV_KEYGET, 0, 1);
                        carve_statement (&set, "file-position", VV_KEYSET, 0, 1);
                        carve_statement (&status, "file-position", VV_KEYSTATUS, 0, 1);

                        define_statement (&get, VV_DEFNUM);
                        define_statement (&status, VV_DEFNUM);

                        if (get == NULL && set == NULL)  _vely_report_error( "either 'get' or 'set' must be used in file-position statement");
                        if (get != NULL && set != NULL)  _vely_report_error( "cannot specify both 'get' and 'set' in file-position statement");
                        if (get != NULL) oprintf("%s%svely_get_file_pos (*((%s)->f), &(%s));\n", status==NULL?"":status, status==NULL?"":"=",fileid, get);
                        else if (set != NULL) oprintf("%s%svely_set_file_pos (*((%s)->f), %s);\n", status==NULL?"":status, status==NULL?"":"=", fileid, set);
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "close-file", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *fileid = find_keyword (mtext, VV_KEYFILEID, 1);
                        char *st = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&fileid, "close-file", VV_KEYFILEID, 1, 1);
                        carve_statement (&st, "close-file", VV_KEYSTATUS, 0, 1);
                        define_statement (&st, VV_DEFNUM);

                        if (st == NULL) oprintf("vely_fclose (*((%s)->f));\n", fileid);
                        else oprintf("%s = vely_fclose (*((%s)->f));\n", st, fileid);
                        // set file pointer to NULL, because that's checked in vely_done()
                        // this is needed beyond vely_done(), any file op would check if NULL so avoids crashes
                        oprintf ("*((%s)->f) = NULL;\n", fileid);
                        oprintf("vely_clear_mem ((%s)->memind);\n", fileid); // also clear vm[]->ptr in velymem.c
                                                                             // this makes vely_done faster because
                                                                             // when ptr is NULL, it doesnt need to check
                                                                             // for VV_MEM_FILE flag
                        oprintf("vely_free (%s);\n", fileid); 
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "open-file", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI;
                        char *fileid = find_keyword (mtext, VV_KEYFILEID, 1);
                        char *newt = find_keyword (mtext, VV_KEYNEWTRUNCATE, 1);
                        char *st = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&newt, "open-file", VV_KEYNEWTRUNCATE, 0, 0);
                        carve_statement (&fileid, "open-file", VV_KEYFILEID, 1, 1);
                        carve_statement (&st, "open-file", VV_KEYSTATUS, 0, 1);

                        define_statement (&fileid, VV_DEFFILE);
                        define_statement (&st, VV_DEFNUM);

                        //VERR is set in vely_fopen
                        oprintf("%s = vely_calloc (1, sizeof(vely_file));\n", fileid); // this sets file pointer f to NULL
                        // need static FILE * so that its address points to valid memory anytime, including in vely_done()
                        // where any open file descriptors are at least checked, if not closed
                        oprintf("static FILE *_vv_fileid_%lld;\n", file_count); 
                        if (st == NULL) oprintf("_vv_fileid_%lld = vely_fopen (%s, \"%s\");\n", file_count, mtext, newt!=NULL?"w+":"r+");
                        else oprintf("if ((_vv_fileid_%lld = vely_fopen (%s, \"%s\")) == NULL) %s=VV_ERR_OPEN; else %s=VV_OKAY;\n", file_count, mtext, newt!=NULL?"w+":"r+", st, st);
                        // set FILE*, and also the index into vv[] (the memind member), so that when closing, it can 
                        // clear the vv[]->ptr and make vely_done() faster
                        oprintf ("if (_vv_fileid_%lld != NULL) {(%s)->f = &_vv_fileid_%lld; (%s)->memind = vely_reg_file(&_vv_fileid_%lld);}\n", file_count, fileid, file_count, fileid, file_count);
                        file_count++;
                        continue;
                    }
                    //
                    // This is a method to have all optional params - check for both with no params (isLast==1) and with params
                    // (isLast==0). Based on return value we may look for optional params.
                    // Checking for non-mandatory (6th param is 1) MUST be done first.
                    //
                    else if ((newI=recog_statement(line, i, "end-write-string", &mtext, &msize, 1, &vely_is_inline)) != 0 ||
                        (newI1=recog_statement(line, i, "end-write-string", &mtext, &msize, 0, &vely_is_inline)) != 0  ||
                        (newI2=recog_statement(line, i, "))", &mtext, &msize, 1, &vely_is_inline)) != 0 ||
                        (newI3=recog_statement(line, i, "))", &mtext, &msize, 0, &vely_is_inline)) != 0)
                     {
                        VV_GUARD
                        i = newI+newI1+newI2+newI3;
                        // end-write-string, being string writing within string writing, can NOT be within <<..>>
                        if (within_inline == 1)
                        {
                            _vely_report_error( "Cannot use write-string as inline code");
                        }
                        // checking for notrim must be before vely_write_to_string (NULL) below because it's used in it
                        // find_keyword must be done before any carve_statement, even if it's used later
                        char *notrim = NULL;
                        char *bytes = NULL;
                        if (newI1 != 0 || newI3 != 0)
                        {
                            notrim = find_keyword (mtext, VV_KEYNOTRIM, 1);
                            bytes = find_keyword (mtext, VV_KEYBYTESWRITTEN, 1);
                            carve_statement (&notrim, "end-write-string", VV_KEYNOTRIM, 0, 0);
                            carve_statement (&bytes, "end-write-string", VV_KEYBYTESWRITTEN, 0, 1);
                        }
                        if (notrim != NULL)
                        {
                            oprintf("vely_write_to_string_notrim();\n");
                        }
                        oprintf("vely_write_to_string (NULL);\n"); 
                        if (bytes != NULL)
                        {
                            define_statement (&bytes, VV_DEFNUM);
                            oprintf("%s%svely_write_to_string_length();\n", bytes==NULL?"":bytes,bytes==NULL?"":"=");
                        }
                        gen_ctx->total_write_string--;

                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "write-string", &mtext, &msize, 0, &vely_is_inline)) != 0 ||
    -                    (newI1=recog_statement(line, i, "((", &mtext, &msize, 0, &vely_is_inline)) != 0)
                    {
                        VV_GUARD
                        i = newI+newI1;
                        // write-string, being string writing within string writing, can NOT be within <<..>>
                        if (within_inline == 1)
                        {
                            _vely_report_error( "Cannot use write-string as inline code");
                        }

                        char *tval = mtext;
                        carve_statement (&tval, "write-string", " ", 1, 1);
                        define_statement (&tval, VV_DEFSTRING);

                        oprintf("vely_write_to_string (&(%s));\n", tval); 
                        gen_ctx->total_write_string++;


                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "read-line", &mtext, &msize, 0, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;

                        char *to = find_keyword (mtext, VV_KEYTO, 1);
                        char *del = find_keyword (mtext, VV_KEYDELIMITER, 1);
                        char *len = find_keyword (mtext, VV_KEYSTATUS, 1);

                        carve_statement (&to, "read-line", VV_KEYTO, 1, 1);
                        carve_statement (&del, "read-line", VV_KEYDELIMITER, 0, 1);
                        carve_statement (&len, "read-line", VV_KEYSTATUS, 0, 1);

                        define_statement (&to, VV_DEFSTRING);
                        define_statement (&len, VV_DEFNUM);
                        if (del == NULL) del = "'\\n'";

                        oprintf("{\n");
                        if (len != NULL) oprintf("num *vely_rl_read_%lld = &(%s);\n", open_readline, len);
                        else 
                        {
                            oprintf("num _vely_rl_read_%lld;\n", open_readline);
                            oprintf("num *vely_rl_read_%lld = &(_vely_rl_read_%lld);\n", open_readline, open_readline);
                        }
                        oprintf("size_t vely_rl_local_len_%lld = 2;\n", open_readline);
                        oprintf("size_t vely_rl_len_%lld = 0;\n", open_readline);
                        oprintf("char *vely_rl_local_memptr_%lld = vely_malloc (vely_rl_local_len_%lld);\n", open_readline, open_readline);
                        oprintf("char *vely_rl_memptr_%lld = NULL;\n", open_readline);
                        oprintf("FILE *vely_rl_%lld = fopen (%s, \"r\");\n", open_readline, mtext); 
                        oprintf("if (vely_rl_%lld != NULL) {\n", open_readline);
                        oprintf("while (1) {\n");
                        oprintf("*vely_rl_read_%lld = (num)getdelim (&vely_rl_memptr_%lld, &vely_rl_len_%lld, (int)%s, vely_rl_%lld);\n", open_readline, open_readline, open_readline, del, open_readline);
                        oprintf("if (*vely_rl_read_%lld == -1) { break;}\n",  open_readline);
                        oprintf("if (vely_rl_local_len_%lld <= vely_rl_len_%lld) { vely_rl_local_memptr_%lld = vely_realloc (vely_rl_local_memptr_%lld, vely_rl_local_len_%lld=vely_rl_len_%lld); };\n", open_readline,open_readline,open_readline,open_readline,open_readline,open_readline);
                        oprintf ("memcpy(vely_rl_local_memptr_%lld, vely_rl_memptr_%lld, *vely_rl_read_%lld+1);\n", open_readline,open_readline,open_readline);
                        oprintf ("%s = vely_rl_local_memptr_%lld;\n", to, open_readline);
                        oprintf ("free(vely_rl_memptr_%lld); vely_rl_memptr_%lld = NULL; vely_rl_len_%lld=0;\n", open_readline,open_readline,open_readline); // open_readline was ++ after generating the code, so this is it

                        check_next_readline
                        continue;
                    }
                    else if ((newI=recog_statement(line, i, "end-read-line", &mtext, &msize, 1, &vely_is_inline)) != 0)  
                    {
                        VV_GUARD
                        i = newI;
                        oprintf("}\n");
                        oprintf ("if (!feof(vely_rl_%lld)) {VERR;*vely_rl_read_%lld = (num)VV_ERR_READ;} else {*vely_rl_read_%lld = (num)VV_OKAY;}\n", open_readline-1, open_readline-1, open_readline-1); // this is if there was an error, not end of file
                        oprintf ("fclose (vely_rl_%lld);\n", open_readline-1); // open_readline was ++ after generating the code, so this is it
                        oprintf("} else { VERR;*vely_rl_read_%lld = (num)VV_ERR_OPEN; }\n", open_readline-1);
                        oprintf ("if (vely_rl_local_memptr_%lld!=NULL) vely_free (vely_rl_local_memptr_%lld);\n", open_readline-1, open_readline-1);
                        oprintf ("if (vely_rl_memptr_%lld != NULL) free(vely_rl_memptr_%lld);\n",  open_readline-1, open_readline-1);
                        oprintf("}\n");
                        open_readline--;

                        continue;
                    }
                    else if (print_mode != 1)
                    {
                        // anything unrecognized is C code
                        check_vely(line+i);
                        oprintf("%s\n", line+i);
                        i += strlen (line+i);
                        continue;
                    }
                    else
                    {
                        if (within_inline == 1) 
                        {
                            // anything unrecognized is C code
                            check_vely(line+i);
                            oprintf("%s", line+i);
                            i += strlen (line+i);
                            continue;
                        }
                    }
                    // we come here if it is print mode and all others are unrecognized 
                }
            }
            first_on_line = 0;

            if (line[i] == '"') 
            {
                oprintf("\\\"");
            }
            else if (line[i] == '\\') 
            {
                oprintf("\\\\");
            }
            else if (line[i] == '\n' || line[i] == 0 || i>=len) 
            {
                // should not get here because we vely_trim line, and never reach 0 char
                // and should never go beyond the length of the line
                _vely_report_error ("Parsing error, extra text found, [%s]", line);
            }
            else if (line[i] == '%')
            {
                oprintf("%%");
            }
            else 
            {
                oprintf("%c", line[i]);
            }
        
        }
        if (within_inline == 1)
        {
            _vely_report_error( "Inline code was open with << but not closed with >>");
        }
        //See comment above for this. In addition, this is when >> is the very last on line, so the above does not
        //execute. We have to start this, even if it's gonna be empty, because we need to print this new line below
        //to finish the @ line. This is only if we're in @, i.e. printing.
        VV_OPEN_PRINT

        if (print_mode == 1)
        {
            // Print new line at the end
            oprintf("\\n\");\n");
        }
    

        if (feof (f)) break; // something read in, then EOF
    }

    if (ccomm_open)
    {
        _vely_report_error( "Opening C comment /* found on line [%lld], but it was never closed", ccomm_line);
    }

    if (gen_ctx->total_write_string != 0)
    {
        _vely_report_error( "Imbalance in write-string/end-write-string statements, too many open or not closed");
    }

    if (open_queries != 0)
    {
        _vely_report_error( "'query' code block imbalance at line %lld, %lld %s open than closed", last_line_query_closed, llabs(open_queries), open_queries > 0 ? "more" : "less" );
    }
    if (open_readline != 0)
    {
        _vely_report_error( "'read-line' code block imbalance at line check line %lld, %lld %s open than closed", last_line_readline_closed, llabs(open_readline), open_readline > 0 ? "more" : "less");
    }
     if (open_ifs != 0)
    {
        _vely_report_error( "'if-task' code block imbalance at line %lld, %lld %s open than closed", last_line_if_closed, llabs(open_ifs), open_ifs > 0 ? "more" : "less" );
    }
    if (gen_ctx->curr_qry_ptr !=0)
    {
        _vely_report_error( "Query imbalance (too many queries opened, too few closed)");
    }


}

#undef VV_FILE
#undef VV_LINE
#define VV_FILE "[no file opened yet]"
#define VV_LINE 0


//
// Main code, generates the source C file (as a __<file name>.c) based on input file
// and input parameters
//

int main (int argc, char* argv[])
{

    // environment var subst, does not use malloc
    if (argc == 2 && !strcmp (argv[1], "-envsub"))
    {
        envsub();
        exit(0); // no return from environment substitution
    }

    vely_pc = vely_alloc_config ();
    vely_memory_init();

    vely_gen_ctx *gen_ctx = (vely_gen_ctx*)vely_malloc(sizeof (vely_gen_ctx));
    init_vely_gen_ctx (gen_ctx);

    //
    // The following two options allow URL and WEB encoding of strings
    // from the command line. Typically used to construct URLs for command line
    // execution.
    //
    if (argc == 3 && !strcmp (argv[1], "-urlencode"))
    {
        // display URL encoded version of a string
        char *res = NULL;
        vely_encode (VV_URL, argv[2], -1, &res);
        fprintf (stdout, "%s", res);
        exit(0);
    }
    if (argc == 3 && !strcmp (argv[1], "-webencode"))
    {
        // display URL encoded version of a string
        char *res = NULL;
        vely_encode (VV_WEB, argv[2], -1, &res);
        fprintf (stdout, "%s", res);
        exit(0);
    }

    //
    // Get help from command line
    //
    if (argc == 1 || (argc == 2 && !strcmp (argv[1], "-version")))
    {
        fprintf (stdout, "%s %s on %s (%s)\nCopyright (c) 2020 Dasoftver LLC",argv[0], VV_VERSION, VV_OS_NAME, VV_OS_VERSION);
        exit(0);
    }

    //
    // parse input parameters
    //
    num i;
    char *out_file_name = NULL;
    char *_item = NULL;
    num _main = 0;
    for (i = 1; i < argc; i ++)
    {
        if (!strcmp (argv[i], "-main"))
        {
            _main = 1;
        }
        else if (!strcmp (argv[i], "-trace"))
        {
            if (i + 1 >= argc)
            {
                _vely_report_error ( "Trace (0 or 1) not specified after -trace option");
                exit (1);
            }
            vely_is_trace = atoi (argv[i+1]);
            i++; // skip db location now
            continue;
        }
        else if (!strcmp (argv[i], "-app-path"))
        {
            if (i + 1 >= argc)
            {
                _vely_report_error ( "URL application prefix not specified after -app-path option");
                exit (1);
            }

            app_path = vely_strdup (argv[i+1]);
            i++; // skip app path
            continue;
        }
        else if (!strcmp (argv[i], "-plain-diag"))
        {
            if (i + 1 >= argc)
            {
                _vely_report_error ( "Type of diagnostics not specified after -plain-diag option");
                exit (1);
            }
            if (!strcmp (argv[i+1], "1")) vv_plain_diag = 1;
            i++; // skip diag option
            continue;
        }
        else if (!strcmp (argv[i], "-max-upload"))
        {
            if (i + 1 >= argc)
            {
                _vely_report_error ( "Maxium upload length not specified after -max-upload option");
                exit (1);
            }
            vely_max_upload = atoll (argv[i+1]);
            i++; // skip db location now
            continue;
        }
        else if (!strcmp (argv[i], "-name"))
        {
            if (i + 1 >= argc)
            {
                _vely_report_error ( "Application name not specified after -name option");
                exit (1);
            }
            VV_STRDUP (vely_app_name, argv[i+1]);
            if (vely_is_valid_param_name(vely_app_name) != 1)
            {
                _vely_report_error( "Application name must have only alphanumeric characters and underscore, found [%s]", vely_app_name);
            }
            i++; // skip db location now
            continue;
        }
        else if (!strcmp (argv[i], "-out"))
        {
            if (i + 1 >= argc)
            {
                _vely_report_error ( "Output file not specified after -out option");
                exit (1);
            }
            VV_STRDUP (out_file_name, argv[i+1]);
            i++; // skip db location now
            continue;
        }
        else if (!strcmp (argv[i], "-v"))
        {
            verbose = 1;
        }
        else if (!strcmp (argv[i], "-x"))
        {
            no_vely_line = 1;
        }
        else
        {
            _item = argv[i];
            if (src_file_name != NULL)
            {
                _vely_report_error ( "Only one file name can be specified for processing, already specified [%s]", src_file_name);
                exit (1);
            }
            if (src_file_name != NULL && _main == 1)
            {
                _vely_report_error ( "Cannot specify file name to process [%s], and the -main option to generate program main code. Use one or the other.", src_file_name);
                exit (1);
            }
            src_file_name = _item;
        }
    }

    if (vely_is_trace != 0 && vely_is_trace != 1) 
    {
        _vely_report_error ( "Tracing (-trace option) must be 0 or 1");
        exit (1);
    }
    num upper_limit = (num)1024*1024*1024*1024;
    if (vely_max_upload <1024 || vely_max_upload > upper_limit)
    {
        _vely_report_error ( "Max_upload_size (-max-upload) must be a number between 1024 and %lld", upper_limit);
        exit (1);
    }
    if (vely_app_name == NULL || vely_app_name[0] == 0) 
    {
        _vely_report_error ( "Application name is missing (-name option)");
        exit (1);
    }
    // app path is by default /<app name>
    if (app_path[0] == 0) 
    {
        num namel = (num)strlen (vely_app_name);
        app_path = (char*)vely_malloc (namel + 2); // 1 for ending-null and 1 for /
        snprintf (app_path, namel+2, "/%.*s", (int)namel, vely_app_name);
    }
    vely_is_valid_app_path(app_path);

    vely_bld_dir = (char*)vely_malloc (VV_FILE_NAME_LEN);
    snprintf (vely_bld_dir, VV_FILE_NAME_LEN, "/var/lib/vv/bld/%s", vely_app_name);
    vely_dbconf_dir = (char*)vely_malloc (VV_FILE_NAME_LEN);
    snprintf (vely_dbconf_dir, VV_FILE_NAME_LEN, "/var/lib/vv/%s/app/db", vely_app_name);

    // get home directory
    char cwd[300];
    //
    // Then we get current working directory
    //
    if (getcwd (cwd, sizeof(cwd)) == NULL)
    {
        _vely_report_error ( "Cannot get current working directory, error [%s]", strerror(errno));
        exit (1);
    }
    //
    //
    // We must set various variables used in VELY shared library, for example, global connection data (description, transaction marker, 
    // connection marker). Encryption function is set too, as well as other variable.
    // This is the SAME code as below generated for application. This is because for VELY, we need it in order for db connection etc. to work.
    // And we need it for application too. Here in vely, we don't need it to be static since it's only one run of this process.
    // We can't really put this code in a common place because it spans two projects. Perhaps something could be done with generated code
    // but the effort doesn't seem worth it.
    //
    memset (&vely_dbs, 0, sizeof(vely_dbs));
    vely_get_config()->ctx.db=&vely_dbs;
    totconn = 0;
    vely_get_config()->app.dbconf_dir=vely_dbconf_dir;

    // For any .vely file, we need a list of connections available (in /.../bld/.dbvendors file). Only connections available
    // there can be used in .vely files
    char vely_db_file[300];
    snprintf (vely_db_file, sizeof(vely_db_file), "%s/.dbvendors", vely_bld_dir);

    FILE *f = fopen (vely_db_file, "r");
    // it is okay for f to be NULL - this is if app has no connections, i.e. it does not use databases
    if (f != NULL)
    {
        vely_dbs.conn = NULL; // vely_malloc in add_connection
        char line[200];
        num dbtype;
        while (1)
        {
            if (fgets (line, sizeof (line) - 1, f) != NULL)
            {
                num len = strlen (line);
                vely_trim (line, &len);
                char *eq = strchr (line, '=');
                if (eq == NULL) 
                {
                    _vely_report_error ( "dbvendors file format is incorrect");
                    exit (1);
                }
                *(eq++) = 0;
                if (!strcmp (eq, "mariadb")) dbtype = VV_DB_MARIADB;
                else if (!strcmp (eq, "sqlite")) dbtype = VV_DB_SQLITE;
                else if (!strcmp (eq, "postgres")) dbtype = VV_DB_POSTGRES;
                else 
                {
                    _vely_report_error ( "Unsupported database type [%s]", eq);
                    exit (1);
                }
                add_connection (line, dbtype); // add connection one by one, none is actually found since the list was empty
            } else break;
        }
        fclose (f);
    }


    if (out_file_name != NULL)
    {
        outf = vely_fopen (out_file_name, "w");
        if (outf == NULL)
        {
            _vely_report_error ( "Cannot open output file [%s] for writing, error [%s]", out_file_name, strerror(errno));
            exit (1);
        }
    }


    if (_main == 0 && src_file_name == NULL)
    {
        _vely_report_error ( "Neither -main option or the file name to process is specified.");
        exit (1);
    }


    if (_main == 1)
    {

        // Generate C code
        // 
        // NOTE: NOTHING IN THIS MAIN CODE CAN HAVE VV_MALLOC OR SUCH - that memory is release at the end of the request, thus anything that needs to be 
        // global must be C's malloc etc.
        //
        VV_VERBOSE(0,"Generating main code");

        oprintf("#include \"vely.h\"\n");
        oprintf("extern num vely_end_program;\n");

        // 
        // code can be generated for standalone (program) version that can be executed from command line
        // or a web-server plug-in for the web
        //

        //
        // ANY variables used in main MUST be outside, or longjmp will destroy them (they cannot be automatic within the function)
        //
        oprintf("volatile num vely_in_request=0;\n");
        oprintf("volatile num vely_done_setjmp=0;\n");
        oprintf("volatile num vely_done_err_setjmp=0;\n");
        oprintf("volatile num vely_in_fatal_exit=0;\n");
        oprintf("sigjmp_buf vely_jmp_buffer;\n");
        oprintf("sigjmp_buf vely_err_jmp_buffer;\n");
        oprintf("char * vely_app_name=\"%s\";\n", vely_app_name);
        // default application path is /<app name>
        oprintf("char * vely_app_path=\"%s\";\n", app_path);
        oprintf("num vely_is_trace=%lld;\n", vely_is_trace);
        oprintf("num vely_max_upload=%lld;\n", vely_max_upload);
        // The following are static variables, i.e. those that need not be seen outside this module
        oprintf ("static vely_db_connections vely_dbs;\n");
        oprintf("static num vv_done_init=0;\n");
        oprintf("static vely_input_req *vv_req;\n");
        oprintf("static vely_config *vv_pc;\n");

        oprintf("int main (int argc, char *argv[])\n");
        oprintf("{\n");
        // vv_done_init and allocation of config structure are done once per program run
        // this also reads config file
        oprintf("vv_pc = vely_alloc_config ();\n");
        oprintf("vely_get_runtime_options();\n");

        //
        // trace_vely must be the very first one, or tracing won't work
        //
        oprintf("if (vv_pc->debug.trace_level != 0) vely_open_trace();\n");


        oprintf ("while(vely_FCGI_Accept() >= 0) {\n");
        oprintf ("vely_in_request = 1;\n");
        oprintf("VV_UNUSED (argc);\n");
        oprintf("VV_UNUSED (argv);\n");


        // BEFORE anything, initialize memory for this request - FCGI only. We need to release memory at the end of FCGI loop, so it doesn't hang there when iactive
        // for FCGI, we must initialize in the very first loop, but not later, since we're going it also at the end of the loop
        oprintf("if (vv_done_init == 0) vely_memory_init();\n");

        // user/group-only permissions
        oprintf("if (!vv_done_init) umask (007);\n");
        oprintf("vely_get_tz ();\n");

        oprintf("if (!vv_done_init) {\n"); 
        // first memset base struct, then connections (not the other way around as conn would be wiped out)
        oprintf("   memset (&vely_dbs, 0, sizeof(vely_dbs));\n");
        if (totconn == 0) oprintf("   vely_dbs.conn = NULL;\n"); else oprintf("   vely_dbs.conn = calloc (%lld, sizeof(one_db));\n", totconn);
        oprintf("   vv_pc->ctx.db = &vely_dbs;\n");
        oprintf("   vv_pc->ctx.db->ind_current_db=-1;\n"); // no db used yet
        // Generate name of database connections
        for (i = 0; i < totconn; i++)
        {
            oprintf ("   vv_pc->ctx.db->conn[%lld].db_name = \"%s\";\n", i, vely_get_config()->ctx.db->conn[i].db_name);
            oprintf ("   vv_pc->ctx.db->conn[%lld].db_type = %lld;\n", i, vely_get_config()->ctx.db->conn[i].db_type);
            oprintf ("   vv_pc->ctx.db->conn[%lld].dbc = NULL;\n", i); // not connected yet
            oprintf ("   vv_pc->ctx.db->conn[%lld].exit_on_error = 1;\n", i); // all db errors fatal
        }
        oprintf ("  vv_pc->ctx.tot_dbs = %lld;\n", totconn); // all db errors fatal
        oprintf("}\n");

        //
        // Setup crash handler. Also print out all shared libraries loaded and their start/end addresses.
        //
        oprintf("if (!vv_done_init) vely_set_crash_handler (vv_pc->app.trace_dir);\n");

        //
        // Initialize curl
        //
        oprintf("#ifdef VV_CURL_USED\nif (!vv_done_init) curl_global_init(CURL_GLOBAL_ALL);\n#endif\n");

        //
        // Initialize crypto
        //
        oprintf("#ifdef VV_CRYPTO_USED\nif (!vv_done_init) vely_sec_load_algos();\n#endif\n");

        // start request anew
        oprintf("vely_reset_config (vv_pc);\n");

        // need to startprint, it is automatic with first vely_printf

        oprintf("vv_req = (vely_input_req*)vely_malloc (sizeof (vely_input_req));\n");
        oprintf("vely_init_input_req(vv_req);\n");

        oprintf("vv_pc->ctx.req = vv_req;\n"); 
        oprintf("vv_req->app = &(vv_pc->app);\n");


        oprintf("VV_TRACE (\"STARTING REQUEST [%%s]\", vv_pc->app.trace_dir);\n");


        // input parameter(s) from the FCGI process manager (vfcgi),  but present for standalone too
        oprintf ("vv_pc->ctx.req->args.num_of_args = argc - 1;\nvv_pc->ctx.req->args.args = argv + 1;\n");

        // After startup, it must be Vely memory, even if set to true in _startup, so that first request is all Vely mem regardless
        // Assign global process data to be NULL before _startup()
        oprintf("if (!vv_done_init) { vv_pc->ctx.data = NULL; _startup(); vely_mem_os = false; }\n");

        // Anything that should have been done before the first request
        // one, MUST have been done by now.
        oprintf("vv_done_init=1;\n");

        // This is if there was an error caught in the request, go to handler after all requests
        oprintf("num ret_val = sigsetjmp(vely_err_jmp_buffer, 1);\n");
        oprintf("if (ret_val != 0) goto end_point;\n");
        oprintf("vely_done_err_setjmp = 1;\n");

        // this gets input from either web server or from the environment
        // We check if return is 1 or 0. If not 1, this means there was a problem, but it was
        // handled (such as Forbidden reply) (otherwise there would be an erorring out). If return
        // value is 0, we just go directly to vely_shut() to flush the response out.
        //
        oprintf("if (vely_get_input(vv_req, NULL, NULL) == 1)\n");
        oprintf("{\n");
        // main function that handles everything - programmer must implement this or use generated
        oprintf("vely_dispatch_request();\n");

        // if there is a transaction that wasn't explicitly committed or rollbacked
        // rollback it and error out, as this is a programming bug.
        oprintf("vely_check_transaction (0);\n");
        oprintf("}\n");

        // we can jump here and vv_req will be valid because long jump to above (where we goto end_point) is valid only between above sigsetjmp and below
        // setting of vely_done_err_setjmp to 0; outside of this block of code we just _Exit()
        oprintf("end_point:\n");
        // If we jumped to here because of a report-error, memory handling must return to Vely
        oprintf("vely_mem_os = false;\n");


        //
        // vely_shut MUST ALWAYS be called at the end - no request can bypass it
        //
        oprintf("vely_shut(vv_req);\n");
        // now we now at this point vv_req is valid; if it was NULL, vely_shut would exit

        // by setting vely_done_err_setjmp to 0 here, we make sure that report-error will jump to 
        // end-point only between above setting vely_done_err_setjump to 1 
        // and here where we set it to 0. Because vv_req is valid in this case, we can do vely_shut after end_point:
        oprintf("vely_done_err_setjmp = 0;\n");

        // All data must be flushed at the end of the request, no none is outstanding somehow later
        oprintf("vely_FCGI_Finish();\n"); 
        // if vely_end_program set to 1:
        // All db connections (vely_end_all_db) will be closed and program exists
        // For web program, we exit here when program catches SIGTERM, and the exit code is always 0.
        // For command line, we use exit-code value, if set.
        oprintf("if (vely_end_program == 1) { vely_end_all_db(); vely_exit ();}\n"); // if SIGHUP or SIGTERM happened
        oprintf ("vely_in_request = 0;\n");
        //
        // vely_memory_init() MUST be at the very end befoe the loop goes back to the next request
        // if it were anywhere before (it used to be before vely_FCGI_Finish()), the memory gets released
        // and all references fail with random values or sigsegv (for instance req->ec would be like that in vely_FCGI_Finish).
        // DO NOT PLACE ANY CODE BETWEEN vely_memory_init() and <curlybrace> that ends the request processing
        //
        oprintf("vely_memory_init();\n");
        oprintf("}\n");
        oprintf("return 0;\n");

        oprintf("}\n");

        VV_VERBOSE(0,"End generating main code");
    }
    else
    {
        VV_VERBOSE(0,"Generating code for [%s]", src_file_name);
        vely_gen_c_code (gen_ctx, src_file_name);
    }

    oprintf("// END OF GENERATED CODE\n");
    oprintf (NULL); //  flush output
    if (outf != NULL) fclose (outf);

    // release of memory done automatically by the OS and likely faster, so no
    // vely_done();
    return 0;
}


