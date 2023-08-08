#include "../kcgi_headers.h"
#include "../job_seeker.h"
#include "bson/bson.h"

static void populate_register_info(job_seeker_register_info_t *user, bson_t *payload) {
    bson_iter_t payload_iter;
    uint32_t len;
    bson_iter_init_find(&payload_iter, payload, "emailaddress");
    user->email_address = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "firstname");
    user->first_name = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "lastname");
    user->last_name = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "password");
    user->password = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "gender");
    user->gender = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "phonenumber");
    user->phone_number = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "profileimg");
    user->profile_image_id = bson_iter_utf8(&payload_iter, &len);
    bson_iter_init_find(&payload_iter, payload, "dateofbirth");
    // dob = bson_iter_date_time(&payload_iter);
    // user->date_of_birth = *gmtime(&dob);
}

static void set_response_headers(struct kreq *request_ptr) {
    khttp_head(request_ptr, kresps[KRESP_CONTENT_TYPE], "%s",
    kmimetypes[KMIME_APP_JSON]);
    khttp_head(request_ptr, kresps[KRESP_STATUS], "%s",
    khttps[KHTTP_200]);
}

static void set_response_status(bson_t *response) {
    bson_t child;
    bson_append_document_begin(response, "status", -1, &child);
    bson_append_int32(&child, "code", -1, job_seeker_error);
    bson_append_utf8(&child, "str", -1, job_seeker_error_to_str[job_seeker_error], -1);
    bson_append_document_end(response, &child);
}

int main(int arg, char **argv) {
    struct kfcgi *client;
    struct kreq request;
    struct kreq *request_ptr = &request;
    bson_t *payload;
    bson_t *response;
    char *response_json;
    bson_error_t error;
    size_t len;
    char *user_id;
    static time_t dob;
    job_seeker_register_info_t user;
    if(khttp_fcgi_init(&client, 0, 0, 0,0 ,0) != KCGI_OK) {
        fprintf(stderr, "Failed to initlaize FCGI server.\n");
        return 1;
    }
    if(job_seeker_initialize() != 0) {
        fprintf(stderr, "Failed to start db.\n");
        return -1;
    }
    while(khttp_fcgi_parse(client, &request) == KCGI_OK) {
        //process request
        set_response_headers(request_ptr);
        khttp_body(request_ptr);
        payload = bson_new_from_json(request.fields[0].val,
        -1,&error);
        response = bson_new();
        if(payload == NULL) {
            set_response_status(response);
            goto cleanup;
        }
        populate_register_info(&user, payload);
        if(job_seeker_email_is_taken(user.email_address) != JOB_SEEKER_OK) {
            set_response_status(response);
            goto cleanup;
        } 
        int l = job_seeker_register(&user, (void*) &user_id);
        response = bson_new();
        set_response_status(response);
        bson_append_utf8(response, "message", -1, user_id, strlen(user_id));    
cleanup:
        khttp_printf(request_ptr, "%s", response_json = bson_as_json(response, (size_t *)&len));
        khttp_free(request_ptr);
        if(payload != NULL) {
            bson_destroy(payload);
        }
        bson_free(response_json);
        bson_free(response);
    } //while
    khttp_fcgi_free(client);
    return 0;
}