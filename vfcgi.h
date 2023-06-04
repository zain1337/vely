// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC.

// 
// Include file for VELY FastCGI client
//

#ifndef _VV_FCGI_INC

#define _VV_FCGI_INC

// Include basics
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>


// Version+Release. We use major plus minor plus release, as in 1.3.34,2.1.11,3.7.41... 
#define VV_VERSION "17.0.0"

// Client error codes
#define VV_OKAY 0 // success
#define VV_FC_ERR_SOCK_READ -1 // socket read
#define VV_FC_ERR_PROT_ERR -2  // protocol error
#define VV_FC_ERR_BAD_VER -3 // bad fcgi version
#define VV_FC_ERR_SRV -4 // server cannot complete request
#define VV_FC_ERR_UNK -5 // server says unknown type
#define VV_FC_ERR_OUT_MEM -6 // out of memory
#define VV_FC_ERR_RESOLVE_ADDR -7 //ip cannot be resolved
#define VV_FC_ERR_PATH_TOO_LONG -8 //sock path too long
#define VV_FC_ERR_CONNECT -9 //cannot connect
#define VV_FC_ERR_TIMEOUT -10 // timed out
#define VV_FC_ERR_SOCK_WRITE -11 // error writing to server
#define VV_FC_ERR_INTERNAL -12 // not used yet
#define VV_FC_ERR_ENV_TOO_LONG -13 // env too long
#define VV_FC_ERR_BAD_TIMEOUT -14 // invalid timeout
#define VV_FC_ERR_ENV_ODD -15 // number of env vars must be even
#define VV_FC_ERR_SOCKET -16 //cannot create socket
#define VV_FC_ERR_TOTAL -16

// Each protocol must have its own set of types and functions
// here it's vv_fc*. They would be abstracted in Vely as server-*
// but types/functions must be separated. server-* would have 
// "type" clause to differentiate and C declaration would be vv_fc,
// vv_xy etc.
typedef struct vv_fc vv_fc;

// Hooks
typedef void (*vv_fc_out_hook)(char *recv, int recv_len, vv_fc *req);
typedef void (*vv_fc_err_hook)(char *err, int err_len, vv_fc *req);
typedef void (*vv_fc_done_hook)(char *recv, int recv_len, char *err, int err_len, vv_fc *req);

// FastCGI request
typedef struct vv_fc {
    char *fcgi_server; // the IP:port/socket_path to server
    char *req_method; // request method
    char *app_path; // application path
    char *req; // request name
    char *url_payload; // URL payload (path+query string)
    char *content_type; // content type
    int content_len; // content len
    char *req_body; // request body (i.e. content)
    char **env; // environment to pass into request on server side
    int timeout; // timeout for request
    int req_status; // status of request from server
    int data_len; // length of response from server
    int error_len; // length of error from server
    char *other; // actual response from server, other kind (not stdout or stderr)
    int other_len; // length of response from server for other kind
    char *errm; // error message when vv_fc_request returns other than VV_OKAY
    vv_fc_out_hook out_hook; // get output data as soon as it arrives
    vv_fc_err_hook err_hook; // get error data as soon as it arrives
    vv_fc_done_hook done_hook; // get all data when request is complete
    int thread_id; // thread ID when executing in a multithreaded fashion
    volatile char done; // true if request completed. This is in addition to req_status (as some servers
               // may not set it) and read_status (as a request may fail at writing prior to reading)
               // false if request did not complete. "Complete" means done, no matter how, even with failure.
               // It's volatile because it is generally meant to be set by a running thread, but read by a 
               // controlling thread
    int return_code; // the return code from vv_fc_request(), either VV_OKAY or error code, usefule
                     // when multithreaded, in single threaded not really.
    struct s_internal  // these are internal members, do NOT use them directly!!
    {
        char invalid_thread; // 1 if thread that was supposed to be created to process this request is invalid
                         // eg. pthread_create() failed. 0 if okay.
        char *data; // actual response from server
        char *error; // error message from server
        int read_status; // status of reading from server
    } internal;
} vv_fc;


// API
int vv_fc_request (vv_fc *fc_in);
char *vv_fc_error (vv_fc *callin);
char *vv_fc_data (vv_fc *callin);
void vv_fc_delete (vv_fc *callin);

#endif
