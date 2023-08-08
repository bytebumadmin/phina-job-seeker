#include "../job_seeker.h"
#include "../job_seeker_db.h"
#include <assert.h>
#include <stdio.h>

int main(int argc, char **argv) {
    assert(job_seeker_db_initialize() == 0);
    char *data;
    int r = job_seeker_register(&(job_seeker_register_info_t) {
        .email_address = argv[1],
        .first_name = argv[2],
        .gender = argv[3],
        .last_name = argv[4],
        .password = argv[5],
        .phone_number = argv[6],
        .profile_image_id = argv[7],
        .date_of_birth = {
            .tm_mon = 8,
            .tm_year = 99,
            .tm_mday = 13
        }
    }, (void**) &data);
    if(r == -1) {
        fprintf(stderr, "failed: %s\n", data);
    } else {
        printf("User (#%s) added successfully!!.\n", data);
    }
    return r;
}