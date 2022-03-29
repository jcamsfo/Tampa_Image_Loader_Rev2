#include <iostream>
#include <thread>
#include <string>

#include "KeyValueClient.h"
#include "TestCommon.h"

enum State {
    BEFORE_LOGIN,
    AFTER_LOGIN
};

struct LoginState {
    string username;
    string password;
};

void enable_disable_login(KeyValueClient & key_value_client, const LoginState& login_state) {
    if (login_state.username.empty() || login_state.password.empty()) {
        key_value_client.disable_widget("login");
    }
    else {
        key_value_client.enable_widget("login");
    }
}

int main(int argc, char * argv[]) {

    usage("sequence_test");
    cout << endl;
    cout << "  Sequence_test shows how it can request key_value_gui to load a sequence of ui files, based on events from the GUI." << endl;
    cout << "  To illustrate, sequence_test uses a fake login with both success and failure branches." << endl;
    cout << "  To succeed, use username and password 'admin', to fail use any other username and/or password." << endl;
    
    Params params = collect_params(argc, argv);

    LoginState login_state;

    KeyValueClient key_value_client;
    key_value_client.connect(params.role, "localhost", params.port);

    while (key_value_client.connect_result() == ConnectError::PENDING) {
        sleep_one_millisecond();
    }

    if (key_value_client.connect_result() != ConnectError::SUCCESS) {
        return 0;
    }

    // tell key_value_gui to open the login.ui file
    key_value_client.open_ui_file("login.ui");

    State state = State::BEFORE_LOGIN;
    int counter = 0;
    string success_or_fail_filename;
    while (key_value_client.connect_result() == ConnectError::SUCCESS) {
        KeyValue * incoming_key_value = key_value_client.next_received();
        if (incoming_key_value != nullptr) {
            cout << "received:" << incoming_key_value->key << " " << incoming_key_value->value << endl;

            if (state == State::BEFORE_LOGIN) {
                if (incoming_key_value->key == "username") {
                    login_state.username = incoming_key_value->value;
                    enable_disable_login(key_value_client, login_state);
                }
                else if (incoming_key_value->key == "password") {
                    login_state.password = incoming_key_value->value;
                    enable_disable_login(key_value_client, login_state);
                }
                else if (incoming_key_value->key == "login" && incoming_key_value->value == "clicked") {
                    state = State::AFTER_LOGIN;
                    key_value_client.close_ui_file("login.ui");
                   
                    if (login_state.username == "admin" && login_state.password == "admin") {
                        success_or_fail_filename = "succeeded.ui";
                    }
                    else {
                        success_or_fail_filename = "failed.ui";
                    }
                    key_value_client.open_ui_file(success_or_fail_filename);
                }
            }
            else {
                // state = AFTER_LOGIN
                if (incoming_key_value->key == "ok" && incoming_key_value->value == "clicked") {
                    // close the 'succeeded' or 'failed' dialog
                    key_value_client.close_ui_file(success_or_fail_filename);
                    break;
                }
            }
            delete incoming_key_value;
        }

        sleep_one_millisecond();
    }

    key_value_client.disconnect();

    return 0;
}
