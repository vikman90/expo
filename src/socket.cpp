// September 29, 2018

#include <expo.h>

Socket::Socket(int domain) {
    if (sock = socket(domain, SOCK_STREAM, 0), sock < 0) {
        throw ::Exception(HERE, errno);
    }
}

Socket::~Socket() {
    close(sock);
}

void Socket::listen(int backlog) {
    if (::listen(sock, backlog) < 0) {
        throw ::Exception(HERE, errno);
    }
}

void Socket::send(const Message & message) {
    string payload = message.payload();

    if (::send(sock, payload.data(), payload.length(), 0) != payload.length()) {
        throw ::Exception(HERE, errno);
    }
}

Message Socket::recv() {
    Message::Header header;
    recv(&header, sizeof(header));

    Message message(header);

    if (message.data.size()) {
        recv(&message.data[0], message.data.size());
    }

    return message;
}

void Socket::recv(void * buffer, size_t length) {
    switch (::recv(sock, buffer, length, MSG_WAITALL)) {
    case -1:
        throw ::Exception(HERE, errno);
    case 0:
        throw Closed(HERE);
    }
}

void Socket::setsockopt(int name, int value) {
    if (::setsockopt(sock, SOL_SOCKET, name, &value, sizeof(value)) < 0) {
        throw ::Exception(HERE, errno);
    }
}

void Socket::setsockopt(int name, const struct timeval & value) {
    if (::setsockopt(sock, SOL_SOCKET, name, &value, sizeof(value)) < 0) {
        throw ::Exception(HERE, errno);
    }
}

int Socket::getDescriptor() const {
    return sock;
}

int Socket::accept(struct sockaddr * addr, socklen_t addrlen) {
    int sock;

    if (sock = ::accept(this->sock, addr, &addrlen), sock < 0) {
        throw ::Exception(HERE, errno);
    }

    return sock;
}

void Socket::bind(const struct sockaddr * addr, socklen_t addrlen) {
    if (::bind(sock, addr, addrlen) < 0) {
        throw ::Exception(HERE, errno);
    }
}

void Socket::connect(const struct sockaddr * addr, socklen_t addrlen) {
    if (::connect(sock, addr, addrlen) < 0) {
        throw ::Exception(HERE, errno);
    }
}

void Socket::setDescriptor(int sock) {
    if (this->sock >= 0) {
        close(this->sock);
    }

    this->sock = sock;
}

NetSocket::NetSocket(short port, const char * host) : Socket(AF_INET) {
    addr.sin_family = AF_INET;

    setPort(port);
    setHost(host);
}

void NetSocket::bind() {
    setsockopt(SO_REUSEADDR, 1);
    Socket::bind((struct sockaddr *)&addr, sizeof(addr));
}

NetSocket * NetSocket::accept() {
    struct sockaddr_in addr;
    return new NetSocket(Socket::accept((struct sockaddr *)&addr, sizeof(addr)), addr);
}

void NetSocket::connect() {
    Socket::connect((struct sockaddr *)&addr, sizeof(addr));
}

bool NetSocket::isLoopback() const {
    return ntohl(addr.sin_addr.s_addr) == INADDR_LOOPBACK;
}

string NetSocket::getHost() const {
    return inet_ntoa(addr.sin_addr);
}

unsigned short NetSocket::getPort() const {
    return ntohs(addr.sin_port);
}

void NetSocket::setHost(const char * host) {
    if (!inet_aton(host, &addr.sin_addr)) {
        struct hostent * hostent;

        if (hostent = gethostbyname(host), !hostent) {
            throw ::Exception(HERE, (int)h_errno, string("Cannot resolve host: ") + hstrerror(h_errno));
        }

        if (hostent->h_addrtype != AF_INET) {
            throw ::Exception(HERE, string("Unsupported address type."));
        }

        addr.sin_addr = *(struct in_addr *)hostent->h_addr;
    }
}

void NetSocket::setPort(unsigned short port) {
    addr.sin_port = htons(port);
}

NetSocket::NetSocket(int sock, struct sockaddr_in & addr) : addr(addr) {
    setDescriptor(sock);
}

LocalSocket::LocalSocket(const char * path) : Socket(AF_UNIX) {
    addr.sun_family = AF_UNIX;
    strncpy(addr.sun_path, path, sizeof(addr.sun_path));
}
