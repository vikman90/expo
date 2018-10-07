// September 29, 2018

#ifndef LOGGER_H
#define LOGGER_H

#include <expo.h>

enum LogLevel { Error, Warn, Info, Debug };

class Logger {
public:
    Logger(LogLevel level);
    ~Logger();

    template <typename T> Logger & operator << (T const & value) {
        buffer << value;
        return *this;
    }

    static void logLevel(LogLevel level);

private:
    int level;
    std::ostringstream buffer;
    static LogLevel level_threshold;
};

#endif
