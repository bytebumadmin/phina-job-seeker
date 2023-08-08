#ifndef JOB_SEEKER_TYPES_H
#define JOB_SEEKER_TYPES_H

#include <time.h>

typedef char* job_seeker_id_t;

typedef char* job_seeker_session_id_t;

// // single error code variable for the library
extern int job_seeker_error;

enum JOB_SEEKER_ERROR {
    JOB_SEEKER_EMAIL_TAKEN,
    JOB_SEEKER_ACCOUNT_NOT_FOUND,
    JOB_SEEKER_SESSION_INVALID,
    JOB_SEEKER_ACCOUNT_UNVERIFIED,
    JOB_SEEKER_SUSPENDED,
    JOB_SEEKER_DB_ERROR
};

typedef struct job_seeker_register_info {
    char *first_name;
    char *last_name;
    char *email_address;
    char *phone_number;
    char *gender;
    char *password;
    char *profile_image_id;
    struct tm date_of_birth;
} job_seeker_register_info_t;

typedef struct job_seeker_login_info {
    char *email_address;
    char *password;
} job_seeker_login_info_t;

typedef struct job_seeker_logged_user_info {
    char *email_address;
    char *id;
    char *profile_img;
    int email_verified;
    char *first_name;
} job_seeker_logged_user_info_t;

typedef struct job_seeker_session {
    char *session_id;
    char *device_name;
    job_seeker_id_t job_seeker_id;
    size_t creation_time;
    size_t expiry_time;
} job_seeker_session_t;


#endif