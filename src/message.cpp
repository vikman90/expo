// October 3, 2018

#include <expo.h>

Message::Message(const Header & header) : counter(ntohl(header.counter)), type((Message::Type)ntohl(header.type)), dest(ntohl(header.dest)), argument(ntohl(header.argument)) {
    data.resize(ntohl(header.length));
}

/*
void Message::clear() {
    type = None;
    counter = dest = argument = 0;
    data.clear();
}
*/

string Message::payload() const {
    Header header = {
        htonl(type),
        htonl(counter),
        htonl(dest),
        htonl(argument),
        htonl(data.length())
    };

    return string((char *)&header, sizeof(header)) + data;
}
