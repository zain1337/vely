#include "pthread.h"
#include "assert.h"
#include "vfcgi.h"
#define REQ_LEN 200
#define MT_RUNS 100

void *call_server (void *inp)
{
    // Request type vv_fc - create a request variable and zero it out
    vv_fc req = {0};

    req.fcgi_server = "/var/lib/vv/hash_server/sock/sock"; // Unix socket
    req.req_method = "GET"; // GET HTTP method
    req.app_path = "/hash_server"; // application path
    req.req = "/server"; // request name
    char *env[3];
    env[0]="VV_SILENT_HEADER";
    env[1]="yes";
    env[2]=NULL;
    req.env = env;

    req.url_payload = (char*)malloc (REQ_LEN); assert(req.url_payload);
    snprintf (req.url_payload, REQ_LEN, "/op/add/key/%ld/data/data_%ld>>", (off_t)inp, (off_t)inp);

    int res = vv_fc_request (&req);

    // Check return status, and if there's an error, display error code and the
    // corresponding error message. Otherwise, print out server response.
    if (res != VV_OKAY) {
        fprintf (stderr, "Request failed [%d] [%s]\n", res, vv_fc_error(&req));
    }
    else {
        printf ( "%s", vv_fc_data(&req));
    }

    // Free up resources so there are no memory leaks
    vv_fc_delete(&req);
    return (void*)(off_t)res;
}

int main ()
{

    // Make a request
    pthread_t thread_id[MT_RUNS+1];
    int i;
    for (i = 0; i < MT_RUNS; i++){
        pthread_create(&(thread_id[i]), NULL, call_server, (void*)(off_t)i);
    }
    int bad = 0;
    void *thread_res[MT_RUNS+1];
    for (i = 0; i < MT_RUNS; i++){
        pthread_join(thread_id[i], &(thread_res[i]));
        int r = (int)(off_t)(thread_res[i]);
        if (r != VV_OKAY) bad++;
    }
    if (bad!=0) {
        fprintf (stderr, "Total [%d] bad\n", bad);
        return -1;
    } else {
        printf ("All okay return value\n");
        return 0;
    }

}
