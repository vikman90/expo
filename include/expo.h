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
#include "socketpool.h"
#include "phrase.h"

enum ExpoMode { Client, Daemon };

class Expo {
public:
    Expo();
    ~Expo();

    void addClient(const char * host, int port = DEFAULT_PORT);
    void addNode(const string & name, Socket * sock);
    void bindHost(const char * host);
    void bindPort(int port);
    void handler(NetSocket * sock);
    void handshake(NetSocket * client);
    void loop();
    Message parse(Message & message, NetSocket * sock);
    void removeNode(const string & name, Socket * sock);
    void sendCluster(NetSocket * sock);
    void threadStopped();
    const string & getName() const;
    void setName(const char * name);

    friend void runClientThread(Expo * expo, NetSocket * sock);
    friend void runLocalThread(Expo * expo, LocalSocket * sock);
    friend void runServerThread(Expo * expo, NetSocket * server);

private:

    void closePeer(Socket * sock);
    unsigned nextCounter();
    template <class T> void startThread(void (*routine)(Expo *, T *), T * sock);
    void waitThreads();

    string name;
    unsigned counter;
    NetSocket server;
    LocalSocket communicator;
    int threadCount;
    mutex counter_mtx;
    mutex threadCount_mtx;
    condition_variable threadStopped_cv;
    vector<NetSocket *> clients;
    list<Socket *> peers;
    map<string, Socket *> cluster;
    mutex cluster_mtx;
};

#endif
