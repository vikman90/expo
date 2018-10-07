// October 7, 2018

#ifndef EXCEPTION_H
#define EXCEPTION_H

#define HERE __func__, __FILE__, __LINE__

class Exception {
public:
    Exception(const string & func, const string & file, int lineno, int _errno = 0, const char * message = NULL) ;
    Exception(const string & func, const string & file, int lineno, int _errno, const string & message);
    Exception(const string & func, const string & file, int lineno, const string & message);

    friend ostream & operator << (ostream & os, const Exception & e);

private:
    int _errno;
    int lineno;
    string file;
    string func;
    string message;
};

class Closed : public Exception {
public:
    Closed(const string & func, const string & file, int lineno) : Exception(func, file, lineno, "Socket closed.") { }
};

class Duplicate : public Exception {
public:
    Duplicate(const string & func, const string & file, int lineno) : Exception(func, file, lineno, "Duplicate entry.") { }
};

class Invalid : public Exception {
public:
    Invalid(const string & func, const string & file, int lineno) : Exception(func, file, lineno, "Invalid message.") { }
};

class Rejected : public Exception {
public:
    Rejected(const string & func, const string & file, int lineno) : Exception(func, file, lineno, "Node rejected.") { }
};

#endif
