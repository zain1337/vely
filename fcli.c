// SPDX-License-Identifier: EPL-2.0
// Copyright 2017 DaSoftver LLC.

// FastCGI client library. It's part of Vely for the corresponding call* statements
// and it is also a standalone library API for writing FastCGI clients in C.

// General includes
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <string.h>
#include <sys/param.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/time.h>
// FastCGI includes
#include "fcgi_config.h"
#include "fcgiapp.h"
#include "fastcgi.h"
#include "fcgios.h"
// Public API prototypes
#include "vfcgi.h"

// If memory allocation fails in Vely, this is handled by vely_* functions
// For a FCGI client, the handling is explicit here in code
#if VV_APPMAKE==1
#define vv_malloc vely_malloc
#define vv_calloc vely_calloc
#define vv_realloc vely_realloc
#define vv_strdup vely_strdup
#define vv_free vely_free
#define VV_ONE_BYTE VV_EMPTY_STRING
#else
#define vv_malloc malloc
#define vv_calloc calloc
#define vv_realloc realloc
#define vv_strdup strdup
#define vv_free free
#define VV_ONE_BYTE calloc(1,1)
#endif

// Fcgi end request (the body)
typedef struct {
    unsigned char app_status3;
    unsigned char app_status2;
    unsigned char app_status1;
    unsigned char app_status0;
    unsigned char status; // protocol status
    unsigned char future_use[3];
} fc_end_req_body;

// Fcgi begin request (the body)
typedef struct {
    unsigned char role_1;
    unsigned char role_0;
    unsigned char flags;
    unsigned char future_use[5];
} fc_begin_req_body;

// Fcgi request header (body follows afterwards), and an empty header (cont_len=0) marks the 
// end of a stream
typedef struct {
    unsigned char ver;
    unsigned char type;
    unsigned char req_id_1;
    unsigned char req_id_0;
    unsigned char cont_len_1;
    unsigned char cont_len_0;
    unsigned char pad_len;
    unsigned char future_use;
} fc_header;


// Defines
#define VV_UNUSED(x) (void)(x)
#define VV_FC_RESPONDER 1 // responder role
#define VV_FC_VER_1 1 // version of fcgi
#define VV_FC_BEG_REQ 1 // begin request
#define VV_FC_END_REQ 3 // end request
#define VV_FC_PARAM 4 // env params record
#define VV_FC_STDIN 5 // stdin to server
#define VV_FC_STDOUT 6 // stdout from server
#define VV_FC_STDERR 7 // stderr from server
#define VV_FC_GET_VAL_RES 10 // not used
#define VV_FC_UNK_TYPE 11 // unknown
#define VV_FC_REQ_DONE 0 // request done record
#define VV_FC_REQ_OVERLOADED 2 // not used
#define VV_FC_MAX_LEN 0xffff // maxium record size


// Local variables used among various functions, more since this is even driven
// These are thread-based, each thread has a copy
typedef struct {
    int param_len; // total length of environment passed to server
    char *env; // one large chunk for all of environment, packed in
    int *env_nlen; // lengths of names in env 
    int *env_vlen; // lengths of values in env
    int env_p; // position where we're writing in env
    int sock;  // socket for sending to server
    bool done_header;  // was header read (true) or not (false)
    int msg_remain;  // how many bytes remains to be read in message
    int padding_len;     // padding length (see fastcgi spec), we don't use it
    int req_id;  // req id of a request, we set to 1 always
    int read_done; // true when reading is done, i.e. the number of bytes sent from server is received
    const char *errm[-VV_FC_ERR_TOTAL + 1]; // an array of error messages
    struct timeval tout; // timeout seconds, microseconds
    struct timeval remaining; // time remaining processing request
    struct timeval begin; // time when request began
    volatile bool interrupted; // true when interrupted
    vv_fc *fc; // input/output data
    bool initerrm; // if error messages initialized (true), otherwise false
} fc_local;



// Local Function prototypes
static int vv_write_socket(fc_local *fc_l, const char *buf, int len);
static int vv_read_socket(fc_local *fc_l, char *buf, int len);
static void shut_sock(fc_local *fc_l);
static void vv_size_env(fc_local *fc_l, int name_len, int val_len);
static int vv_send_env(fc_local *fc_l, const char *name, int name_len, const char *val, int val_len);
static void fc_server_read(fc_local *fc_l);
static void check_read(fc_local *fc_l, int read_len);
static int vv_connect_socket(fc_local *fc_l, const char *conn_string);
static int fc_edge(fc_local *fc_l, int timeout, bool start);
static fc_header build_hdr(fc_local *fc_l, int content_len, int type);
static bool vv_check_timeout(fc_local *fc_l, bool startup);
static bool vv_set_timeout(fc_local *fc_l);
static void begin_request(fc_begin_req_body *beg_req);


//
// Functions
//


//
// Read data from socket. 
// buf is data, len is length to read.
// Returns len if read okay or -1 if could not read.
//
int vv_read_socket(fc_local *fc_l, char *buf, int len)
{
    int res;
    int gotten = 0;
    while (true)
    {
        // read can return -1, 0 or >0
        // -1 is error. 0 is server shutdown orderly. >0 is data read and for blocking can be 1..len
        res = read(fc_l->sock, buf+gotten, len-gotten);
        // If -1 or 0, get out of here, error in reading
        if (res <= 0) 
        { 
            if (errno == EINTR) continue; // if call interrupted, continue
            if (!vv_check_timeout(fc_l, false)) return -1; 
            check_read (fc_l, res); // this checks for negative, 0 
            return res;
        }
        gotten += res;
        if (gotten >= len) break; // if gotten message (>= instead of == just in case), get out
        // if not the whole message received, check if we got partial due to timeout and exit with
        // setting interrupted flag
        if (!vv_check_timeout(fc_l, false)) return -1;
        if (!vv_set_timeout(fc_l)) { check_read(fc_l, -1); return -1;}
    }
    return len;
}

//
// Write data to socket. 
// buf is data, len is length to send.
// Returns len if sent okay or -1 if could not write. 
//
int vv_write_socket(fc_local *fc_l, const char *buf, int len)
{
    int res;
    int written = 0;
    while (true)
    {
        // Write data
        res = write(fc_l->sock, buf+written, len-written);
        // If error
        if (res < 0) 
        {
            if (errno == EINTR) continue; // if call interrupted, continue
            // Check if timeout
            if (!vv_check_timeout(fc_l, false)) return -1; 
            return -1; // either way return -1, the vv_check_timeout will set timeout flags
        }
        written += res;
        if (written >= len) break; // check if all message sent
        if (!vv_check_timeout(fc_l, false)) return -1; 
        if (!vv_set_timeout(fc_l)) return -1;
    }
    return len;
}


//
// Make a begin-request record for the server. Connection is not kept open.
// The role is VV_FC_RESPONDER.
//
void begin_request(fc_begin_req_body *beg_req)
{
    beg_req->flags  = 0; // do not keep connection open
    beg_req->role_1 = (VV_FC_RESPONDER >>  8) & 0xff;
    beg_req->role_0 = VV_FC_RESPONDER & 0xff;
    memset(beg_req->future_use, 0, sizeof(beg_req->future_use));
}



//
// Build a header for request. content_len is the length of message, type is either
// begin-request or the input to server.
//
fc_header build_hdr(fc_local *fc_l, int content_len, int type)
{
    fc_header header;
    header.cont_len_1 = (content_len >> 8) & 0xff;
    header.cont_len_0 = content_len & 0xff;
    header.type = type;
    // request id is always 1, higher protocol should serialize if needed
    header.req_id_1 = 0;
    header.req_id_0 = fc_l->req_id;
    header.pad_len = 0;
    header.ver = VV_FC_VER_1;
    header.future_use = 0;
    return header;
}

//
// Close socket to server.
//
void shut_sock(fc_local *fc_l)
{
    if (fc_l->sock != -1) 
    {
        close(fc_l->sock);
    }
    fc_l->sock = -1;
}

//
// When reading from server, check length of data read. This indicates problem
// with connection or success. read_len is the data read.
// fc_l->fc->read_status is set to negative if there's a problem.
// Returns true if okay, false if problem.
//
void check_read(fc_local *fc_l, int read_len)
{
    if (read_len < 0) { fc_l->fc->read_status = VV_FC_ERR_SOCK_READ; return; } // error in reading from server

    if (read_len == 0)
    {
        // no bytes available any more, this may be some kind of error, probably closed on
        // server side, close the socket first before returning
        // this is when no more bytes to read, but we haven't reached the end of header or message or padding
        if (fc_l->done_header || fc_l->msg_remain > 0 || fc_l->padding_len > 0) 
        {
            fc_l->fc->read_status = VV_FC_ERR_PROT_ERR;
            return;
        }
        //
        // spurious zero-length read should not happen, but it's not clear if they sometime do, so
        // just in case, we already read the message, and so do not declare error
        //
        if (!fc_l->read_done) fc_l->fc->read_status = VV_FC_ERR_PROT_ERR;
        return; // done here
    }
}


//
// Read data from the server (this is a client). 
// fc_l->fc->read_status is VV_FC_ERR_SOCK_READ if error reading, VV_FC_ERR_PROT_ERR if protocol error, VV_FC_ERR_BAD_VER if bad fcgi version.
// VV_FC_ERR_SRV server cannot complete request, VV_FC_ERR_UNK server says unknown type, VV_FC_ERR_OUT_MEM if out of memory,
// 0 if okay
//
void fc_server_read(fc_local *fc_l)
{
    fc_header header; // header of a packet

    // We get all the data we want here, this is not event driven, so all variables (such as header)
    // can remain local.
    while (true)
    {
        // Check if header not complete yet first, cannot get message until header is in
        if (fc_l->done_header) 
        {
            // This is when the header is received, now it's message.
            if (fc_l->msg_remain > 0) // this is the end of stream record for header.type
                                 // nothing to read here if fc_l->msg_remain == 0
            {
                char *inbuff;
                // get the buffer to read into
                if (header.type == VV_FC_STDOUT) inbuff = fc_l->fc->data+fc_l->fc->data_len;
                else if (header.type == VV_FC_STDERR) inbuff = fc_l->fc->error+fc_l->fc->error_len;
                else inbuff = fc_l->fc->other+fc_l->fc->other_len;

                // read data, this reads all of it specified (msg_remain)
                int cnt = vv_read_socket (fc_l, inbuff, fc_l->msg_remain);
                if (cnt <= 0) return;
                // advance read pointer
                if (header.type == VV_FC_STDOUT) 
                {
                    fc_l->fc->data_len += cnt;
                    if (fc_l->fc->out_hook != NULL) (*(fc_l->fc->out_hook))(inbuff, cnt); // exec out hook if there
                }
                else if (header.type == VV_FC_STDERR) 
                {
                    fc_l->fc->error_len += cnt;
                    if (fc_l->fc->err_hook != NULL) (*(fc_l->fc->err_hook))(inbuff, cnt); // exec err hook if there
                }
                else if (header.type == VV_FC_END_REQ)// end record 
                {
                    //finish output with null for convenience
                    fc_l->fc->data[fc_l->fc->data_len] = 0;
                    fc_l->fc->error[fc_l->fc->error_len] = 0;

                    // end of request
                    if (cnt != sizeof(fc_end_req_body)) 
                    {
                        fc_l->fc->read_status = VV_FC_ERR_PROT_ERR;
                        return;
                    }
                    fc_end_req_body *server_end = (fc_end_req_body*)inbuff; // end of request, when server fulfills the request, it sends this
                    fc_l->read_done = true; // whole reply message from server received
                    if(server_end->status != VV_FC_REQ_DONE) {
                        // this can be VV_FC_REQ_OVERLOADED overloaded only
                        fc_l->fc->read_status = VV_FC_ERR_SRV;
                        return;
                    }
                    fc_l->fc->req_status = (server_end->app_status3 << 24) + (server_end->app_status2 << 16) + (server_end->app_status1 <<  8) + (server_end->app_status0);
                    if (fc_l->fc->out_hook != NULL) (*(fc_l->fc->out_hook))("", 0); // exec out hook if there, end request
                }
                else fc_l->fc->other_len += cnt;

                fc_l->msg_remain = 0; // nothing else to read
            }
            else 
            {
                ; // nothing to do. This is a "zero-length" record that marks the end of the stream.
                  // Note that "end of stream" does not necessarily mean end of message - there could
                  // be stdout, then stderr, then stdout, then stdout, i.e. different streams and multiples
                  // of them in no particular order. VV_FC_END_REQ is the end of message, i.e. end of any
                  // streams in it.
            }
            if (header.type == VV_FC_UNK_TYPE)
            {
                fc_l->fc->read_status = VV_FC_ERR_UNK;
                return;
            }
            //
            // Check if there's padding, vacuum it.
            // If that was all in this record, then the whole request is done,
            // so set fc_l->done_header to true, and we can start with the next request's response.
            // if there's padding, read it and it's discarded. We don't use padding.
            // Note that a record may have msg_remaining==0 but padding_len>0 - rare but it can happen! 
            //
            if(fc_l->padding_len > 0) 
            {
                char padb[256]; // max length of paddding is 255
                int cnt = vv_read_socket (fc_l, padb, fc_l->padding_len);
                if (cnt <= 0) return;
                fc_l->padding_len = 0;
            }
            fc_l->done_header = false; // all data read, move on to the next header
        }
        else
        {
            // Get the header first. We read the entire header at once.
            int cnt = vv_read_socket (fc_l, (char*)&header, sizeof(header));
            if (cnt <= 0) return;

            if (header.ver != VV_FC_VER_1) { fc_l->fc->read_status = VV_FC_ERR_BAD_VER; return; }

            if (header.req_id_0 + (header.req_id_1 << 8) != fc_l->req_id) 
            { // must be 1 as req ID
                fc_l->fc->read_status = VV_FC_ERR_PROT_ERR;
                return;
            }
            
            fc_l->msg_remain = header.cont_len_0 + (header.cont_len_1 << 8); // msg size to get next
            fc_l->padding_len =  header.pad_len; // padding to get afterwards

            // allocate memory for the reply and/or error, depending on what this header is
            if (header.type == VV_FC_STDOUT) 
            {
                // Originally we set fc_l->fc->data to calloc(1,1) or VV_EMPTY_STRING so it can be re-alloced
                // Initially fc_l->fc->data_len is 0
                fc_l->fc->data = vv_realloc(fc_l->fc->data, fc_l->fc->data_len+fc_l->msg_remain+1);
                if (fc_l->fc->data == NULL)
                {
                    fc_l->fc->read_status = VV_FC_ERR_OUT_MEM;
                    return;
                }
            }
            else if (header.type == VV_FC_STDERR) 
            {
                // Originally we set fc_l->fc->error to calloc(1,1) or VV_EMPTY_STRING so it can be re-alloced
                // Initially fc_l->fc->error_len is 0
                fc_l->fc->error = vv_realloc(fc_l->fc->error, fc_l->fc->error_len+fc_l->msg_remain+1);
                if (fc_l->fc->error == NULL) 
                {
                    fc_l->fc->read_status = VV_FC_ERR_OUT_MEM;
                    return;
                }
            }
            else
            {
                // Originally we set fc_l->fc->other to calloc(1,1) or VV_EMPTY_STRING so it can be re-alloced
                // Initially fc_l->fc->other_len is 0
                // This is the buffer we get VV_FC_END_REQ into as well.
                fc_l->fc->other = vv_realloc(fc_l->fc->other, fc_l->fc->other_len+fc_l->msg_remain+1);
                if (fc_l->fc->other == NULL) 
                {
                    fc_l->fc->read_status = VV_FC_ERR_OUT_MEM;
                    return;
                }
            }
            fc_l->done_header = true; // done with header, now read data
        } 

        // exit loop if reply from server received
        if (fc_l->read_done) break; 
    } 
    return; // this is when message read okay
}

void vv_size_env(fc_local *fc_l, int name_len, int val_len)
{
    if (name_len < 0x80) fc_l->param_len++; else fc_l->param_len += 4; 
    if (val_len < 0x80) fc_l->param_len++; else fc_l->param_len += 4; 
    fc_l->param_len += name_len;
    fc_l->param_len += val_len;
}

//
// Send environment to server, this is done prior to sending a request. name/name_len are env var name and its
// length, val/val_len are its value and length. fcgi_inp_param is the stream that is open to the server.
// Returns number of bytes written or -1 if failed;
//
int vv_send_env(fc_local *fc_l, const char *name, int name_len, const char *val, int val_len)
{
    // First, setup a header, name, then value lengths
    unsigned char hdr[8+1]; // max possible length of header, plus 1
    int hdr_len = 0;
    if (name_len < 0x80) 
    {
        hdr[hdr_len++] = name_len;
    }
    else 
    {
        hdr[hdr_len++] = (name_len >> 24) | 0x80;
        hdr[hdr_len++] = name_len >> 16;
        hdr[hdr_len++] = name_len >> 8;
        hdr[hdr_len++] = name_len;
    }
    if (val_len < 0x80) 
    {
        hdr[hdr_len++] = val_len;
    }
    else 
    {
        hdr[hdr_len++] = (val_len >> 24) | 0x80;
        hdr[hdr_len++] = val_len >> 16;
        hdr[hdr_len++] = val_len >> 8;
        hdr[hdr_len++] = val_len;
    }

    // Then send header, followed by name and value    
    memcpy (fc_l->env + fc_l->env_p, (char*)hdr, hdr_len);
    fc_l->env_p += hdr_len;
    memcpy (fc_l->env + fc_l->env_p, (char*)name, name_len);
    fc_l->env_p += name_len;
    memcpy (fc_l->env + fc_l->env_p, (char*)val, val_len);
    fc_l->env_p += val_len;
    return fc_l->env_p;
}

//
// Set timeout for socket according to time remaining. If cannot do it (socket disconnected for
// instance), return false, otherwise true;
//
bool vv_set_timeout(fc_local *fc_l)
{
    if (fc_l->fc->timeout == 0) return true;
    if (setsockopt (fc_l->sock, SOL_SOCKET, SO_RCVTIMEO, &(fc_l->remaining), sizeof(fc_l->remaining)) ||
        setsockopt (fc_l->sock, SOL_SOCKET, SO_SNDTIMEO, &(fc_l->remaining), sizeof(fc_l->remaining)))
        return false;
    else return true;
}

//
// Check if time passed since the beginning of the request is up. If it is, return false;
// Otherwise return true. If true, then set recv/send timeout to whatever time is remaining.
// This should be called on startup, before connect and right after any blocking operation (including connect).
// If this is on startup, startup is true.
// We do not check for EAGAIN, EWOULDBLOCK or EINPROGRESS, because if 1) there's an error and time is up or
// 2) there's no error, but not connected, data sent or received, and time is up -- what is the difference?
// We have technically timed out, even if at that very moment some other error happened.
//
bool vv_check_timeout(fc_local *fc_l, bool startup)
{
    if (fc_l->fc->timeout == 0) return true;

    if (startup)
    {
        fc_l->remaining = fc_l->tout; // in the beginning, we have the time
        return true;
    }

    // This is other than startup
    struct timeval curr;
    struct timeval elapsed;
    // Get current time
    gettimeofday (&(curr), NULL);
    // See how much time elapsed
    timersub (&curr, &(fc_l->begin), &elapsed);
    // Is elapsed lesser than timeout
    if (timercmp (&elapsed, &(fc_l->tout) ,<)) 
    {
        // find out remaining time
        timersub(&(fc_l->tout), &elapsed, &(fc_l->remaining));
        // setting socket to block at most the remaining time will be done in vv_set_timeout()
        return true;
    }
    else
    {
        // timeout, it's over
        fc_l->interrupted = true;
        return false;
    }
}

//
// Connect socket to conn_string, which can be IP:port of unix socket.
// If there is : in conn_string, it's IP:port. Unix socket path must not have :
// Return VV_FC_ERR_RESOLVE_ADDR if IP cannot be resolved, VV_FC_ERR_PATH_TOO_LONG if sock path too long, VV_FC_ERR_CONNECT if cannot
// connect or VV_OKAY if okay. Global fc_l->sock is set if socket created; but connect may have failed.
//
int vv_connect_socket(fc_local *fc_l, const char *conn_string)
{
    // we support both IP4 and IP6 addresses
    struct sockaddr_in tcp_sock;
    struct sockaddr_in6 tcp_sock6;
    // this is for Unix sockets
    struct sockaddr_un unix_sock;

    int st;
    char *col;
    char *port = NULL;
    char *hn;
    fc_l->sock = -1; // default when not created
    if ((col = strchr(conn_string, ':')) != NULL) 
    {
        hn = vv_strdup (conn_string);
        if (hn == NULL) { return VV_FC_ERR_OUT_MEM; }
        hn[col-conn_string] = 0;
        port = col+1;
    }
    // Connecting is given all of timeout. It should use just a fraction, but if it doesn't, we
    // will measure what was used and set the remainder for the following operation. 
    if (port != NULL) 
    {
        struct addrinfo* res = NULL;
        struct addrinfo* cres = NULL;
        // Get all possible addresses for hn:port
        if (getaddrinfo(hn, port, 0, &res)) { return VV_FC_ERR_RESOLVE_ADDR; }
        vv_free (hn); // free host name
        int len;
        // Init IP4 and IP6 ports to nothing
        tcp_sock.sin_port = 0;
        tcp_sock6.sin6_port = 0;
        // Find the first address that is INET, be it IPv4 or IPv6. 
        for(cres=res; cres!=NULL; cres=cres->ai_next)
        {
            if (cres->ai_addr->sa_family == AF_INET) 
            {
                memcpy (&tcp_sock, (struct sockaddr_in *)cres->ai_addr, sizeof(struct sockaddr_in));
                len = sizeof(tcp_sock6);
                break;
            }
            else if (cres->ai_addr->sa_family == AF_INET6) 
            {
                memcpy (&tcp_sock6, (struct sockaddr_in6 *)cres->ai_addr, sizeof(struct sockaddr_in6));
                len = sizeof(tcp_sock6);
                break;
            }
        }
        freeaddrinfo (res); // free OS allocated memory for host name resolution
        // Check we have one good address
        if (tcp_sock.sin_port == 0 && tcp_sock6.sin6_port == 0) return VV_FC_ERR_RESOLVE_ADDR;
        // socket is non-blocking
        fc_l->sock = socket(AF_INET, SOCK_STREAM, 0);
        if (fc_l->sock == -1) return VV_FC_ERR_SOCKET;
        if (!vv_check_timeout(fc_l, false)) return VV_FC_ERR_TIMEOUT;
        if (!vv_set_timeout(fc_l)) return VV_FC_ERR_SOCKET; // this shouldn't happen, since we're just starting;
        st = connect(fc_l->sock, tcp_sock.sin_port != 0? (struct sockaddr*)&tcp_sock:(struct sockaddr*)&tcp_sock6, len);
    } 
    else {
        int conn_str_len = strlen(conn_string);
        if(conn_str_len > (int)sizeof(unix_sock.sun_path)) return VV_FC_ERR_PATH_TOO_LONG;
        memset((char *) &(unix_sock), 0, sizeof(unix_sock));
        unix_sock.sun_family = AF_UNIX;
        memcpy(unix_sock.sun_path, conn_string, conn_str_len);
        int len = sizeof(unix_sock);
        fc_l->sock = socket(AF_UNIX, SOCK_STREAM, 0);
        if (fc_l->sock == -1) return VV_FC_ERR_SOCKET;
        if (!vv_check_timeout(fc_l, false)) return VV_FC_ERR_TIMEOUT; 
        if (!vv_set_timeout(fc_l)) return VV_FC_ERR_SOCKET; // this shouldn't happen, since we're just starting;
        st = connect(fc_l->sock, (struct sockaddr*)&unix_sock, len);
    }

    if(st >= 0) 
    {
        return VV_OKAY;
    }
    else 
    {
        if (!vv_check_timeout(fc_l, false)) return VV_FC_ERR_TIMEOUT; 
        shut_sock(fc_l);
        return VV_FC_ERR_CONNECT;
    }
}

//
// Startup/Shutdown. If start is true, this is startup, otherwise shutdown.
// timeout is the timeout for a request
//
int fc_edge(fc_local *fc_l, int timeout, bool start)
{

    if (!fc_l->initerrm) 
    {
        fc_l->initerrm = true;
        // Error messages
        fc_l->errm[-VV_FC_ERR_SOCK_READ] = "Error reading from server";
        fc_l->errm[-VV_FC_ERR_PROT_ERR] = "Protocol error";
        fc_l->errm[-VV_FC_ERR_BAD_VER] = "Bad FastCGI version";
        fc_l->errm[-VV_FC_ERR_SRV] = "Server cannot complete request";
        fc_l->errm[-VV_FC_ERR_UNK] = "Unknown record type";
        fc_l->errm[-VV_FC_ERR_OUT_MEM] = "Out of memory";
        fc_l->errm[-VV_FC_ERR_RESOLVE_ADDR] = "Cannot resolve host address";
        fc_l->errm[-VV_FC_ERR_PATH_TOO_LONG] = "Unix socket path too long";
        fc_l->errm[-VV_FC_ERR_CONNECT] = "Cannot connect to server";
        fc_l->errm[-VV_FC_ERR_TIMEOUT] = "Request timed out";
        fc_l->errm[-VV_FC_ERR_SOCK_WRITE] = "Error writing to server";
        fc_l->errm[-VV_FC_ERR_INTERNAL] = "Internal error";
        fc_l->errm[-VV_FC_ERR_ENV_TOO_LONG] = "Environment variables too long";
        fc_l->errm[-VV_FC_ERR_BAD_TIMEOUT] = "Invalid timeout value";
        fc_l->errm[-VV_FC_ERR_ENV_ODD] = "Name or value missing in environment variables";
    }

    if (start) 
    {
        // This is initialization
        //
        // In order to restore a signal handler properly, do NOT place any returns in between here and
        // the line stating ENDINIT
        //
        // initialize reading stats, in case we return prior to reading so they have default values
        fc_l->fc->read_status = VV_OKAY; // by default, read succeeds, we set error if it doesn't
        fc_l->fc->data = VV_ONE_BYTE;
        if (fc_l->fc->data == NULL) { return VV_FC_ERR_OUT_MEM; }
        fc_l->fc->data_len = 0;
        fc_l->fc->error = VV_ONE_BYTE;
        if (fc_l->fc->error == NULL) { return VV_FC_ERR_OUT_MEM; }
        fc_l->fc->error_len = 0;
        fc_l->fc->other = VV_ONE_BYTE;
        if (fc_l->fc->other == NULL) { return VV_FC_ERR_OUT_MEM; }
        fc_l->fc->other_len = 0;
        fc_l->read_done = false;
        fc_l->env = NULL;
        fc_l->env_nlen = NULL;
        fc_l->env_vlen = NULL;
        fc_l->sock = -1;  // socket for sending to server
        fc_l->req_id = 1;  // id of request
        fc_l->done_header = false;  // was header read (true) or not (false)
        // set timeout to "no timeout" by default
        fc_l->tout.tv_sec = 0;
        fc_l->tout.tv_usec = 0;
        fc_l->interrupted = false;
        if (timeout != 0)
        {
            // check if timeout value in range, don't do anythig if not
            if (timeout < 1 || timeout > 86400) return VV_FC_ERR_BAD_TIMEOUT;
            fc_l->tout.tv_sec = timeout;
            fc_l->tout.tv_usec = 0;
            vv_check_timeout(fc_l, true); // arm timer
        }
    }
    else
    {
        shut_sock(fc_l); // close socket if not already closed
        if (fc_l->env != NULL) vv_free(fc_l->env);
        if (fc_l->env_nlen != NULL) vv_free(fc_l->env_nlen);
        if (fc_l->env_vlen != NULL) vv_free(fc_l->env_vlen);
        if (fc_l->fc->other != NULL) vv_free (fc_l->fc->other);
    }
    return VV_OKAY;
}

// Returning from fc_request must annul the alarm, close socket, and account for any timeout.
// This will cancel the alarm and restore original (if any) alarm handler, close socket, and return either the exit code or
// the value for being interrupted. Also set error message;
#define VV_FC_RET(rc) {fc_edge (fc_l, fc_l->fc->timeout, false); int _rc = (fc_l->interrupted?VV_FC_ERR_TIMEOUT:(rc)); if (_rc != VV_OKAY) fc_l->fc->errm = fc_l->errm[-_rc]; else fc_l->fc->errm = ""; return _rc;}
//
// Make a request to FCGI server from a client. 
// fc_l->fc->fcgi_server is 1. the file path to local server or 2. IP:port of the FCGI server
// fc_l->fc->req_method is request method (GET, POST..), fc_l->fc->app_path is PATH_INFO (application path),
// fc_l->fc->req is SCRIPT_NAME (/request_name), fc_l->fc->content_len is the length of request body,
// fc_l->fc->url_payload is the query string or rest params and it can be NULL or empty,
// fc_l->fc->content_type is CONTENT_TYPE (application/json...), fc_l->fc->req_body is the request body,
// fc_l->fc->env is the environment to pass (name,value,name,value..) that ends with NULL, 
// fc_l->fc->timeout is timeout in seconds (1..86400)
// Returns  connect errors (VV_FC_ERR_RESOLVE_ADDR if IP cannot be resolved, VV_FC_ERR_PATH_TOO_LONG if sock path too long, 
// VV_FC_ERR_CONNECT if cannot connect), 
// VV_FC_ERR_SOCK_WRITE error in writing to server, VV_FC_ERR_ENV_TOO_LONG if fc_l->fc->env too long.
// VV_FC_ERR_BAD_TIMEOUT invalid timeout.
// VV_FC_ERR_SOCK_READ if cannot read from server,
// VV_FC_ERR_PROT_ERR if protocol error,
// VV_FC_ERR_BAD_VER if fcgi versions don't match
// VV_FC_ERR_UNK if server doesn't recognize client record types,
// VV_FC_ERR_OUT_MEM if out of memory,
// VV_FC_ERR_ENV_TOO_LONG if environment too long,
// VV_FC_ERR_BAD_TIMEOUT if timeout value lesser than 0 or greater than 86400
// VV_FC_ERR_SRV if server cannot complete request
// If timed out, returns VV_FC_ERR_TIMEOUT
// fc_l->fc->content_type can be NULL, and fc_l->fc->content_len 0. If either is like that, then fc_l->fc->req_body should be NULL, as it won't be accounted for.
// fc_l->fc->env can be NULL. If like that, that no environment is used.
// 
//
int vv_fc_request (vv_fc *fc_in)
{
    // Thread-local internal data
    fc_local _fc_l = {0};
    fc_local *fc_l = &_fc_l;

    gettimeofday(&(fc_l->begin), NULL); // record begin of request so timeout can work properly

    // Input/output request data, must be separate for each thread of course
    fc_l->fc = fc_in;

    // Initialize, this must happen before anything else
    int init = fc_edge(fc_l, fc_l->fc->timeout, true);
    if (init != VV_OKAY) return init;

    struct struct_rec {
        fc_header start_rec_header;
        fc_begin_req_body start_rec_body;
    } srec; // put in structure so we can do a single write instead of 2

    int cres =  vv_connect_socket(fc_l, fc_l->fc->fcgi_server); // connect to Unix or TCP socket
    if (cres != VV_OKAY)  VV_FC_RET(cres);

    // Begin request
    srec.start_rec_header = build_hdr(fc_l, sizeof(srec.start_rec_body), VV_FC_BEG_REQ);
    begin_request(&(srec.start_rec_body));
    // Send beginning of request to server
    if (vv_write_socket(fc_l, (const char *)&srec, sizeof(srec)) < 0) { shut_sock(fc_l); VV_FC_RET(VV_FC_ERR_SOCK_WRITE);}

    // sanity check for content
    // must have all of these to have content
    bool body = true;
    const char *ibody = fc_l->fc->req_body; // use local pointer to avoid changing input data
    if (ibody == NULL || fc_l->fc->content_len == 0 || fc_l->fc->content_type == NULL || fc_l->fc->content_type[0] == 0)
    {
        ibody = NULL;
        fc_l->fc->content_len = 0;
        fc_l->fc->content_type = NULL;
        body = false; // there is no req body
    }

    // Send environment variables to the server, this isn't real sending, just queing it up
    char clen[30];
    // do not calculate lengths more than once
    int clen_len = 0;
    int cont_type_l = 0;
    if (body)
    {
        clen_len = snprintf(clen, sizeof(clen), "%d", fc_l->fc->content_len);
        cont_type_l = strlen(fc_l->fc->content_type);
    }
    int req_method_l = strlen(fc_l->fc->req_method);
    int req_l = strlen(fc_l->fc->req);
    int url_payload_l = 0;
    if (fc_l->fc->url_payload != NULL && fc_l->fc->url_payload[0] !=0) url_payload_l = strlen(fc_l->fc->url_payload);
    int app_path_l = strlen(fc_l->fc->app_path);
    // calculate length of environment
    fc_l->param_len = 0; // init size before calling vv_size_env's
    vv_size_env(fc_l, strlen("REQUEST_METHOD"), req_method_l);
    vv_size_env(fc_l, strlen("SCRIPT_NAME"), app_path_l);
    vv_size_env(fc_l, strlen("PATH_INFO"), req_l);
    if (fc_l->fc->url_payload != NULL && fc_l->fc->url_payload[0] !=0) vv_size_env(fc_l, strlen("QUERY_STRING"), url_payload_l);
    if (body)
    {
        vv_size_env(fc_l, strlen("CONTENT_LENGTH"), clen_len);
        vv_size_env(fc_l, strlen("CONTENT_TYPE"), cont_type_l);
    } 
    int envn = 0;
    if (fc_l->fc->env != NULL)
    {
        while (fc_l->fc->env[envn] != NULL) envn++;
        if (envn % 2 != 0) VV_FC_RET(VV_FC_ERR_ENV_ODD);
        fc_l->env_nlen = vv_calloc (envn/2, sizeof(int));
        fc_l->env_vlen = vv_calloc (envn/2, sizeof(int));
        if (fc_l->env_nlen == NULL || fc_l->env_vlen == NULL) { shut_sock(fc_l); VV_FC_RET (VV_FC_ERR_OUT_MEM); }
        int i;
        for (i = 0; i < envn; i+=2) vv_size_env(fc_l, fc_l->env_nlen[i/2]=strlen(fc_l->fc->env[i]), fc_l->env_vlen[i/2]=strlen(fc_l->fc->env[i+1]));
    }
    if (fc_l->param_len >= (int)(VV_FC_MAX_LEN - sizeof(fc_header))) { shut_sock(fc_l); VV_FC_RET(VV_FC_ERR_ENV_TOO_LONG);}
    // Parameters for the request
    fc_header header = build_hdr(fc_l, fc_l->param_len, VV_FC_PARAM);
    if (vv_write_socket(fc_l, (const char *)&header, sizeof(header)) < 0) { shut_sock(fc_l); VV_FC_RET(VV_FC_ERR_SOCK_WRITE);}
    // send environment, total limit just under 64K (query string limited to 32K in Vely)
    // Build environment string to send
    fc_l->env = vv_malloc (fc_l->param_len);
    if (fc_l->env == NULL) { shut_sock(fc_l); VV_FC_RET (VV_FC_ERR_OUT_MEM); }
    fc_l->env_p = 0;
    vv_send_env(fc_l, "REQUEST_METHOD", strlen("REQUEST_METHOD"), fc_l->fc->req_method, req_method_l);
    vv_send_env(fc_l, "SCRIPT_NAME", strlen("SCRIPT_NAME"), fc_l->fc->app_path, app_path_l);
    vv_send_env(fc_l, "PATH_INFO", strlen("PATH_INFO"), fc_l->fc->req, req_l);
    if (fc_l->fc->url_payload != NULL && fc_l->fc->url_payload[0] !=0) vv_send_env(fc_l, "QUERY_STRING", strlen("QUERY_STRING"), fc_l->fc->url_payload, url_payload_l);
    if (body)
    {
        vv_send_env(fc_l, "CONTENT_LENGTH", strlen("CONTENT_LENGTH"), clen, clen_len);
        vv_send_env(fc_l, "CONTENT_TYPE", strlen("CONTENT_TYPE"), fc_l->fc->content_type, cont_type_l);
    }
    if (fc_l->fc->env != NULL)
    {
        int i;
        for (i = 0; i < envn; i+=2) vv_send_env(fc_l, fc_l->fc->env[i], fc_l->env_nlen[i/2], fc_l->fc->env[i+1], fc_l->env_vlen[i/2]);
    }
    if (vv_write_socket(fc_l, (const char*)(fc_l->env), fc_l->env_p) < 0) { shut_sock(fc_l); VV_FC_RET(VV_FC_ERR_SOCK_WRITE);}
    // End param stream
    header = build_hdr(fc_l, 0, VV_FC_PARAM);
    if (vv_write_socket(fc_l, (const char *)&header, sizeof(header)) < 0) { shut_sock(fc_l); VV_FC_RET(VV_FC_ERR_SOCK_WRITE);}


    if (body)
    {
        int req_body_len = fc_l->fc->content_len; // keep track of how many bytes sent
        // limit of 65535 bytes per packet, so it may have to be split, accounting for FCGI header size
        int curr_packet_len;
        while (req_body_len>0) 
        {
            // first (and possibly last if less than 64K-ish) packet length
            if (req_body_len > VV_FC_MAX_LEN - (int)sizeof(fc_header)) curr_packet_len = VV_FC_MAX_LEN - sizeof(fc_header);
            else curr_packet_len = req_body_len;
            // make header for the packet
            header = build_hdr(fc_l, curr_packet_len, VV_FC_STDIN);
            int res;
            res = vv_write_socket(fc_l, (const char*)&header, sizeof(header));
            if (res < 0) VV_FC_RET(VV_FC_ERR_SOCK_WRITE);
            res = vv_write_socket(fc_l, ibody, curr_packet_len);
            if (res < 0) VV_FC_RET(VV_FC_ERR_SOCK_WRITE);
            req_body_len -= curr_packet_len; // now less to send
            ibody += curr_packet_len;
        }
        // Send an empty record
        header = build_hdr(fc_l, 0, VV_FC_STDIN);
        int res;
        res = vv_write_socket(fc_l, (const char*)&header, sizeof(header));
        if (res < 0) VV_FC_RET(VV_FC_ERR_SOCK_WRITE);
    }

    fc_server_read (fc_l);
    
    // We use connect, request, close method. This is much easier on resources than keeping a connection open (especially
    // on the server which may have to juggle many). For Unix sockets, or local 127.0.0.1, there is really no penalty to speak
    // of, and it eliminates programming issues of keeping and multiplexing connections, which probably exerts more of a penalty
    // to begin with. For fast secure local networks (i.e. no encryption needed, as we don't provide any), there is a  
    // penalty with connect/close but then server has to manage many open connections and may actually slow things down because while
    // some clients may have connected to server process A, server process B may be open and idling. A strategy to overcome this may
    // be costly. The end result can go either way, but for now single connection seems to deliver better results. Perhaps
    // in the future we will support connection reuse on both client and server, but that remains to be seen.
    VV_FC_RET(fc_l->fc->read_status);
}
// Note: valgrind testing with (for tf.c source test):
// valgrind --log-file=$(pwd)/tf.log -s --trace-children=yes --track-origins=yes --verbose  --leak-check=full --show-leak-kinds=all --  ./tf