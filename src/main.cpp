// September 29, 2018


#include <expo.h>

using namespace std;

static void help(const char * argv0);
static void version();

int main(int argc, char ** argv) {
#ifndef NDEBUG
    Logger::logLevel(Debug);
#endif

    int c;
    ExpoMode mode = Client;
    char * action_exec = NULL;
    char * action_send = NULL;
    int actions = 0;
    char * token;
    Expo expo;

    // Parse options

    opterr = 0;

    try {
        while (c = getopt(argc, argv, "hdn:b:p:c:x:s:V"), c != -1) {
            switch (c) {
            case 'b':
                expo.bindHost(optarg);
                break;

            case 'c':
                if (token = strchr(optarg, ':'), token) {
                    *token++ = '\0';
                    expo.addClient(optarg, atoi(token));
                } else {
                    expo.addClient(optarg);
                }

                break;

            case 'd':
                mode = Daemon;
                break;

            case 'h':
                help(argv[0]);
                exit(EXIT_SUCCESS);

            case 'n':
                expo.setName(optarg);
                break;

            case 'p':
                expo.bindPort(atoi(optarg));
                break;

            case 's':
                action_send = optarg;
                actions++;
                break;

            case 'x':
                action_exec = optarg;
                actions++;
                break;

            case 'V':
                version();
                exit(EXIT_SUCCESS);

            case '?':
                Logger(Error) << "Invalid usage of option -" << (char)optopt;
                help(argv[0]);
                exit(EXIT_FAILURE);

            default:
                Logger(Error) << "Cannot parse options. This should not occur.";
                exit(EXIT_FAILURE);
            }
        }
    } catch (Exception & e) {
        Logger(Error) << e;
        exit(EXIT_FAILURE);
    }

    // Run Expo

    switch (mode) {
    case Client:
        switch (actions) {
        case 0:
            Logger(Warn) << "No actions defined.";
            help(argv[0]);
            exit(EXIT_FAILURE);

        case 1:
            if (action_send) {
                if (optind + 2 > argc) {
                    Logger(Error) << "No files defined to send.";
                    exit(EXIT_FAILURE);
                }

                char * origin = argv[optind];
                char * target = argv[optind + 1];

                Logger(Info) << "Send file: " << origin << " to " << action_send << ":" << target;
            } else if (action_exec) {
                if (optind + 1 > argc) {
                    Logger(Error) << "No command defined to run.";
                    exit(EXIT_FAILURE);
                }

                char * command = argv[optind];

                Logger(Info) << "Run command at: " << action_exec << ": " << command;
            }

            break;

        default:
            Logger(Warn) << "Can define one action only per call.";
            exit(EXIT_FAILURE);
        }

        break;

    case Daemon:
        try {
            expo.loop();
        } catch (Exception e) {
            Logger(Error) << e;
        }
    }

    return EXIT_SUCCESS;
}

void help(const char * argv0) {
    cout << "Usage: " << argv0 << " <options>" << endl;
    cout << "Options:" << endl;
    cout << "    -h                             Print this help." << endl;
    cout << "    -d                             Run in daemon mode." << endl;
    cout << "    -n                             Name for this node." << endl;
    cout << "    -b <ip>                        Bind to this address." << endl;
    cout << "    -p <port>                      Listen to this port." << endl;
    cout << "    -c <ip>[:<port>]               Connect to a node." << endl;
    cout << "    -V                             Print application version." << endl;
    cout << "Client-only (no daemon) options:" << endl;
    cout << "    -s <node> <origin> <target>    Send a file." << endl;
    cout << "    -x <node> <command>            Run a remote command." << endl;

}

void version() {
    cout << "Expo Platform" << endl;
    cout << "Version: 0.1" << endl;
}
