#ifndef JOB_SEEKER_LOG_H
#define JOB_SEEKER_LOG_H

int job_seeker_log_init(void*);

int job_seeker_log_write(void*,...);

int job_seeker_log_close(void*);
#endif