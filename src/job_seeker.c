#include "../kcgi_headers.h"
#include "../job_seeker_db.h"
#include "../job_seeker.h"
#include <uuid/uuid.h>
#include <stdlib.h>

int job_seeker_error;

int job_seeker_initialize() {
    return job_seeker_db_initialize();
}

int job_seeker_register(job_seeker_register_info_t *info, void **data) {
    return job_seeker_db_account_create(info, data);
}

int job_seeker_account_exist(job_seeker_login_info_t *info, job_seeker_logged_user_info_t **data) {
    *data = calloc(1, sizeof(**data));
    return job_seeker_db_account_exist(info, *data);
}

static char *job_seeker_generate_uuid() {
    static uuid_t uuid;
    static char uuid_str[37];
    uuid_generate(uuid);
    uuid_unparse_lower(uuid, uuid_str);
    return uuid_str;
}

int job_seeker_session_create(job_seeker_session_t *session) {
    session->session_id = job_seeker_generate_uuid();
    return job_seeker_db_session_create(session);
}

int job_seeker_session_exist(char *session_id, job_seeker_session_t **session) {
    *session = calloc(1, sizeof(**session));
    return job_seeker_db_session_exist(session_id, *session);
}

int job_seeker_session_drop(const char *session_id) {
    return job_seeker_db_session_drop(session_id);
}