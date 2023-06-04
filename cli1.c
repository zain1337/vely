#include "vfcgi.h"
#include <stdbool.h>
#include <unistd.h>
#include <assert.h>
#include <pthread.h>

pthread_mutex_t outm;

void *testreq (void *inp);
void set_fast (vv_fc *call, int thread_id, char *server, char *req_method, char *app_path, char *req, char *payload, char *ctype, char *body, int clen, int timeout, char **env);

void *testreq (void *inp)
{
  vv_fc *req = (vv_fc*) inp;

  int retval = 0;
  int res = vv_fc_request (req);
     
  if (res != VV_OKAY) retval = res;
  else {
      // If successful, display request results
      // Exit code from the server processing
      pthread_mutex_lock(&outm); 
      printf("Server status %d thread %d\n", req->req_status, req->thread_id);
      // Length of reply from server
      printf("Len of data %d\n", req->data_len);
      // Length of any error from server
      printf("Len of error %d\n", req->error_len);
      // Reply from server
      printf("Data [%s]\n", req->data);
      // Any error message from server
      printf("Error [%s]\n", req->error);
      pthread_mutex_unlock(&outm); 

      // Free up resources so there are no memory leaks
      //TEST free request
      free (req->data);
      free (req->error);
  }
  return (void*)(off_t)retval;
}


// server, req_method, app_path, req must be specified.
void set_fast (vv_fc *call, int thread_id, char *server, char *req_method, char *app_path, char *req, char *payload, char *ctype, char *body, int clen, int timeout, char **env)
{
  if (env != NULL) call->env = env; 
  call->fcgi_server = server; //TEST using IP:port (TCP)
  call->req_method = req_method;
  call->app_path = app_path;
  call->req = req;
  call->thread_id = thread_id;
  if (payload != NULL) call->url_payload = payload;
  if (body != NULL) call->content_type = ctype;
  if (body != NULL) call->req_body = body;
  if (body != NULL) call->content_len = clen;
  if (timeout >0) call->timeout = timeout;
}

#define MT_RUNS 100
int main (int argc, char **argv) {
    int milli = 300;
    pthread_t thread_id[MT_RUNS+1];
    int i;
    // Request type vv_fc - create a request variable
    vv_fc *req = (vv_fc*)calloc (MT_RUNS, sizeof(vv_fc));
    for (i = 0; i < MT_RUNS; i++){
        char *pay = malloc(100);
        snprintf (pay, 100, "/op/add/key/k_%d/data/d_%d", i, i);
        set_fast (&(req[i]), i, "/var/lib/vv/hash/sock/sock", "GET", "/hash", "/server", pay, NULL, NULL, 0, 0, NULL);
        pthread_create(&(thread_id[i]), NULL, testreq, &(req[i]));
    }
    void *thread_res[MT_RUNS+1];
    bool working = true;
    do  {
        struct timespec slp;
        slp.tv_sec = milli / 1000;
        slp.tv_nsec = (milli % 1000) * 1000000;
        nanosleep(&slp, NULL);
        for (i = 0; i < MT_RUNS; i++){
            if (req[i].done) printf ("!!!!!! Request [%d] DONE, returned [%d]\n", i, req[i].return_code);
            else working=false;
        }

    } while (!working);
    exit(0);
}

