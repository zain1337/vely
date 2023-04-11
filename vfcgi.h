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


// Version+Release. We use major plus minor plus release, as in 1.3.34,2.1.11,3.7.41... 
#define VV_VERSION "16.7.0"

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

typedef void (*vv_fc_out_hook)(char *recv, int recv_len);
typedef void (*vv_fc_err_hook)(char *recv, int recv_len);

// FastCGI request
typedef struct {
    const char *fcgi_server; // the IP:port/socket_path to server
    const char *req_method; // request method
    const char *app_path; // application path
    const char *req; // request name
    const char *url_payload; // URL payload (path+query string)
    const char *content_type; // content type
    int content_len; // content len
    const char *req_body; // request body (i.e. content)
    char **env; // environment to pass into request on server side
    int timeout; // timeout for request
    int read_status; // status of reading from server
    int req_status; // status of request from server
    char *data; // actual response from server
    int data_len; // length of response from server
    char *error; // error message from server
    int error_len; // length of error from server
    char *other; // actual response from server, other kind (not stdout or stderr)
    int other_len; // length of response from server for other kind
    const char *errm; // error message when vv_fc_request returns other than VV_OKAY
    vv_fc_out_hook out_hook; // get output data as soon as it arrives
    vv_fc_err_hook err_hook; // get error data as soon as it arrives
} vv_fc;


int vv_fc_request (vv_fc *fc_in);

#endif
