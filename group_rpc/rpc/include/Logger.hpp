#pragma once

#include "AsyQueue.hpp"

#include <memory>

enum LogLevel {
    INFO,
    ERROR
};

class Logger {

public:
    static Logger& GetInstance();

    // 设置日志级别
    void setLogLevel(LogLevel level);
    // 写日志
    void log(std::shared_ptr<std::string> msg);

private:
    Logger();
    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(const Logger&&) = delete;

private:
    int m_logLevel;
    AsyQueue<std::shared_ptr<std::string>> m_queue;

};

#define LOG_INFO(logMsgFormat, ...) \
    do { \
        Logger& logger = Logger::GetInstance(); \
        logger.setLogLevel(INFO); \
        char c[1024] = { 0 }; \
        snprintf(c, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(std::make_shared<std::string>(c)); \
    } while (0)

#define LOG_ERROR(logMsgFormat, ...) \
    do { \
        Logger& logger = Logger::GetInstance(); \
        logger.setLogLevel(ERROR); \
        char c[1024] = { 0 }; \
        snprintf(c, 1024, logMsgFormat, ##__VA_ARGS__); \
        logger.log(std::make_shared<std::string>(c)); \
    } while (0)
