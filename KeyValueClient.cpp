//
// Created by jonathan on 9/10/20.
//


#ifndef _WINDOWS
#include <unistd.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/poll.h>
#endif

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <string.h>
#include <iostream>
#include <sstream>
#include <thread>
#include <fcntl.h>
#include <fstream>
#include <streambuf>

#ifdef USING_SSL
#include <openssl/ssl.h>
#include <openssl/err.h>
#include <openssl/bio.h>
#endif

#include <regex>
#include <algorithm>

#include "rapidjson/document.h"
#include "rapidjson/writer.h"
#include "rapidjson/prettywriter.h"
#include "rapidjson/stringbuffer.h"
#include "rapidjson/error/en.h"
#include "rapidjson/filereadstream.h"

#include "KeyValueClient.h"

using namespace std;

#ifdef _WINDOWS
#define ssize_t SSIZE_T 
#endif

void *get_in_addr(struct sockaddr *sa)
{
    if (sa->sa_family == AF_INET) {
        return &(((sockaddr_in*) sa)->sin_addr);
    }

    return &(((sockaddr_in6*) sa)->sin6_addr);
}

bool set_socket_blocking_enabled(int fd, bool blocking) {
    // https://stackoverflow.com/questions/1543466/how-do-i-change-a-tcp-socket-to-be-non-blocking

    if (fd < 0) {
        return false;
    }

#ifdef _WIN32
    unsigned long mode = blocking ? 0 : 1;
    return (ioctlsocket(fd, FIONBIO, &mode) == 0) ? true : false;
#else
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) {
        return false;
    }

    flags = blocking ? (flags & ~O_NONBLOCK) : (flags | O_NONBLOCK);
    return fcntl(fd, F_SETFL, flags) == 0;
#endif
}

void cross_close(int sockfd) {
#ifdef _WINDOWS
    closesocket(sockfd);
#else
    ::close(sockfd);
#endif
}

int cross_poll(pollfd* ufds, unsigned int nfds, int timeout) {
#ifdef _WINDOWS
    return WSAPoll(ufds, nfds, timeout);
#else
    return poll(ufds, nfds, timeout);
#endif
}

#ifdef USING_SSL
// see https://gist.github.com/MartinMReed/6393150
void init_openssl()
{
    SSL_load_error_strings();
    ERR_load_crypto_strings();

    OpenSSL_add_all_algorithms();
    SSL_library_init();
}

// see https://gist.github.com/MartinMReed/6393150
void cleanup_openssl()
{
    EVP_cleanup();
}

// see https://gist.github.com/MartinMReed/6393150
SSL_CTX* create_context()
{
    const SSL_METHOD* method = SSLv23_server_method();;
    SSL_CTX* ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return nullptr;
    }

    return ctx;
}

// see https://gist.github.com/MartinMReed/6393150
bool configure_context(SSL_CTX* ctx, const string & cert_pem, const string & key_pem)
{
    /* Set the key and cert */
    if (SSL_CTX_use_certificate_file(ctx, cert_pem.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, key_pem.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return false;
    }

    // only needed if cert is self-issued?
    SSL_CTX_set_verify(ctx, SSL_VERIFY_NONE, 0);
    return true;
}
#endif  // USING_SSL

KeyValue::KeyValue() = default;

KeyValue::KeyValue(const KeyValue & key_value) {
    *this = key_value;
}

KeyValueClient::KeyValueClient() {
#ifdef _WINDOWS
    WSADATA wsa_data;
    // Initialize Winsock
    int result = WSAStartup(MAKEWORD(2, 2), &wsa_data);
    if (result != 0) {
        cerr << "WSAStartup failed:" << result << endl;
    }
#endif
}

KeyValueClient::~KeyValueClient() {
#ifdef _WINDOWS
    WSACleanup();
#endif

#ifdef USING_SSL
    close_ssl();
     
    if (ssl_ctx != nullptr) {
        SSL_CTX_free((SSL_CTX *) ssl_ctx);
        cleanup_openssl();
    }
#endif
}

void KeyValueClient::set_connect_error(ConnectError a_connect_error) {
    lock_guard<mutex> guard(this->connect_result_mutex);
    this->connect_error = a_connect_error;
}

void KeyValueClient::set_keep_going(bool keep_going_flag) {
    lock_guard<mutex> guard(this->keep_going_mutex);
    this->keep_going_flag = keep_going_flag;
}

bool KeyValueClient::keep_going() {
    lock_guard<mutex> guard(keep_going_mutex);
    return keep_going_flag;
}

void KeyValueClient::set_keep_sending(bool keep_sending_flag) {
    lock_guard<mutex> guard(this->keep_going_mutex);
    this->keep_sending_flag = keep_sending_flag;
}

bool KeyValueClient::keep_sending() {
    lock_guard<mutex> guard(keep_going_mutex);
    return keep_sending_flag;
}

void KeyValueClient::allow_new_connections() {
    this->remote_fd = unused_socket;
}

void KeyValueClient::sendAndReceive(int sock_fd) {
    set_keep_sending(true);
    lock_guard<mutex> guard(thread_mutex);
    send_thread = new thread(&KeyValueClient::execute_send, this, sock_fd);
    receive_thread = new thread(&KeyValueClient::execute_receive, this, sock_fd);
}

#ifdef USING_SSL
void KeyValueClient::close_ssl() {
    if (ssl != nullptr) {
        SSL_shutdown((SSL *) ssl);
        SSL_free((SSL *) ssl);
        ssl = nullptr;
    }

    if (bio != nullptr) {
        BIO_free_all((BIO *) bio);
        bio = nullptr;
    }

    if (accept_bio != nullptr) {
        BIO_free_all((BIO *) accept_bio);
        accept_bio = nullptr;
    }
}
#endif

int KeyValueClient::create_socket(const string & ip_address, const string & port) {

    int sock_fd = unused_socket;
    if (this->role != Role::CLIENT) {
        sock_fd = socket(AF_INET, SOCK_STREAM, 0);
        if (sock_fd < 0) {
            printf("opening socket\n");
            return unused_socket;
        }

        sockaddr_in socket_addr;
        memset(&socket_addr, 0, sizeof(socket_addr));
        socket_addr.sin_family = AF_INET;
        if (this->role == Role::CLIENT) {
            // can only handle xxx.xxx.xxx.xxx strings
            inet_pton(AF_INET, ip_address.c_str(), &socket_addr.sin_addr);
        }
        else {
            socket_addr.sin_addr.s_addr = INADDR_ANY;
        }
        socket_addr.sin_port = htons(stoi(port));

        if (this->role == Role::CLIENT) {
            int connectResult = ::connect(sock_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr));
            if (connectResult == -1) {
                cross_close(sock_fd);
                cerr << "connection attempt failed " << gai_strerror(errno) << endl;
                return unused_socket;
            }
        }
        else {
            if (bind(sock_fd, (struct sockaddr*) &socket_addr, sizeof(socket_addr)) == -1) {
                cross_close(sock_fd);
                cerr << "server bind failed " << gai_strerror(errno) << endl;
                return unused_socket;
            }
        }

        cout << "server listening at localhost:" << port << endl;
    }
    else {
        addrinfo hints;
        memset(&hints, 0, sizeof hints);
        hints.ai_family = AF_INET;
        hints.ai_socktype = SOCK_STREAM;

        int addrinfo_result;
        addrinfo* servinfo;
        if ((addrinfo_result = getaddrinfo(ip_address.c_str(), port.c_str(), &hints, &servinfo)) != 0) {
            cerr << "getaddrinfo: " << gai_strerror(addrinfo_result) << endl;
            set_connect_error(ADDR_INFO_ERROR);
            return unused_socket;
        }

        // loop through all the results and bind (server) or connect (client) to the first we can
        addrinfo* p;
        for (p = servinfo; p != nullptr; p = p->ai_next) {
            cout << "family:" << p->ai_family << " type:" << p->ai_socktype << " protocol:" << p->ai_protocol << " addr:" << p->ai_addr << endl;
            if ((sock_fd = socket(p->ai_family, p->ai_socktype, p->ai_protocol)) == -1) {
                cout << "unable to create socket, try another candidate" << endl;
                continue;
            }


            int connectResult = ::connect(sock_fd, p->ai_addr, p->ai_addrlen);
            if (connectResult == -1) {
                cross_close(sock_fd);
                cerr << "connection attempt failed " << gai_strerror(errno) << endl;
                continue;
            }

            break;
        }

        if (p == nullptr) {
            freeaddrinfo(servinfo); // all done with this structure
            if (role == Role::CLIENT) {
                cerr << "client: failed to connect" << endl;
                set_connect_error(FAILED_TO_CONNECT);
            }
            else {
                cerr << "server: failed to bind" << endl;
                set_connect_error(FAILED_TO_BIND);
            }
            return unused_socket;
        }

        char info_buffer[INET6_ADDRSTRLEN];
        inet_ntop(p->ai_family, get_in_addr((struct sockaddr*)p->ai_addr), info_buffer, sizeof info_buffer);
        cout << "client: connecting to " << info_buffer << endl;

        freeaddrinfo(servinfo); // all done with this structure
    }

    return sock_fd;
}

bool KeyValueClient::is_server() {
    if (role == Role::SERVER) {
        return true;
    }

#ifdef USING_SSL
    if (role == Role::SSL_SERVER) {
        return true;
    }
#endif

    return false;
}

void KeyValueClient::execute_connect(Role role, const string & ip_address, const string & port, const string & cert_pem, const string & key_pem) {
    // see https://beej.us/guide/bgnet/html/#a-simple-stream-client

    this->role = role;

    this->local_fd = create_socket(ip_address, port);
    if (this->local_fd == unused_socket) {
        // failed to create socket
        return;
    }

    if (is_server()) {
        bool blocking_result = set_socket_blocking_enabled(this->local_fd, false);
        if (!blocking_result) {
            set_connect_error(BLOCKING_FAILURE);
            set_keep_going(false);
            return;
        }
    }

    if (keep_going()) {
        if (is_server()) {
            int backlog = 2;  // how many pending connections queue will hold
            if (listen(this->local_fd, backlog) == -1) {
                cout << "listen failure" << endl;
                set_connect_error(LISTEN_FAILURE);
                set_keep_going(false);
                return;
            }

            while (keep_going()) {
                struct sockaddr_storage client_addr; // connector's address information
                socklen_t sin_size = sizeof client_addr;
                int candidate_fd = accept(this->local_fd, (struct sockaddr*)&client_addr, &sin_size);
                if (candidate_fd == -1) {
                    //cerr << "server accept failed" << endl;
                    continue;
                }

                char client_info_buffer[INET6_ADDRSTRLEN];
                inet_ntop(client_addr.ss_family, get_in_addr((struct sockaddr*)&client_addr), client_info_buffer, sizeof client_info_buffer);
                cout << "server: got new connection from " << client_info_buffer << endl;

                if (this->remote_fd >= 0) {
                    cout << "Only one connection allowed at a time; closing new connection." << endl;
                    cross_close(candidate_fd);
                    continue;
                }

#ifdef USING_SSL
                if (this->role == Role::SSL_SERVER) {
                    if (!this->ssl_initialized) {
                        init_openssl();
                        this->ssl_initialized = true;

                        ssl_ctx = create_context();
                        if (ssl_ctx == nullptr) {
                            cout << "unable to create ssl context " << endl;
                            return;
                        }
                        if (!configure_context((SSL_CTX * ) ssl_ctx, cert_pem, key_pem)) {
                            cout << "unable to configure context" << endl;
                            return;
                        }

                        if (!SSL_CTX_check_private_key ((SSL_CTX *) ssl_ctx)) {
                            cout << "private key problem " << endl;
                            return;
                        }
                    }

                    ssl = SSL_new((SSL_CTX *) ssl_ctx);
                    if (!SSL_check_private_key ((SSL *) ssl)) {
                        cout << "private key failed" << endl;
                        close_ssl();
                        cross_close(candidate_fd);
                        continue;
                    }


                    accept_bio = BIO_new_socket(candidate_fd, BIO_CLOSE);
                    SSL_set_bio((SSL *) ssl, (BIO *) accept_bio, (BIO *) accept_bio);
                    int accept_result = SSL_accept((SSL *) ssl);

                    ERR_print_errors_fp(stderr);

                    bio = BIO_pop((BIO *) accept_bio);
                    if (accept_result <= 0) {
                        strerror(errno);
                        ERR_print_errors_fp(stdout);
                        cerr << "socket accept failed: " << accept_result << " " << ERR_get_error() << " " << ERR_error_string(ERR_get_error(), nullptr) << endl;
                        close_ssl();
                        cross_close(candidate_fd);
                        continue;
                    }
                }

#endif // USING_SSL

                this->remote_fd = candidate_fd;
                set_connect_error(SUCCESS);
                sendAndReceive(this->remote_fd);
            }
        }
        else {
            sendAndReceive(this->local_fd);
            set_connect_error(SUCCESS);
        }
    }

    cout << "exited connect thread" << endl;
}


KeyValue * KeyValueClient::parse_json(const string & line) {
    if (line.empty()) {
        return nullptr;
    }

    rapidjson::Document doc;
    rapidjson::ParseResult parse_ok = doc.Parse(line.c_str());
    if (!parse_ok) {
        cerr << "Parse error:" << rapidjson::GetParseError_En(parse_ok.Code()) << " " << parse_ok.Offset() << " in " << line << std::endl;
        return nullptr;
    }

    KeyValue* key_value = new KeyValue();
    int field_count = 0;

    if (doc.HasMember("r") && doc["r"].IsInt()) {
        key_value->row = doc["r"].GetInt();
    }    
    if (doc.HasMember("c") && doc["c"].IsInt()) {
        key_value->column = doc["c"].GetInt();
    }
    if (doc.HasMember("k") && doc["k"].IsString()) {
        key_value->key = doc["k"].GetString();
        field_count += 1;
    }
    if (doc.HasMember("v") && doc["v"].IsString()) {
        key_value->value = doc["v"].GetString();
        field_count += 1;
    }
    if (doc.HasMember("f") && doc["f"].IsString()) {
        key_value->function = doc["f"].GetString();
        field_count += 1;
    }

    if (field_count < 2) {
        // need at least two string fields filled in to be valid
        delete key_value;
        return nullptr;
    }

    return key_value;
}

KeyValue * KeyValueClient::deserialize(string & received) {
    // changes the contents of received
    if (received.empty()) {
        return nullptr;
    }

    KeyValue * key_value = nullptr;
    std::istringstream stringstream(received);
    std::string line;
    string remainder;
    while (std::getline(stringstream, line)) {
        if (stringstream.good()) {
            key_value = parse_json(line);
            if (key_value) {
                auto pos = stringstream.tellg();
                remainder = received.substr(pos);
                break;
            }
        }
        else {
            // the data is only partial, put it back for the next deserialize
            remainder = line;
        }
    }

    received = remainder;
    return key_value;
}

void add_string(const string& name, const string& value, rapidjson::Document& doc) {
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    
    const char* something = name.c_str();
    
}

string KeyValueClient::serialize(KeyValue * key_value) {
    rapidjson::Document doc;
    doc.SetObject();
    
    rapidjson::Document::AllocatorType& allocator = doc.GetAllocator();
    if (key_value->row != KeyValue::NOT_TABLE_ENTRY && key_value->column != KeyValue::NOT_TABLE_ENTRY) {
        doc.AddMember("r", key_value->row, allocator);
        doc.AddMember("c", key_value->column, allocator);
    }
    if (!key_value->key.empty()) {
        rapidjson::Value v(rapidjson::kObjectType);
        v.SetString(key_value->key.c_str(), allocator);
        doc.AddMember("k", v, allocator);
    }
    if (!key_value->value.empty()) {
        rapidjson::Value v(rapidjson::kObjectType);
        v.SetString(key_value->value.c_str(), allocator);
        doc.AddMember("v", v, allocator);
    }
    if (!key_value->function.empty()) {
        rapidjson::Value v(rapidjson::kObjectType);
        v.SetString(key_value->function.c_str(), allocator);
        doc.AddMember("f", v, allocator);
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);
    return buffer.GetString();
}

void KeyValueClient::execute_send(int sock_fd) {
    pollfd ufds[1];
    ufds[0].fd = sock_fd;
    ufds[0].events = POLLOUT;

    timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 1000000;  // 1 millisecond delay
    long counter = 0;

    while (true) {
        while (KeyValue * key_value = next_send()) {
            int poll_result = cross_poll(ufds, 1, 500);
            if (poll_result == -1) {
                set_connect_error(SEND_POLL_ERROR);
                set_keep_going(false);
                cerr << "send poll error" << endl;
                break;
            }

            if (poll_result == 0) {
                counter += 1;
                // cout << "send timeout " << counter << endl;
            }
            else {
                string key_value_string = serialize(key_value) + "\n";
                delete key_value;
                const char * buffer = key_value_string.c_str();
                int sent = 0;
                switch (role) {
                    case Role::SERVER:
                    case Role::CLIENT:
                        sent = ::send(sock_fd, buffer, key_value_string.size(), 0);
                        break;
#ifdef USING_SSL
                    case Role::SSL_SERVER:
                        sent = SSL_write((SSL *) ssl, buffer, key_value_string.size());
                        break;
#endif
                }

                if (sent != key_value_string.size()) {
                    set_connect_error(SEND_COUNT_FAILURE);
                    set_keep_going(false);
                    cerr << "send count failure" << endl;
                    break;
                }

               // cout << "sent:" << key_value_string << endl;
            }

        }

        if (!keep_going()) {
            break;
        }
        if (!keep_sending()) {
            break;
        }

#ifdef _WINDOWS
        Sleep(1);
#else
        nanosleep(&delay, nullptr);
#endif
    }

    cout << "exited send thread" << endl;
}

void KeyValueClient::execute_receive(int sock_fd) {
    pollfd ufds[1];
    ufds[0].fd = sock_fd;
    ufds[0].events = POLLIN;
    char buffer[512];
    long counter = 0;

    while (true) {
        int poll_result = cross_poll(ufds, 1, 500);
        if (poll_result == -1) {
            set_connect_error(RECEIVE_POLL_ERROR);
            set_keep_going(false);
            cerr << "receive poll error" << endl;
            break;
        }

        if (poll_result == 0) {
            counter += 1;
            // cout << "receive timeout " << counter << endl;
        }
        else {
            if (ufds[0].events & POLLIN) {
                counter = 0;
                int received_count = 0;
                switch (this->role) {
                    case Role::SERVER:
                    case Role::CLIENT:
                        received_count = recv(sock_fd, buffer, sizeof buffer, 0);
                        break;
#ifdef USING_SSL
                    case Role::SSL_SERVER:
                        received_count = SSL_read((SSL *) ssl, buffer, sizeof buffer);
                        break;
#endif
                }
                if (received_count <= 0) {
                    cerr << "remote disconnected while looking for incoming" << endl;
                    set_connect_error(SERVER_DISCONNECTED);
                    if (is_server()) {
                        set_keep_sending(false);
                        cross_close(sock_fd);
#ifdef USING_SSL
                        // accept_bio seems to already be released, so clear it before calling close_ssl?
                        this->accept_bio = nullptr;
                        close_ssl();
#endif
                        allow_new_connections();
                    }
                    else {
                        set_keep_going(false);
                    }
                    break;
                }

                received_so_far.append(buffer, received_count);

                // cout << "so far:" << received_so_far << endl;
                while (KeyValue * key_value = deserialize(received_so_far)) {
                    lock_guard<mutex> guard(this->received_values_mutex);
                    received_key_values.emplace_back(key_value);
                }

            }
        }

        if (!keep_going()) {
            break;
        }
    }

    cout << "exited receive thread" << endl;
}

bool KeyValueClient::connect(Role pending_role, const string & ip_address, const string & port, const string & cert_pem, const string & key_pem) {
    if (connect_thread != nullptr) {
        return false;
    }

    string use_ip_address = (pending_role == Role::CLIENT ? ip_address : "localhost");

    cout << "attempting to connect to " << use_ip_address << ":" << port << " as " << (pending_role == Role::CLIENT ? "client" : "server") << endl;

    set_keep_going(true);
    set_keep_sending(true);
    // this->role isn't set until the execute_connect thread runs
    connect_thread = new thread(&KeyValueClient::execute_connect, this, pending_role, use_ip_address, port, cert_pem, key_pem);
    return true;
}

ConnectError KeyValueClient::connect_result() {
    lock_guard<mutex> guard(this->connect_result_mutex);
    return connect_error;
}

void KeyValueClient::disconnect() {
    if (connect_thread == nullptr) {
        return;
    }

    cout << "disconnecting" << endl;

    set_keep_going(false);

    connect_thread->join();
    delete connect_thread;
    connect_thread = nullptr;
    lock_guard<mutex> guard(thread_mutex);
    if (send_thread) {
        send_thread->join();
        delete send_thread;
        send_thread = nullptr;
    }
    if (receive_thread) {
        receive_thread->join();
        delete receive_thread;
        receive_thread = nullptr;
    }

    if (is_server()) {
        if (this->remote_fd >= 0) {
#ifdef USING_SSL
            close_ssl();
#endif
            cross_close(this->remote_fd);
            this->remote_fd = 0;
        }
    }

    cross_close(this->local_fd);
    this->local_fd = unused_socket;
}

KeyValue * KeyValueClient::next_send() {
    lock_guard<mutex> guard(this->send_values_mutex);
    if (send_key_values.empty()) {
        return nullptr;
    }

    KeyValue * keyValue = send_key_values.front();
    send_key_values.pop_front();

    return keyValue;
};

void KeyValueClient::send(const KeyValue & key_value) {
    lock_guard<mutex> guard(this->send_values_mutex);
    auto q_key_value = new KeyValue(key_value);
    send_key_values.emplace_back(q_key_value);
}

bool KeyValueClient::open_ui_file(const string& ui_filename) {
    if (!cache_ui_file(ui_filename)) {
        return false;
    }

    display_ui_file(ui_filename);
    return true;
}

bool KeyValueClient::cache_ui_file(const string & ui_filename) {
    std::ifstream ui_stream(ui_filename);
    if (!ui_stream.is_open()) {
        cout << "unable to open " << ui_filename << endl;
        return false;
    }

    std::stringstream buffer;
    buffer << ui_stream.rdbuf();
    ui_stream.close();

    KeyValue key_value;
    key_value.function = "cache_ui_file";
    key_value.key = ui_filename;
    key_value.value = buffer.str();
    this->send(key_value);
    return true;
}

void KeyValueClient::uncache_ui_file(const string& ui_filename) {
    // assumes cache_ui_file was previously invoked with the given ui_filename
    KeyValue key_value;
    key_value.function = "uncache_ui_file";
    key_value.key = ui_filename;
    this->send(key_value);
}

void KeyValueClient::display_ui_file(const string& ui_filename) {
    // assumes cache_ui_file was previously invoked with the given ui_filename
    KeyValue key_value;
    key_value.function = "display_ui_file";
    key_value.key = ui_filename;
    this->send(key_value);
}


void KeyValueClient::close_ui_file(const string& ui_filename) {
    KeyValue key_value;
    key_value.function = "close_ui_file";
    key_value.key = ui_filename;
    this->send(key_value);
}

void KeyValueClient::enable_widget(const string& widget_name) {
    KeyValue key_value;
    key_value.function = "enable_widget";
    key_value.key = widget_name;
    key_value.value = "true";
    this->send(key_value);
}

void KeyValueClient::disable_widget(const string& widget_name) {
    KeyValue key_value;
    key_value.function = "enable_widget";
    key_value.key = widget_name;
    key_value.value = "false";
    this->send(key_value);
}

void KeyValueClient::show_widget(const string& widget_name) {
    KeyValue key_value;
    key_value.function = "show_widget";
    key_value.key = widget_name;
    key_value.value = "true";
    this->send(key_value);
}

void KeyValueClient::hide_widget(const string& widget_name) {
    KeyValue key_value;
    key_value.function = "show_widget";
    key_value.key = widget_name;
    key_value.value = "false";
    this->send(key_value);
}

void KeyValueClient::save_key_value_file(const string& key_value_filename) {
    KeyValue key_value;
    key_value.function = "get_state";
    key_value.key = key_value_filename;
    this->send(key_value);
}

bool KeyValueClient::load_key_value_file(const string& key_value_filename) {

#ifdef WIN32
    string mode = "rb";
#else
    string mode = "r";
#endif

    FILE* fp = fopen(key_value_filename.c_str(), mode.c_str()); // non-Windows use "r"
    char readBuffer[4096];
    rapidjson::FileReadStream read_stream(fp, readBuffer, sizeof(readBuffer));

    rapidjson::Document doc;
    rapidjson::ParseResult parse_ok = doc.ParseStream(read_stream);
    fclose(fp);

    if (!parse_ok) {
        cerr << "Parse error:" << rapidjson::GetParseError_En(parse_ok.Code()) << " " << parse_ok.Offset() << " in " << key_value_filename << std::endl;
        return false;
    }

    rapidjson::StringBuffer buffer;
    rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
    doc.Accept(writer);

    KeyValue key_value;
    key_value.function = "put_state";
    key_value.value = buffer.GetString();
    this->send(key_value);
    return true;
}

void KeyValueClient::set_label(const string& widget_name, const string& label) {
    KeyValue key_value;
    key_value.function = "set_label";
    key_value.key = widget_name;
    key_value.value = label;
    this->send(key_value);
}

void KeyValueClient::set_stylesheet(const string& widget_name, const string& stylesheet) {
    KeyValue key_value;
    key_value.function = "set_stylesheet";
    key_value.key = widget_name;
    key_value.value = stylesheet;
    this->send(key_value);
}

KeyValue * KeyValueClient::next_received() {
    lock_guard<mutex> guard(this->received_values_mutex);
    if (received_key_values.empty()) {
        return nullptr;
    }

    KeyValue * key_value = this->received_key_values.front();
    this->received_key_values.pop_front();
    if (key_value->function == "get_state") {
        string filename = key_value->key;
        std::ofstream out(filename);
        if (!out.is_open()) {
            cout << "unable to open " << filename << endl;
            return nullptr;
        }

        rapidjson::Document doc;
        rapidjson::ParseResult parse_ok = doc.Parse(key_value->value.c_str());
        if (!parse_ok) {
            cerr << "Parse error:" << rapidjson::GetParseError_En(parse_ok.Code()) << " " << parse_ok.Offset() << " in " << key_value->value << std::endl;
            return nullptr;
        }

        rapidjson::StringBuffer buffer;
        rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
        doc.Accept(writer);

        out << buffer.GetString();
        out.close();

        return nullptr;
    }

    return key_value;
}