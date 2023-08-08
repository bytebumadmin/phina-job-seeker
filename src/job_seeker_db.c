#include "../job_seeker_db.h"
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define JSDB_MAX_LINE_LEN 512

static int job_seeker_db_user_tbl_handler = 0;
static int job_seeker_db_session_tbl_handler = 0;

int job_seeker_db_initialize() {
    if(job_seeker_db_user_tbl_handler == 0) {
        job_seeker_db_user_tbl_handler = open("dummy-data.db", O_CREAT|O_RDWR, S_IRWXO|S_IRWXU);
        if(job_seeker_db_user_tbl_handler == -1) {
            perror("open-user");
            exit(1);
        } 
    }
    if(job_seeker_db_session_tbl_handler == 0) {
        job_seeker_db_session_tbl_handler = open("dummy-session.db", O_CREAT|O_RDWR, S_IRWXO|S_IRWXU);
        if(job_seeker_db_session_tbl_handler == -1) {
            perror("open-session");
            exit(1);
        } 
    }
    return 0;
}

static char *job_seeker_register_info_to_string(job_seeker_register_info_t *info) {
    static char str[JSDB_MAX_LINE_LEN];
    snprintf(str, JSDB_MAX_LINE_LEN, "%s,%s,%s,%s,%s,%s,%s\n",
    info->email_address,
    info->first_name,
    info->gender,
    info->last_name,
    info->password,
    info->phone_number,
    info->profile_image_id
    );
    return str;
}

int job_seeker_db_insert(job_seeker_register_info_t *info) {
    char *info_str = job_seeker_register_info_to_string(info);
    int write_count = write(job_seeker_db_user_tbl_handler, info_str, strlen(info_str));
    if(write_count == -1) {
        perror("write");
        exit(1);
    }
    fsync(job_seeker_db_user_tbl_handler);
    return 0;
}

static char *job_seeker_read_line() {
    static char read_string[JSDB_MAX_LINE_LEN];
    int read_index = 0;
    for(char c; read_index < JSDB_MAX_LINE_LEN; read_index++) {
        int read_count = read(job_seeker_db_user_tbl_handler, &read_string[read_index], 1);
        if(read_count == -1) {
            perror("read");
            exit(1);
        }
        if(read_string[read_index] == '\n' || read_count == 0) {
            read_string[read_index] = 0;
            break;
        }
    }
    if(read_index == JSDB_MAX_LINE_LEN) {
        fprintf(stderr, "Line too long.\n");
        exit(1);
    }
    return read_string;
}

static job_seeker_login_info_t *job_seeker_get_credential() {
    static job_seeker_login_info_t info;
    char *line = job_seeker_read_line();
    int line_is_empty = line[0] == 0;
    if(line_is_empty)  {
        return NULL;
    }
    char *c = line;
    char *word_start;
    int comma_count = 0;
    while(1) {
        c = strchr(word_start = c,',');
        if(comma_count == 0) {
            info.email_address = word_start;
        } else if(comma_count == 4) {
            info.password = word_start;
            *c = 0;
            break;
        } else if(c == NULL) {
            break;
        }
        *c = 0;
        c++;
        comma_count++;
    }
    return &info;
}

static int job_seeker_compare_login_info(job_seeker_login_info_t *a, job_seeker_login_info_t *b) {
    return strcmp(a->email_address, b->email_address) 
            | strcmp(a->password, b->password);
}

int job_seeker_db_login(job_seeker_login_info_t *info) {
   while(1) {
        job_seeker_login_info_t *creds = job_seeker_get_credential();
        if(creds == NULL) {
            return JOB_SEEKER_NOT_FOUND;
        }
        int credential_match = job_seeker_compare_login_info(creds, info) == 0;
        if(credential_match) {
            return 0;
        }
   }
   //not reachable
   return 1;
}

static char *job_seeker_session_to_string(job_seeker_session_t *session) {
    static char session_str[JSDB_MAX_LINE_LEN];
    snprintf(session_str, JSDB_MAX_LINE_LEN, "%s,%s,%s,%lu,%lu",
    session->session_id, session->job_seeker_id,session->device_name,
    session->creation_time, session->expiry_time);
    return session_str;
}

int job_seeker_db_session(job_seeker_session_t *session) {
    char *session_str = job_seeker_session_to_string(session);
    int write_count = write(job_seeker_db_session_tbl_handler, session_str, strlen(session_str));
    if(write_count == -1) {
        perror("write");
        exit(1);
    }
    fsync(job_seeker_db_user_tbl_handler);
    return 0;
}

