#include "../kcgi_headers.h"
#include "../job_seeker.h"
#include "bson/bson.h"

int main(int arg, char **argv) {
    struct kfcgi *client;
    struct kreq request;
    // struct kvalid valid[] = {
    //     {
            
    //     }
    // }
    if(khttp_fcgi_init(&client, 0, 0, 0,0 ,0) != KCGI_OK) {
        fprintf(stderr, "Failed to initlaize FCGI server.\n");
        return 1;
    }
    while(khttp_fcgi_parse(client, &request) == KCGI_OK) {
        //process request
        kcgi
    }
}