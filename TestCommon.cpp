#include "TestCommon.h"

#include <iostream>
#include <string.h>

using namespace std;

void usage(const string & program_name) {
    cout << "Usage: " << program_name << endl;
    cout << "       [-ui <folder where .ui files are located>]" << endl;
    cout << "       [-h <host, defaults to localhost, other host used only as client>]" << endl;
    cout << "       [-p <port number, defaults to 4567>]" << endl;
    cout << "       [-s <flag only, act as a server (default is to act as a client)>]" << endl;
#ifdef USING_SSL
    cout << "       [-ssl <flag only, act as an SSL server>]" << endl;
    cout << "       [-cert <path to cert file if -ssl option is used>]" << endl;
    cout << "       [-key <path to key file if -ssl option is used>]" << endl;
#endif
    cout << endl;
    cout << "  Before you run " << program_name << " as a client, first run key_value_gui with its port set to its default of 4567." << endl;
    cout << "  (Or if key_value_client is to use a different port, use the -p command-line argument when you run " << program_name << ".) " << endl;
    cout << "  Then run " << program_name << "." << endl;
    cout << endl;
    cout << "  If you run " << program_name << " as a server, then start " << program_name << " first"  << endl;
    cout << "  Except for starting key_value_gui second, follow the other key_value_gui instructions above." << endl;
}

Params collect_params(int argc, char* argv[]) {
    Params params;
    for (int ix = 0; ix < argc; ix++) {
        if (strcmp(argv[ix], "-s") == 0 || strcmp(argv[ix], "-server") == 0 || strcmp(argv[ix], "--server") == 0) {
            params.role = KeyValueClient::Role::SERVER;
            continue;
        }

#ifdef USING_SSL
        if (strcmp(argv[ix], "-ssl") == 0 || strcmp(argv[ix], "--ssl") == 0) {
            params.role = KeyValueClient::Role::SSL_SERVER;
            continue;
        }
#endif

        if (ix >= argc - 1) {
            continue;
        }

        if (strcmp(argv[ix], "-ui") == 0 || strcmp(argv[ix], "--ui") == 0) {
            params.ui_folder = argv[ix + 1];
            continue;
        }

        if (strcmp(argv[ix], "-p") == 0 || strcmp(argv[ix], "-port") == 0 || strcmp(argv[ix], "--port") == 0) {
            int candidate = stoi(argv[ix + 1]);
            if (candidate > 0) {
                params.port = argv[ix + 1];
            }
            continue;
        }

        if (strcmp(argv[ix], "-h") == 0 || strcmp(argv[ix], "-host") == 0 || strcmp(argv[ix], "--host") == 0) {
            params.host = argv[ix + 1];
            continue;
        }

        if (strcmp(argv[ix], "-cert") == 0 || strcmp(argv[ix], "--cert") == 0) {
            params.cert_file = argv[ix + 1];
            continue;
        }

        if (strcmp(argv[ix], "-key") == 0 || strcmp(argv[ix], "--key") == 0) {
            params.key_file = argv[ix + 1];
            continue;
        }

    }

    if (params.ui_folder.empty()) {
        cout << "-ui command line parameter unspecified; .ui files won't be loaded correctly";
    }

    return params;
}

void sleep_one_millisecond() {
#ifdef _WINDOWS
    Sleep(1);
#else
    timespec delay;
    delay.tv_sec = 0;
    delay.tv_nsec = 1000000;
    nanosleep(&delay, nullptr);
#endif
}