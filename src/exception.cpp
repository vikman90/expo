// October 7, 2018

#include <expo.h>

Exception::Exception(const string & func, const string & file, int lineno, int _errno, const char * message) : func(func), file(file), lineno(lineno), _errno(_errno), message(message ? message : strerror(_errno)) {
}

Exception::Exception(const string & func, const string & file, int lineno, int _errno, const string & message) : func(func), file(file), lineno(lineno), _errno(_errno), message(message) {
}

Exception::Exception(const string & func, const string & file, int lineno, const string & message) : func(func), file(file), lineno(lineno), message(message) {
}

ostream & operator << (ostream & os, const Exception & e) {
#ifdef NDEBUG
    os << e.message << " (" << e._errno << ")";
#else
    os << "[" << e.file << ":" << e.lineno << "] At " << e.func << "(): " << e.message << " (" << e._errno << ")";
#endif
    return os;
}
