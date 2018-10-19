// September 29, 2018

#include <expo.h>

void runClientThread(Expo * expo, NetSocket * sock);
void runLocalThread(Expo * expo, LocalSocket * sock);
void runServerThread(Expo * expo, NetSocket * server);

Expo::Expo() : threadCount(0) {
    mt19937 random(chrono::system_clock::now().time_since_epoch().count());
    name = Phrase::name(random);
    counter = random();
}

Expo::~Expo() {
    Logger(Info) << "Exiting...";
}

void Expo::addClient(const char * host, int port) {
    if (port <= 0 || port > SHRT_MAX) {
        throw Exception(HERE, string("Invalid port number " + to_string(port)));
    }

    clients.push_back(new NetSocket(port, host));
}

void Expo::addNode(const string & name, Socket * sock) {
    list<Socket *>::iterator it;
    unique_lock<mutex> lck(cluster_mtx);

    if (cluster.find(name) != cluster.end()) {
        throw Duplicate(HERE);
    }

    Logger(Info) << "New node: " << name;

    cluster[name] = sock;

    for (it = peers.begin(); it != peers.end(); ++it) {
        if (*it != sock) {
            (*it)->send(Message(Message::Add, name));
        }
    }
}

void Expo::removeNode(const string & name, Socket * sock) {
    list<Socket *>::iterator it;
    unique_lock<mutex> lck(cluster_mtx);

    if (cluster.find(name) != cluster.end()) {
        Logger(Info) << "Node lost: " << name;

        cluster[name] = sock;

        for (it = peers.begin(); it != peers.end(); ++it) {
            if (*it != sock) {
                (*it)->send(Message(Message::Remove, name));
            }
        }
    } else {
        Logger(Debug) << "Removing unexisting node: " << name;
    }
}

void Expo::bindHost(const char * host) {
    server.setHost(host);
}

void Expo::bindPort(int port) {
    if (port <= 0 || port > SHRT_MAX) {
        throw Exception(HERE, string("Invalid port number " + to_string(port)));
    }

    server.setPort(port);
}

void Expo::handler(NetSocket * sock) {
    while (1) {
        try {
            Message message = sock->recv();
            message = parse(message, sock);

            if (message.type != Message::None) {
                sock->send(message);
            }
        } catch (Closed & e) {
            break;
        } catch (Exception & error) {
            Logger(Warn) << "Connection error with " << sock->getHost() << ": " << error;
            break;
        }
    }


}

void Expo::handshake(NetSocket * sock) {
    Message message(Message::Hello, name);
    sock->send(message);

    try {
        switch (message = sock->recv(), message.type) {
        case Message::Accept:
            addNode(message.data, sock);
            break;
        case Message::Reject:
            throw Rejected(HERE);
        default:
            throw Exception(HERE, message.data);
        }
    } catch (Closed) {
        throw Exception(HERE, "Server disconnected during handshake.");
    }

    unique_lock<mutex> lck(cluster_mtx);
    sendCluster(sock);
    peers.push_back(sock);
    Logger(Info) << "Connected to " << sock->getHost() << ":" << sock->getPort() << " (" << message.data << ")";
}

void Expo::loop() {
    Logger(Info) << "Running as node: " << name;

    // Connect clients

    for (unsigned i = 0; i < clients.size(); ++i) {
        if (clients[i]->isLoopback() && clients[i]->getPort() == server.getPort()) {
            Logger(Warn) << "Cannot connect to the server itself (" << clients[i]->getHost() << ":" << clients[i]->getPort() << "). Ignoring client.";
        } else {
            Logger(Debug) << "Connecting to " << clients[i]->getHost() << ":" << clients[i]->getPort();
            startThread(runClientThread, clients[i]);
        }
    }

    // Run server threads
    startThread(runServerThread, &server);
    startThread(runLocalThread, &communicator);

    waitThreads();
}

Message Expo::parse(Message & message, NetSocket * sock) {
    Message response(message.counter);

    switch (message.type) {
    case Message::Hello:
        try {
            if (message.data == this->name) {
                throw Duplicate(HERE);
            }

            addNode(message.data, sock);
            sock->send(Message(Message::Accept, name));

            unique_lock<mutex> lck(cluster_mtx);
            sendCluster(sock);
            peers.push_back(sock);
        } catch (Duplicate) {
            Logger(Warn) << "Rejecting node: " << message.data << ": name already in the cluster.";
            sock->send(Message::Reject);
            throw Rejected(HERE);
        }

        break;

    case Message::Add:
        addNode(message.data, sock);
        break;

    case Message::Remove:
        removeNode(message.data, sock);
        break;

    default:
        Logger(Warn) << "Unknown message from " << sock->getHost() << ":" << sock->getPort();
        throw Invalid(HERE);
    }

    return response;
}

void Expo::sendCluster(NetSocket * sock) {
    map<string, Socket *>::iterator it;

    for (it = cluster.begin(); it != cluster.end(); ++it) {
        if (it->first != name && it->second != sock) {
            sock->send(Message(Message::Add, it->first));
        }
    }
}

void Expo::threadStopped() {
    unique_lock<mutex> lck(threadCount_mtx);
    threadCount--;
    threadStopped_cv.notify_one();
}

const string & Expo::getName() const {
    return name;
}

void Expo::setName(const char * name) {
    this->name = name;
}

void Expo::closePeer(Socket * sock) {
    list<Socket *>::iterator peers_it;
    map<string, Socket *>::iterator cluster_it;
    unique_lock<mutex> lck(cluster_mtx);

    peers.remove(sock);

    // For each node related to the closing peer, broadcast a remove message
    for (cluster_it = cluster.begin(); cluster_it != cluster.end(); ) {
        if (cluster_it->second == sock) {
            Logger(Info) << "Node lost: " << cluster_it->first;

            for (peers_it = peers.begin(); peers_it != peers.end(); ++peers_it) {
                (*peers_it)->send(Message(Message::Remove, cluster_it->first));
            }

            cluster_it = cluster.erase(cluster_it);
        } else {
            ++cluster_it;
        }
    }

    delete sock;
}

unsigned Expo::nextCounter() {
    unique_lock<mutex> lck(counter_mtx);
    return counter++;
}

template <class T> void Expo::startThread(void (*routine)(Expo *, T *), T * sock) {
    unique_lock<mutex> lck(threadCount_mtx);
    threadCount++;
    thread dispatcher(routine, this, sock);
    dispatcher.detach();
}

void Expo::waitThreads() {
    unique_lock<mutex> lck(threadCount_mtx);

    while (threadCount) {
        threadStopped_cv.wait(lck);
    }
}

void runClientThread(Expo * expo, NetSocket * sock) {
    Logger(Debug) << "Client thread started.";

    try {
        sock->connect();
        expo->handshake(sock);
        expo->handler(sock);
    } catch (Invalid) {
    } catch (Rejected) {
        Logger(Warn) << "Cannot connect: node rejected.";
    } catch (Exception e) {
        Logger(Error) << e;
    }

    Logger(Info) << "Node disconnected.";
    expo->threadStopped();
}

void runLocalThread(Expo * expo, LocalSocket * comm) {
    SocketPool pool;

    Logger(Debug) << "Local thread started.";

    // Bind communicator

    try {
        comm->bind();
        comm->listen();
        Logger(Info) << "Creating communicator: " << comm->getPath();
    } catch (Exception & error) {
        Logger(Warn) << "Cannot create communicator: " << comm->getPath() << ": " << error;
        expo->threadStopped();
        return;
    }

    pool.add(*comm);

    while (true) {
        vector<Socket *> ready = pool.wait();

        for (int i = 0; i < ready.size(); ++i) {
            LocalSocket * sock = (LocalSocket *)ready[i];

            try {
                if (sock == comm) {
                    Message message = sock->recv();
                    //message = expo->parse(message, sock);

                    if (message.type != Message::None) {
                        sock->send(message);
                    }
                } else {
                    LocalSocket * peer = comm->accept();
                    pool.add(*peer);
                }
            } catch (Closed & e) {
                Logger(Info) << "Node disconnected.";
                expo->closePeer(sock);
            } catch (Invalid) {
            } catch (Exception & e) {
                Logger(Error) << e;
            }
        }
    }

    // Dead code
    Logger(Debug) << "Local thread finished.";
    expo->threadStopped();
}

void runServerThread(Expo * expo, NetSocket * server) {
    SocketPool pool;

    Logger(Debug) << "Server thread started.";

    // Bind server

    try {
        server->bind();
        server->listen();

        if (!server->isAny()) {
            Logger(Info) << "Listening to " << server->getHost() << ":" << server->getPort();
        } else {
            Logger(Info) << "Listening to port " << server->getPort();
        }
    } catch (Exception & error) {
        Logger(Warn) << "Cannot bind to " << server->getHost() << ":" << server->getPort() << ": " << error;
        expo->threadStopped();
        return;
    }

    pool.add(*server);

    while (true) {
        vector<Socket *> ready = pool.wait();

        for (int i = 0; i < ready.size(); ++i) {
            NetSocket * sock = (NetSocket *)ready[i];

            try {
                if (sock == server) {
                    NetSocket * peer = server->accept();
                    pool.add(*peer);
                } else {
                    Message message = sock->recv();
                    message = expo->parse(message, sock);

                    if (message.type != Message::None) {
                        sock->send(message);
                    }
                }
            } catch (Closed & e) {
                Logger(Info) << "Node disconnected.";
                expo->closePeer(sock);
            } catch (Invalid) {
            } catch (Rejected) {
            } catch (Exception & e) {
                Logger(Error) << e;
            }
        }
    }

    // Dead code
    Logger(Debug) << "Server thread finished.";
    expo->threadStopped();
}
