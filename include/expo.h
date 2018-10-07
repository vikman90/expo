// September 29, 2018

#ifndef EXPO_H
#define EXPO_H

#include <iostream>
#include <cstdlib>
#include <climits>
#include <cstring>
#include <unistd.h>
#include <sstream>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <vector>
#include <map>
#include <list>
#include <random>

using namespace std;

#include "logger.h"
#include "exception.h"
#include "message.h"
#include "socket.h"
#include "phrase.h"

enum ExpoMode { Client, Daemon };

class Expo {
public:
    Expo();
    ~Expo();

    void addClient(const char * host, int port = DEFAULT_PORT);
    void addNode(const string & name, Socket * sock);
    void closePeer(Socket * sock);
    void bindHost(const char * host);
    void bindPort(int port);
    void handler(NetSocket * sock);
    void handshakeActive(NetSocket * client);
    void handshakePassive(NetSocket * client);
    void loop();
    Message parse(Message & message, Socket * sock);
    void removeNode(const string & name, Socket * sock);
    void sendCluster(NetSocket * sock);
    void threadStopped();
    const string & getName() const;
    void setName(const char * name);

private:

    unsigned nextCounter();
    void startThread(void (*routine)(Expo *, NetSocket *), NetSocket * sock);
    void startServerThread(Socket * sock);
    void waitThreads();

    string name;
    unsigned counter;
    NetSocket server;
    int threadCount;
    mutex counter_mtx;
    mutex threadCount_mtx;
    condition_variable threadStopped_cv;
    vector<NetSocket *> clients;
    list<Socket *> peers;
    map<string, Socket *> cluster;
    mutex cluster_mtx;
};

// extern Expo expo;

#endif
