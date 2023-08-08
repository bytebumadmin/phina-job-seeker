JS=job_seeker
SRC_DIR=src
BUILD_DIR=build
TEST_DIR=test
SRC_FILES=$(SRC_DIR)/$(JS).c $(SRC_DIR)/$(JS)_mongo_db.c
LIBS=`pkg-config --libs --cflags uuid libmongoc-1.0`

job-seeker-register-build:
	cc -o $(BUILD_DIR)/js_reg $(SRC_FILES) \
	$(TEST_DIR)/$(JS)_register.c $(LIBS)

job-seeker-register-clean:
	rm -rf $(BUILD_DIR)/js_reg $(BUILD_DIR)/dummy-data.db

job-seeker-register-run:
	cd $(BUILD_DIR) && \
	./js_reg "bumbuna@pm.me" "jacob" "Male" "bumbuna" "12345" "0743591000" "xxxxxxxx"

job-seeker-register-test: job-seeker-register-clean job-seeker-register-build job-seeker-register-run 

job-seeker-login-build:
	cc -o $(BUILD_DIR)/js_log $(SRC_FILES) \
	$(TEST_DIR)/$(JS)_login.c -g $(LIBS)

job-seeker-login-clean:
	rm -rf $(BUILD_DIR)/js_log

job-seeker-login-run:
	cd $(BUILD_DIR) && \
	./js_log "bumbuna@pm.me" "12345"

job-seeker-login-test: job-seeker-register-test job-seeker-login-clean job-seeker-login-build job-seeker-login-run 

# session

job-seeker-session-build:
	cc -o $(BUILD_DIR)/js_sess $(SRC_FILES) \
	$(TEST_DIR)/$(JS)_session.c -g ${LIBS}

job-seeker-session-clean:
	rm -rf $(BUILD_DIR)/js_sess $(BUILD_DIR)/dummy-session.db

job-seeker-session-run:
	cd $(BUILD_DIR) && \
	./js_sess "bumbuna@pm.me" "12345"



job-seeker-session-test: job-seeker-login-test job-seeker-session-clean job-seeker-session-build job-seeker-session-run 