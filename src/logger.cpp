// September 29, 2018

#include <logger.h>
#include <iostream>

Logger::Logger(LogLevel level) : level(level) {
    if (level <= level_threshold) {
        switch (level) {
        case Error:
            buffer << "[\e[31mERROR\e[0m] ";
            break;
        case Warn:
            buffer << "[\e[33mWARN\e[0m] ";
            break;
        case Info:
            buffer << "[\e[32mINFO\e[0m] ";
            break;
        case Debug:
            buffer << "[\e[34mDEBUG\e[0m] ";
            break;
        }
    }
}

Logger::~Logger() {
    if (level <= level_threshold) {
        buffer << std::endl;
        std::cerr << buffer.str();
    }
}

void Logger::logLevel(LogLevel level) {
    level_threshold = level;
}

LogLevel Logger::level_threshold = Info;
