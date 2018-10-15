// September 29, 2018

#ifndef SOCKET_H
#define SOCKET_H

#include <sys/socket.h>
#include <sys/un.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#define UNIX_SOCK "/var/run/expo.sock"
#define DEFAULT_BIND "0.0.0.0"
#define DEFAULT_PORT 1517

class Socket {
public:
    Socket(int domain);
    ~Socket();
    void listen(int backlog = 128);
    void send(const Message & message);
    Message recv();
    void recv(void * buffer, size_t length);
    void setsockopt(int name, int value);
    void setsockopt(int name, const struct timeval & value);
    int getDescriptor() const;

protected:
    Socket() : sock(-1) {}
    int accept(struct sockaddr * addr, socklen_t addrlen);
    void bind(const struct sockaddr * addr, socklen_t addrlen);
    void connect(const struct sockaddr * addr, socklen_t addrlen);
    void setDescriptor(int sock);

private:
    int sock;
};

class NetSocket : public Socket {
public:
    NetSocket(short port = DEFAULT_PORT, const char * host = DEFAULT_BIND);
    NetSocket * accept();
    void bind();
    void connect();
    bool isAny() const;
    bool isLoopback() const;

    string getHost() const;
    unsigned short getPort() const;
    void setHost(const char * host);
    void setPort(unsigned short port);

private:
    NetSocket(int sock, struct sockaddr_in & addr);
    struct sockaddr_in addr;
};

class LocalSocket : public Socket {
public:
    LocalSocket(const char * path = UNIX_SOCK);

private:
    LocalSocket(int sock, struct sockaddr_un & addr);

    struct sockaddr_un addr;
};

#endif
