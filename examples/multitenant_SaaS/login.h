#ifndef _VV_LOGIN
#define _VV_LOGIN

typedef struct s_reqdata {
    bool displayed_logout; // true if Logout link displayed
    bool is_logged_in; // true if session verified logged-in
    char *sess_userId; // user ID of current session
    char *sess_id; // session ID
} reqdata;

void login_or_signup ();

#endif
