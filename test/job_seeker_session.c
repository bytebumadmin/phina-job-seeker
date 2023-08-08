#include "../job_seeker.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    job_seeker_logged_user_info_t *user;
    job_seeker_session_t *sess;
    int r;
    assert(job_seeker_initialize() == 0);
    r = job_seeker_account_exist(&(job_seeker_login_info_t) {
        .email_address = argv[1],
        .password = argv[2]
    }, &user);
    assert(r == 0);
    job_seeker_session_t session = {
        .device_name = "My hp",
        .job_seeker_id = user->id,
        .creation_time = time(0)*1000,
        .expiry_time = time(0)*1000+100000
    };
    r = job_seeker_session_create(&session);
    assert(r == 0);
    printf("Session #%s created for user #%s\n", session.session_id, session.job_seeker_id);
    r = job_seeker_session_exist(session.session_id, &sess);
    assert(r == 0);
    printf("Session #%s is valid for user #%s\n", sess->session_id, sess->job_seeker_id);
    r = job_seeker_session_drop(session.session_id);
    assert(r == 0);
    printf("Session #%s has been deleted successfully.\n", session.session_id);
    return 0;
}