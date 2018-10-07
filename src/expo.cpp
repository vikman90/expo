// September 29, 2018

#include <expo.h>

static void runClientThread(Expo * expo, NetSocket * socket);
static void runServerThread(Expo * expo, NetSocket * socket);

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

void Expo::handshakeActive(NetSocket * sock) {
    Message message(Message::Hello, name);
    sock->send(message);

    try {
        switch (message = sock->recv(), message.type) {
        case Message::Accept:
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
    cluster[message.data] = sock;
    peers.push_back(sock);
    Logger(Info) << "Connected to " << sock->getHost() << ":" << sock->getPort() << " (" << message.data << ")";
}

void Expo::handshakePassive(NetSocket * sock) {
    Message message = sock->recv();

    switch (message.type) {
    case Message::Hello:
        try {
            if (message.data == this->name) {
                throw Duplicate(HERE);
            }

            addNode(message.data, sock);
            sock->send(Message(Message::Accept, name));
        } catch (Duplicate) {
            Logger(Warn) << "Rejecting node: " << message.data << ": name already in the cluster.";
            sock->send(Message::Reject);
            sleep(1);
            throw Rejected(HERE);
        }

        break;

    default:
        Logger(Warn) << "Unknown message from " << sock->getHost() << ":" << sock->getPort();
        throw Invalid(HERE);
    }

    unique_lock<mutex> lck(cluster_mtx);
    sendCluster(sock);
    peers.push_back(sock);
}

void Expo::loop() {
    Logger(Info) << "Running as node: " << name;

    // Connect clients

    for (unsigned i = 0; i < clients.size(); ++i) {
        Logger(Debug) << "Connecting to " << clients[i]->getHost() << ":" << clients[i]->getPort();
        startThread(runClientThread, clients[i]);
    }

    // Bind server

    try {
        server.bind();
        server.listen();

        {
            string host = server.getHost();

            if (host != "0.0.0.0") {
                Logger(Info) << "Listening to " << host << ":" << server.getPort();
            } else {
                Logger(Info) << "Listening to port " << server.getPort();
            }
        }

        while (true) {
            NetSocket * peer = server.accept();
            startThread(runServerThread, peer);
        }
    } catch (Exception & error) {
        Logger(Warn) << "Cannot bind to " << server.getHost() << ":" << server.getPort() << ": " << error;
    }

    waitThreads();
}

Message Expo::parse(Message & message, Socket * sock) {
    Message response(message.counter);

    switch (message.type) {
    case Message::Add:
        addNode(message.data, sock);
        break;

    case Message::Remove:
        removeNode(message.data, sock);
        break;

    default:
        Logger(Debug) << "Unknown message: " << message.type;
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

unsigned Expo::nextCounter() {
    unique_lock<mutex> lck(counter_mtx);
    return counter++;
}

void Expo::startThread(void (*routine)(Expo *, NetSocket *), NetSocket * sock) {
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
        expo->handshakeActive(sock);
        expo->handler(sock);
    } catch (Invalid) {
    } catch (Rejected) {
        Logger(Warn) << "Cannot connect: node rejected.";
    } catch (Exception e) {
        Logger(Error) << e;
    }

    Logger(Info) << "Node disconnected.";
    expo->closePeer(sock);
    Logger(Info) << "Stopping client thread...";
    expo->threadStopped();
}

void runServerThread(Expo * expo, NetSocket * sock) {
    Logger(Debug) << "Server thread started.";

    try {
        expo->handshakePassive(sock);
        expo->handler(sock);
    } catch (Invalid) {
    } catch (Rejected) {
    } catch (Exception e) {
        Logger(Error) << e;
    }

    Logger(Info) << "Node disconnected.";
    expo->closePeer(sock);
    expo->threadStopped();
}
