#include "../job_seeker_db.h"
#include <mongoc/mongoc.h>
#include <bson/bson.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>

static mongoc_client_t *job_seeker_db_client;
static mongoc_database_t *job_seeker_db_db;
static mongoc_collection_t *job_seeker_db_collection_accounts;
static mongoc_collection_t *job_seeker_db_collection_sessions;
static mongoc_collection_t *job_seeker_db_collection_preferences;
static bson_error_t job_seeker_db_error;
static bson_t *job_seeker_db_query;
static bson_t *job_seeker_db_opts;
static const bson_t *job_seeker_db_doc;
static bson_t job_seeker_db_reply;
static bson_iter_t job_seeker_db_doc_iter;
static bson_error_t job_seeker_db_error;
static bson_oid_t job_seeker_db_oid;
static mongoc_cursor_t *job_seeker_db_cursor;
static char oid_string[25];

int job_seeker_db_account_create(job_seeker_register_info_t *info, void **data) {
    int return_code = 0;
    bson_oid_init(&job_seeker_db_oid, 0);
    job_seeker_db_query = BCON_NEW(
        "_id", BCON_OID(&job_seeker_db_oid),
        "name", "{",
            "first", BCON_UTF8(info->first_name),
            "last", BCON_UTF8(info->last_name),
        "}",
        "emailaddress", BCON_UTF8(info->email_address),
        "password", BCON_UTF8(info->password),
        "profileimg", BCON_UTF8(info->profile_image_id),
        "phonenumber", BCON_UTF8(info->phone_number),
        "gender", BCON_UTF8(info->gender),
        "dateofbirth", BCON_DATE_TIME(mktime(&info->date_of_birth) * 1000),
        "emailverified", BCON_BOOL(false),
        "phoneverified", BCON_BOOL(false),
        "createtime", BCON_DATE_TIME(time(0) * 1000)
    );
    if(
        mongoc_collection_insert_one(job_seeker_db_collection_accounts, 
        job_seeker_db_query, NULL, &job_seeker_db_reply, &job_seeker_db_error) == 0
    ) {
        //log error
        *data = job_seeker_db_error.message;
        job_seeker_error = JOB_SEEKER_DB_ERROR;
        return_code = -1;
        goto cleanup;
    }
    // success
    bson_oid_to_string(&job_seeker_db_oid, oid_string);
    *data = oid_string;
cleanup:
    bson_destroy(job_seeker_db_query);
    return return_code;
}

int job_seeker_db_account_exist(job_seeker_login_info_t *info, job_seeker_logged_user_info_t *data) {
    int return_code = 0;
    job_seeker_db_query = BCON_NEW(
        "emailaddress", BCON_UTF8(info->email_address),
        "password", BCON_UTF8(info->password)
    );
    job_seeker_db_opts = BCON_NEW(
        "projection", "{",
            "name.first", BCON_BOOL(true),
            "profileimg", BCON_BOOL(true),
            "emailverified", BCON_BOOL(true),
        "}",
        "limit", BCON_INT64(1)
    );
    job_seeker_db_cursor = mongoc_collection_find_with_opts(job_seeker_db_collection_accounts,
    job_seeker_db_query, job_seeker_db_opts, NULL);

    if(job_seeker_db_cursor == NULL) {
        //log error
        job_seeker_error = JOB_SEEKER_DB_ERROR;
        return_code = -1;
        goto cleanup;
    }
    if(mongoc_cursor_next(job_seeker_db_cursor, &job_seeker_db_doc) == 0) {
        job_seeker_error = JOB_SEEKER_ACCOUNT_NOT_FOUND;
        return_code = -1;
        goto cleanup;
    }
    // bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "emailaddress");
    bson_iter_t temp;
    const char *n;
    uint32_t length;
    bson_iter_init(&job_seeker_db_doc_iter, job_seeker_db_doc);
    if(bson_iter_find_descendant(&job_seeker_db_doc_iter, "name.first", &temp)) {
        if(BSON_ITER_HOLDS_UTF8(&temp)) {
            n = bson_iter_utf8(&temp, &length);
            data->first_name = strndup(n, length);
        }
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "profileimg");
    if(BSON_ITER_HOLDS_UTF8(&job_seeker_db_doc_iter)) {
        n = bson_iter_utf8(&job_seeker_db_doc_iter, &length);
        data->profile_img = strndup(n, length);
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "emailverified");
    if(BSON_ITER_HOLDS_BOOL(&job_seeker_db_doc_iter)) {
        // n = bson_iter_utf8_len_unsafe(&job_seeker_db_doc_iter);
        data->email_verified= bson_iter_bool(&job_seeker_db_doc_iter);
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "_id");
    if(BSON_ITER_HOLDS_OID(&job_seeker_db_doc_iter)) {
        data->id = oid_string;
        bson_oid_to_string(bson_iter_oid(&job_seeker_db_doc_iter), oid_string); 
    }
    
cleanup:
    if(job_seeker_db_cursor) {
        mongoc_cursor_destroy(job_seeker_db_cursor);
    }    
    bson_destroy(job_seeker_db_query);
    bson_destroy(job_seeker_db_opts);
    return return_code;
}

int job_seeker_db_initialize() {
    char line[200];
    if(job_seeker_db_client == NULL) {
        char *client_url = getenv("JOB_SEEKER_MONGO_DB_URL");
        if(client_url == NULL) {
            FILE *f = fopen("mongo-db.url", "r");
            if(f == NULL) {
                job_seeker_error = JOB_SEEKER_DB_ERROR;
                return -1;
            }
            size_t line_len = 200;
            fgets(line, line_len, f);
            client_url = line;
        }
        mongoc_uri_t *client_uri = mongoc_uri_new_with_error(client_url, &job_seeker_db_error);
        if(client_uri == NULL) {
            //TODO log error
            job_seeker_error = JOB_SEEKER_DB_ERROR;
            return -1;
        }
        job_seeker_db_client = mongoc_client_new_from_uri_with_error(client_uri, &job_seeker_db_error);
        if(job_seeker_db_client == NULL) {
            //log error
            job_seeker_error = JOB_SEEKER_DB_ERROR;
            return -1;
        }
        job_seeker_db_db = mongoc_client_get_database(job_seeker_db_client, "job_seeker");
        if(job_seeker_db_db == NULL) {
            //log error
            job_seeker_error = JOB_SEEKER_DB_ERROR;
            return -1;
        }
        job_seeker_db_collection_accounts = mongoc_database_get_collection(job_seeker_db_db, "accounts");
        if(job_seeker_db_collection_accounts == NULL) {
            //log error
            job_seeker_error = JOB_SEEKER_DB_ERROR;
            return -1;
        }
        job_seeker_db_collection_sessions = mongoc_database_get_collection(job_seeker_db_db, "sessions");
        if(job_seeker_db_collection_sessions == NULL) {
            //log error
            job_seeker_error = JOB_SEEKER_DB_ERROR;
            return -1;
        }
        job_seeker_db_collection_preferences = mongoc_database_get_collection(job_seeker_db_db, "preferences");
        if(job_seeker_db_collection_preferences == NULL) {
            //log error
            job_seeker_error = JOB_SEEKER_DB_ERROR;
            return -1;
        }
    }

    return 0;
}

int job_seeker_db_session_create(job_seeker_session_t *session) {
    bool r;
    int return_code = 0;
    job_seeker_db_query = BCON_NEW(
        "_id", BCON_UTF8(session->session_id),
        "device", BCON_UTF8(session->device_name),
        "createtime", BCON_DATE_TIME(session->creation_time),
        "expirytime", BCON_DATE_TIME(session->expiry_time),
        "accountid", BCON_UTF8(session->job_seeker_id),
        "isvalid", BCON_BOOL(true)
    );
    r = mongoc_collection_insert_one(job_seeker_db_collection_sessions, job_seeker_db_query, 0,
    &job_seeker_db_reply, &job_seeker_db_error);
    if(r == false) {
        //log error
        return_code = -1;
        goto cleanup;
    }
cleanup:
    bson_destroy(job_seeker_db_query);
    return return_code; 
}

int job_seeker_db_session_exist(const char *session_id, job_seeker_session_t *session) {
    int return_code = 0;
    uint32_t length;
    const char *n;
    job_seeker_db_query =BCON_NEW(
        "_id", BCON_UTF8(session_id),
        "expirytime", "{", "$gt", BCON_DATE_TIME(time(0)*1000), "}" 
    );
    job_seeker_db_opts = BCON_NEW(
        "limit", BCON_INT32(1)
    );
    job_seeker_db_cursor = mongoc_collection_find_with_opts(job_seeker_db_collection_sessions,
    job_seeker_db_query, job_seeker_db_opts, 0);
    if(job_seeker_db_cursor == NULL) {
        return_code = -1;
        goto cleanup;
    }
    if(mongoc_cursor_next(job_seeker_db_cursor, &job_seeker_db_doc) == 0) {
        job_seeker_error = JOB_SEEKER_SESSION_INVALID;
        return_code = -1;
        goto cleanup;
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "_id");
    if(BSON_ITER_HOLDS_UTF8(&job_seeker_db_doc_iter)) {
        n = bson_iter_utf8(&job_seeker_db_doc_iter, &length);
        session->session_id = strndup(n, length);
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "createtime");
    if(BSON_ITER_HOLDS_DATE_TIME(&job_seeker_db_doc_iter)) {
        session->creation_time = bson_iter_time_t(&job_seeker_db_doc_iter);
        // session[0]->creation_time = strndup(n, length);
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "accountid");
    if(BSON_ITER_HOLDS_UTF8(&job_seeker_db_doc_iter)) {
        n = bson_iter_utf8(&job_seeker_db_doc_iter, &length);
        session->job_seeker_id = strndup(n, length);
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "expirytime");
    if(BSON_ITER_HOLDS_DATE_TIME(&job_seeker_db_doc_iter)) {
        session->expiry_time = bson_iter_time_t(&job_seeker_db_doc_iter);
    }
cleanup:
    if(job_seeker_db_cursor != NULL) {
        mongoc_cursor_destroy(job_seeker_db_cursor);
    }
    bson_destroy(job_seeker_db_query);
    bson_destroy(job_seeker_db_opts);
    return return_code;
}

int job_seeker_db_session_drop(const char *session_id) {
    int return_code = 0;
    job_seeker_db_query = BCON_NEW(
        "_id", BCON_UTF8(session_id)
    );
    if(mongoc_collection_delete_one(job_seeker_db_collection_sessions, job_seeker_db_query, NULL,
    &job_seeker_db_reply, &job_seeker_db_error) == false) {
        //log error
        job_seeker_error = JOB_SEEKER_DB_ERROR;
        return_code = -1;
    }
    bson_destroy(&job_seeker_db_reply);
cleanup:
    bson_free(job_seeker_db_query);
    return return_code;
}

int job_seeker_db_search_by_email(const char *email_address, char **user_id) {
    int return_code = 0;
    *user_id = NULL;
    job_seeker_db_query = BCON_NEW(
        "emailaddress", BCON_UTF8(email_address)
    );
     job_seeker_db_opts = BCON_NEW(
        "projection", 
        "{", "_id", BCON_BOOL(true), "}",
        "limit", BCON_INT64(1)
    );
    job_seeker_db_cursor = mongoc_collection_find_with_opts(job_seeker_db_collection_accounts,
    job_seeker_db_query, job_seeker_db_opts, NULL);

    if(job_seeker_db_cursor == NULL) {
        //log error
        job_seeker_error = JOB_SEEKER_DB_ERROR;
        return_code = -1;
        goto cleanup;
    }
    if(mongoc_cursor_next(job_seeker_db_cursor, &job_seeker_db_doc) == false) {
        goto cleanup;
    }
    bson_iter_init_find(&job_seeker_db_doc_iter, job_seeker_db_doc, "_id");
    const bson_oid_t *oid = bson_iter_oid(&job_seeker_db_doc_iter);
    bson_oid_to_string(oid, oid_string);
    *user_id = oid_string;
cleanup:
    if(job_seeker_db_cursor) {
        mongoc_cursor_destroy(job_seeker_db_cursor);
    }    
    bson_destroy(job_seeker_db_query);
    bson_destroy(job_seeker_db_opts);
    return return_code;
}