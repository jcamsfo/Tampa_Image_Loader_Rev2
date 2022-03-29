#ifndef TEST_COMMON_KEYVALUECLIENT_H
#define TEST_COMMON_KEYVALUECLIENT_H

#include <string>
#include "KeyValueClient.h"

struct Params {
    string host = "localhost";
	string port = "4567";
	KeyValueClient::Role role = KeyValueClient::Role::CLIENT;
	string cert_file;
	string key_file;
	string ui_folder;

};

void usage(const std::string & program_name);
Params collect_params(int argc, char* argv[]);
void sleep_one_millisecond();

#endif