#pragma once

#include "job_seeker_types.h"
#include "job_seeker_log.h"

int job_seeker_initialize();

int job_seeker_register(job_seeker_register_info_t *info, void **data);

int job_seeker_account_exist(job_seeker_login_info_t *info, job_seeker_logged_user_info_t **data);

int job_seeker_session_create(job_seeker_session_t *session);

int job_seeker_session_exist(char *session_id, job_seeker_session_t **session);

int job_seeker_session_drop(const char *session_id);;




