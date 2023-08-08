#include "../job_seeker.h"
#include "../job_seeker_db.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    job_seeker_logged_user_info_t *user;
    assert(job_seeker_db_initialize() == 0);
    int r = job_seeker_account_exist(&(job_seeker_login_info_t) {
        .email_address = argv[1],
        .password = argv[2]
    }, &user);
    assert(r == 0);
    printf("User %s (#%s) found/logged in.\n", user->first_name, user->id);
    return 0;
}