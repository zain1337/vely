#include "vfcgi.h"

void main ()
{
  // Request type vv_fc - create a request variable and zero it out
  vv_fc req = {0};

  req.fcgi_server = "/var/lib/vv/hash_server/sock/sock"; // Unix socket
  req.req_method = "GET"; // GET HTTP method
  req.app_path = "/hash_server"; // application path
  req.req = "/server"; // request name
  req.url_payload = "/op/add/key/1001/data/data_1001";

  // Make a request
  int res = vv_fc_request (&req);

  // Check return status, and if there's an error, display error code and the
  // corresponding error message. Otherwise, print out server response.
  if (res != VV_OKAY) printf ("Request failed [%d] [%s]\n", res, vv_fc_error(&req));
  else printf ("%s", vv_fc_data(&req));

  // Free up resources so there are no memory leaks
  vv_fc_delete(&req);
}
