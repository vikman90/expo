// October 3, 2018

#ifndef MESSAGE_H
#define MESSAGE_H

#include <expo.h>

class Message {
public:
    enum Type { None, Hello, Accept, Reject, Add, Remove };

    struct Header {
        unsigned type;
        unsigned counter;
        unsigned dest;
        unsigned argument;
        unsigned length;
    };

    Message(unsigned counter = 0, Type type = None, unsigned dest = 0, unsigned argument = 0) : counter(counter), type(type), dest(dest), argument(argument) { }
    Message(Type type) : counter(0), type(type), dest(0), argument(0) { }
    Message(Type type, const string & data) : counter(0), type(type), dest(0), argument(0), data(data) { }
    Message(const Header & header);
    //void clear();
    string payload() const;

    unsigned counter;
    Type type;
    unsigned dest;
    unsigned argument;
    string data;
};

#endif
