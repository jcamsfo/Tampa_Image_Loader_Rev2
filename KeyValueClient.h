//
// Created by jonathan on 9/10/20.
//

#ifndef CLIENT_TEST_KEYVALUECLIENT_H
#define CLIENT_TEST_KEYVALUECLIENT_H


#ifdef _WINDOWS
#include <WinSock2.h>
#include <WS2tcpip.h>
#else
#include <netdb.h>
#endif

#include <thread>
#include <string>
#include <deque>
#include <mutex>

using namespace std;

enum ConnectError {
    PENDING,
    SUCCESS,
    ADDR_INFO_ERROR,
    FAILED_TO_CONNECT,
    SERVER_DISCONNECTED,
    RECEIVE_POLL_ERROR,
    SEND_POLL_ERROR,
    SEND_COUNT_FAILURE,
    FAILED_TO_BIND,
    LISTEN_FAILURE,
    BLOCKING_FAILURE
};

struct KeyValue {
    static const int NOT_TABLE_ENTRY = -1;

    string key;
    string value;
    string function;
    int row = NOT_TABLE_ENTRY;
    int column = NOT_TABLE_ENTRY;

    KeyValue();
    KeyValue(const KeyValue & key_value);
};


class KeyValueClient {
public:
    KeyValueClient();
    ~KeyValueClient();

    static const int unused_socket = -1;

    enum Role {
        CLIENT,
        SERVER,
#ifdef USING_SSL
        SSL_SERVER
#endif
    };

    // returns false if instance is already connected or connecting
    bool connect(Role role, const string & ip_address,const string & port, const string & cert_pem, const string & key_pem);
    ConnectError connect_result(); 
    //  caller must dispose of the pointer
    KeyValue * next_received();
    void disconnect();
    void send(const KeyValue& key_value);
    bool open_ui_file(const string& ui_filename);
    bool cache_ui_file(const string& ui_filename);
    void uncache_ui_file(const string& ui_filename);
    void display_ui_file(const string& ui_filename);
    void close_ui_file(const string& ui_filename);
    void enable_widget(const string& widget_name);
    void disable_widget(const string& widget_name);
    void hide_widget(const string& widget_name);
    void show_widget(const string& widget_name);
    void save_key_value_file(const string& key_value_filename);
    bool load_key_value_file(const string& key_value_filename);
    void set_label(const string & widget_name, const string & label);
    void set_stylesheet(const string & widget_name, const string & stylesheet);

protected:
    // converts the struct to a string; subclass if you want to change the basic protocol
    virtual string serialize(KeyValue * keyValue);
    // reads the received buffer and parses it for the first KeyValue
    // note that the contents of buffer will usually be altered
    virtual KeyValue * deserialize(string & buffer);
    // helper function for dealing with a single chunk of JSON
    virtual KeyValue * parse_json(const string & line);

private:
    void execute_connect(Role pending_role, const string & ip_address, const string & port, const string & cert_pem, const string & key_pem);
    void execute_send(int sock_fd);
    void execute_receive(int sock_fd);
    void set_connect_error(ConnectError connect_error);
    void set_keep_going(bool keep_going_flag);
    bool keep_going();
    void set_keep_sending(bool keep_sending_flag);
    void allow_new_connections();
    bool keep_sending();
    KeyValue * next_send();
    void sendAndReceive(int sock_fd);
    int create_socket(const string & ip_address, const string & port);
    bool is_server();

#ifdef USING_SSL
    void close_ssl();
#endif

private:
    ConnectError connect_error = ConnectError::PENDING;
    thread * send_thread = nullptr;
    thread * receive_thread = nullptr;
    thread * connect_thread = nullptr;
    mutex connect_result_mutex;
    mutex keep_going_mutex;
    mutex send_values_mutex;
    mutex received_values_mutex;
    mutex thread_mutex;
    string received_so_far;
    Role role = Role::CLIENT;
    int remote_fd = unused_socket;
    int local_fd = unused_socket;

#ifdef USING_SSL
    bool ssl_initialized = false;
    // avoid requiring SSL in header
    void * ssl = nullptr;
    void * ssl_ctx = nullptr;
    void * bio = nullptr;
    void * accept_bio = nullptr;
#endif

    bool keep_going_flag = false;
    // used by Role::SERVER only
    bool keep_sending_flag = false;

    // keeps the list of pending key/values to send
    deque<KeyValue *> send_key_values;
    // keeps the list of incoming values
    deque<KeyValue *> received_key_values;
};


#endif //CLIENT_TEST_KEYVALUECLIENT_H
