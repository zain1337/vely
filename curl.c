// SPDX-License-Identifier: EPL-2.0
// Copyright 2019 DaSoftver LLC.
// Licensed under Eclipse Public License - v 2.0. See LICENSE file.
// On the web https://vely.dev/ - this file is part of Vely framework.

// 
// Curl-related module
//


#include "vely.h"

// Type that defines curl internal data for a single web-call
typedef struct s_curlcall 
{
    CURL *curl; // handle for curl call
    struct curl_slist *hlist; // custom headers
    curl_mime *form; // data for posting a form 
    char **error; // error text of curl call
} curlcall;

static void vely_end_web(char **error, num res, num *tries, curlcall *cc);
static CURLcode vely_process_post (curlcall *cc, char *fields[], char *files[], num *tries);

// Response for web call. Chunked responses are added to the string ptr determined by total dynamic length of len.
//
typedef struct vely_s_url_callback {
    char *ptr;
    size_t len;
} vely_url_data;

void vely_init_url_callback(vely_url_data *s);
void vely_cleanup_curlcall (curlcall *cc);
void vely_init_curlcall (curlcall *cc, char **error);
size_t vely_write_url_callback(void *ptr, size_t size, size_t nmemb, void *s);
num vely_web_set_header (curlcall *cc, num *tries, char *name, char *val);


// Initialize URL response (response received in vely_post_url_with_response(), i.e. when
// calling web service somewhere). 's' is URL response.
//
void vely_init_url_callback(vely_url_data *s)
{
    VV_TRACE("");
    s->len = 0;
    s->ptr = vely_malloc(s->len+1);
    s->ptr[0] = '\0';
}

//
// Write url response data to url response 's'. 'ptr' is the data received as a response from a
// web call in vely_post_url_with_response(), 'size'*'nmemb' is ptr's length. This is curl's callback
// function where pieces of response are channeled here, so this function by nature is cumulative.
// The signature of this function stems of the default curl's handler, which is a write() into a file.
// Returns the number of bytes "written" into our buffer, and that must match the number of bytes
// passed to it.
//
size_t vely_write_url_callback(void *ptr, size_t size, size_t nmemb, void *s)
{
    VV_TRACE("");
    // new len (so far plus what's incoming)
    size_t new_len = ((vely_url_data*)s)->len + size*nmemb;

    // reallocate buffer to hold new len (the input from curl doesn't include zero byte, so +1)
    ((vely_url_data*)s)->ptr = vely_realloc(((vely_url_data*)s)->ptr, new_len+1);

    // append incoming data after current input
    memcpy(((vely_url_data*)s)->ptr+((vely_url_data*)s)->len, ptr, size*nmemb);

    // zero terminate
    ((vely_url_data*)s)->ptr[new_len] = '\0';

    // set new len
    ((vely_url_data*)s)->len = new_len;


    return size*nmemb;
}


//
// End curl call. It must do cleanup, or some interesting crashes may happen
// error is the error returned from web-call, res is the result of curl op, tries is the
// number of tries for redirecting URL, curl is the handle of the call
//
void vely_end_web(char **error, num res, num *tries, curlcall *cc)
{
     if (error != NULL) *error = vely_strdup ((char*)curl_easy_strerror(res));
     *tries = 0;
     vely_cleanup_curlcall(cc);
}

//
// Cleanup after a curl web-call, cc is the curl struct
//
void vely_cleanup_curlcall (curlcall *cc)
{
   if (cc->curl != NULL) curl_easy_cleanup(cc->curl);
   if (cc->form != NULL) curl_mime_free(cc->form);
   if (cc->hlist != NULL) curl_slist_free_all(cc->hlist);
}

//
// Initialize curl web-call, cc is the curl struct, error is the output back to web-call in case of error
//
void vely_init_curlcall (curlcall *cc, char **error)
{
    cc->curl = NULL;
    cc->hlist = NULL; 
    cc->form = NULL; 
    cc->error = error;
}

//
// Set header for curl response. cc is the curl struct, tries is retry number, name/val
// is the header
// returns VV_OKAY or VV_ERR_FAILED.
//
num vely_web_set_header (curlcall *cc, num *tries, char *name, char *val)
{
    VV_TRACE("");
    CURLcode res = CURLE_OK;
    struct curl_slist *tmp;
    char opt[1024];
    snprintf (opt, sizeof(opt), "%s: %s", name, val);
    if ((tmp = curl_slist_append(cc->hlist, opt)) == NULL)
    {
        vely_end_web(NULL, res, tries, cc);
        if (cc->error != NULL) 
        {
            char eopt[1024+200];
            snprintf (eopt, sizeof(eopt), "Cannot add [%s] to header list", opt);
            *(cc->error) = vely_strdup (eopt);
        }
        VERR0;
        return VV_ERR_FAILED;
    }
    cc->hlist = tmp;
    return VV_OKAY;
}

//
// Issue web call, which is url string, and obtain the response in 'result' which will be allocated here.
// 'head' is the header of the response, which if NULL, is not filled
// If there is an SSL certificate (authority for this system having https capability for instance), it is
// specified in 'cert'. Any error by curl is reported in 'error' - this is not response error from web server.
// 'cookiejar' is the file name (full path) of where the cookies are to be stored. Cookies are read from here before
// the call and store here after the call. When communicating with the same server, cookiejar serves the purpose of
// holding all known cookies.
// resp_code is is response code from server (200 for okay, 404 for not found etc.)
// timeout is max # of seconds request can take. It will abort the call after that.
// bodyreq is 1 if this is POST. URL can still have ? and have query params (kind of mix of GET/POST style)
// fields and files are POST fields and files in the form of field,value...; for files, value is file name. Only valid if bodyreq is 1
// vh is request reply, custom options
// 'method' is GET, PUT, POST, PATCH or DELETE (or any other)
// payload is the body content. It's either fields/files or payload, but NOT both. v1
// enforces this in call-web statement. payload_len is its length, or -1 if it's to be strlen-ed.
// Errors (VV_ERR_FAILED, VV_ERR_WEB_CALL) are reported in status as negatives
// Returns number of bytes received if okay, or negative (above) if curl error
// Redirections in response are handled with up to 5 at a time.
//
num vely_post_url_with_response(char *url, char **result, char **head, char **error, char *cert, char *cookiejar, num *resp_code, long timeout, char bodyreq, char *fields[], char *files[], vely_header *vh, char *method, char *payload, num payload_len)
{
    VV_TRACE("URL posted [%s]", url);

    curlcall cc;
    vely_init_curlcall (&cc, error);

    CURLcode res = CURLE_OK;
    num len=0; // length as returned by curl
    num lenhead=0; // length of header as returned by curl
    VV_UNUSED(lenhead);

    // this static is okay because EVERY return sets it to zero. We could do tries-- with each return and
    // technically by the time we get back to the first call (in a series of recursive calls), it should be zero
    // again, but this way we're sure. See code below, we don't do anything with the result other than to pass it up,
    // so the unwind of recursive calls happens without any further action.
    // Also this static is for a single request only.
    static num tries = 0;
    assert (url != NULL);
    assert (result != NULL);

    if (cc.error != NULL && *(cc.error) != NULL) *(cc.error) = VV_EMPTY_STRING;
    if (resp_code != NULL) *resp_code = 0; // default

    // keep track of the depth of recursive calls to this function
    // EVERY RETURN FROM THIS FUNCTION MUST HAVE TRIES-- to keep this correct.
    tries++;
    if (tries>=5)
    {
        tries = 0;
        if (cc.error != NULL) *(cc.error) = vely_strdup ("Too many redirections in URL");
        return VV_ERR_FAILED; // too many redirections followed, error out
    }

    cc.curl = curl_easy_init();
    if (cc.curl)
    {
        vely_url_data sresp;
        vely_url_data shead;
        vely_init_url_callback (&sresp);
        vely_init_url_callback (&shead);

        if (cookiejar!=NULL)
        {
            // the same file is used to read cookies and to write them.
            // CURL will keep this file the same way a browser would do. Cookies
            // can be received from server in one call and then read in another even
            // if they weren't sent in that call.
            curl_easy_setopt(cc.curl, CURLOPT_COOKIEFILE, cookiejar);
            curl_easy_setopt(cc.curl, CURLOPT_COOKIEJAR, cookiejar);
        }

        if (cert == NULL)
        {
            curl_easy_setopt(cc.curl, CURLOPT_SSL_VERIFYHOST, 0);
            curl_easy_setopt(cc.curl, CURLOPT_SSL_VERIFYPEER, 0);
            // this is with-no-cert
        }
        else
        {
            curl_easy_setopt(cc.curl, CURLOPT_SSL_VERIFYHOST, 1);
            curl_easy_setopt(cc.curl, CURLOPT_SSL_VERIFYPEER, 1);
            if (cert[0] == 0)
            {
                // this is default CA authority, installed on system
            }
            else
            {
                // this is with-cert <location> where cert is this location
                curl_easy_setopt(cc.curl, CURLOPT_CAINFO, cert);
            }
        }

        // trace curl call if tracing is on
        vely_config *pc = vely_get_config();
        // trace_level currently only 1
        if (pc->debug.trace_level > 0 && pc->trace.f != NULL)
        {
            // we do not check status of verbose settings, because if it doesn't work, we wouldn't do anything
            if (curl_easy_setopt(cc.curl, CURLOPT_STDERR, pc->trace.f) == CURLE_OK) curl_easy_setopt(cc.curl, CURLOPT_VERBOSE, 1L);
        }

        if ((res = curl_easy_setopt(cc.curl, CURLOPT_URL, url)) != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_FAILED;
        }
        if ((res = curl_easy_setopt(cc.curl, CURLOPT_HEADERFUNCTION, vely_write_url_callback)) != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_FAILED;
        }
        if ((res = curl_easy_setopt(cc.curl, CURLOPT_WRITEFUNCTION, vely_write_url_callback)) != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_FAILED;
        }
        if ((res = curl_easy_setopt(cc.curl, CURLOPT_WRITEDATA, &sresp)) != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_FAILED;
        }
        if ((res = curl_easy_setopt(cc.curl, CURLOPT_HEADERDATA, &shead)) != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_FAILED;
        }
        if (timeout > 86400 || timeout <= 0)
        {
            vely_end_web(NULL, res, &tries, &cc); // don't set error, we set it
            if (cc.error != NULL) *(cc.error) = vely_strdup ("Timeout invalid or too long");
            return VV_ERR_FAILED; // timeout too long
        }
        if ((res = curl_easy_setopt(cc.curl, CURLOPT_TIMEOUT, timeout)) != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_FAILED;
        }

        // set custom header before bodyreq or any header settings done by default, because curl
        // only honors first mention of it, or it doesn't add the second one at all
        if (vh != NULL)
        {
            // Set content type if specified - if specified then it takes precedence
            // as it was specified first
            if (vh->ctype != NULL)
            {
                num r = vely_web_set_header (&cc, &tries, "Content-Type", vh->ctype);
                if (r != VV_OKAY) return r;
            }
            num i = 0;
            while (vh->control[i] != NULL)
            {
                num r = vely_web_set_header (&cc, &tries, vh->control[i], vh->value[i]);
                if (r != VV_OKAY) return r;
                i++;
            }
        }


        if (bodyreq == 1) // body-request specified
        {
            if (fields != NULL || files != NULL)
            {
                if (vely_process_post (&cc, fields, files, &tries) != CURLE_OK)
                {   
                    return VV_ERR_FAILED;
                }
            } else if (payload != NULL)
            {
                // do NOT set CURLOPT_POSTFIELDSIZE to -1 as docs for curl API suggest, results in sigsegv in curl
                // this would have been to have curl strlen() the body, but it does it anyways by default
                if (payload_len != -1) curl_easy_setopt(cc.curl, CURLOPT_POSTFIELDSIZE_LARGE, (curl_off_t)payload_len);
                curl_easy_setopt(cc.curl, CURLOPT_POSTFIELDS, payload);
            } else 
            {
                vely_end_web(NULL, res, &tries, &cc); // don't set error, we set it
                if (cc.error != NULL) *(cc.error) = vely_strdup ("Internal: cannot use files/fields and body-request at the same time");
                return VV_ERR_FAILED; // timeout too long
            }
        }

        // setting headers must be after vely_process_post() so that any headers added/changed there are
        // taken into account
        if (cc.hlist != NULL) curl_easy_setopt(cc.curl, CURLOPT_HTTPHEADER, cc.hlist);

        // setting method & content type for request body is last - it overrides any method/content type already set in headers
        if (method != NULL && method[0] != 0) curl_easy_setopt(cc.curl, CURLOPT_CUSTOMREQUEST, method);

        res = curl_easy_perform(cc.curl);
        if(res != CURLE_OK)
        {
            vely_end_web(cc.error, res, &tries, &cc);
            return VV_ERR_WEB_CALL;
        }
        else
        {
            long response_code;
            res = curl_easy_getinfo(cc.curl, CURLINFO_RESPONSE_CODE, &response_code);
            if (resp_code != NULL) *resp_code = (num)response_code;
            if((res == CURLE_OK) &&
                   ((response_code / 100) != 3))
            {
                /* a redirect implies a 3xx response code , 301/302/308/307 temp and perm*/
                // this is if NOT a redirection (3xx). The actual result is obtained ONLY from the actual data response and NOT from redirection.
                *result = sresp.ptr;
                len = sresp.len;
                if (head != NULL)
                {
                    *head = shead.ptr;
                    lenhead = shead.len;
                }
            }
            else
            {
                char *location;
                res = curl_easy_getinfo(cc.curl, CURLINFO_REDIRECT_URL, &location);

                if((res == CURLE_OK) && location)
                {
                      /* This is the new absolute URL that you could redirect to, even if
                       *            * the Location: response header may have been a relative URL. */
                      VV_TRACE("Redirecting to [%s]", location);
                      vely_free (sresp.ptr); // free the old result
                      vely_free (shead.ptr); // free the old result
                      //
                      // Recursive call to this function is done so that its result is always immediately
                      // passed back to the caller, so that it is a clean winding and unwinding. There is no unwinding followed
                      // by winding followed by unwinding etc. There is only winding and then unwinding back to the original caller.
                      // So 'tries' is increased up to the last recursive call, and after that one returns without a recursion it goes
                      // back to the original one without any interruption. THat's why we can set tries to zero right away.
                      // So, when 'res' is obtained it MUST BE immediate passed back.
                      //
                      num res = vely_post_url_with_response(location, result, head, cc.error, cert, cookiejar, resp_code, timeout, bodyreq, fields, files, vh, method, payload, payload_len);
                      tries = 0;
                      vely_cleanup_curlcall(&cc);
                      VV_TRACE("Finished redirection");
                      return res;
                }
                else
                {
                    // no location? result is empty by default
                }
           }
        }

        vely_cleanup_curlcall(&cc);
        VV_TRACE("Closed curl connection");
    }
    else
    {
        if (cc.error != NULL) *(cc.error) = vely_strdup ("Cannot initialize URL library");
        tries = 0;
        return VV_ERR_FAILED;
    }
    tries = 0;
    return len;
}

static CURLcode vely_process_post (curlcall *cc, char *fields[], char *files[], num *tries)
{
    CURLcode res = CURLE_OK;

    curl_mimepart *fld = NULL;

    // make a form
    if ((cc->form = curl_mime_init(cc->curl)) == NULL)
    {
        vely_end_web(NULL, res, tries, cc);
        if (cc->error != NULL) *(cc->error) = vely_strdup ("Cannot add to header list");
        return VV_ERR_FAILED;
    }

    // add files, input parsing guarantees [i,i+1] are not NULL
    num i = 0;
    if (files != NULL)
    {
        while (files[i] != NULL) 
        {
            if ((fld = curl_mime_addpart(cc->form)) == NULL)
            {
                vely_end_web(NULL, res, tries, cc);
                if (cc->error != NULL) *(cc->error) = vely_strdup ("Cannot add file to form");
                return VV_ERR_FAILED;
            }
            if ((res = curl_mime_name(fld, files[i])) != CURLE_OK)
            {
                vely_end_web(cc->error, res, tries, cc);
                return VV_ERR_FAILED;
            }
            if ((res = curl_mime_filedata(fld, files[i+1])) != CURLE_OK)
            {
                vely_end_web(cc->error, res, tries, cc);
                return VV_ERR_FAILED;
            }
            i+=2;
        }
    }

    // add fields, input parsing guarantees [i,i+1] are not NULL
    i = 0;
    if (fields != NULL)
    {
        while (fields[i] != NULL) 
        {
            if ((fld = curl_mime_addpart(cc->form)) == NULL)
            {
                vely_end_web(NULL, res, tries, cc);
                if (cc->error != NULL) *(cc->error) = vely_strdup ("Cannot add field to form");
                return VV_ERR_FAILED;
            }
            if ((res = curl_mime_name(fld, fields[i])) != CURLE_OK)
            {
                vely_end_web(cc->error, res, tries, cc);
                return VV_ERR_FAILED;
            }
            if ((res = curl_mime_data(fld, fields[i+1], CURL_ZERO_TERMINATED)) != CURLE_OK)
            {
                vely_end_web(cc->error, res, tries, cc);
                return VV_ERR_FAILED;
            }
            i+=2;
        }
    }

    // headers, say Expect: 100-continue is not wanted 
    static char buf[] = "Expect:";
    struct curl_slist *tmp;
    if ((tmp = curl_slist_append(cc->hlist, buf)) == NULL)
    {
        vely_end_web(NULL, res, tries, cc);
        if (cc->error != NULL) *(cc->error) = vely_strdup ("Cannot add 'expect' to header list");
        return VV_ERR_FAILED;
    }
    cc->hlist = tmp;
    curl_easy_setopt(cc->curl, CURLOPT_MIMEPOST, cc->form);

    return CURLE_OK;
}

