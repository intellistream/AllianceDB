// Copyright (C) 2021 by the INTELLI team (https://github.com/INTELLI)

#ifndef INTELLI_INCLUDE_UTILS_LOGGER_HPP_
#define INTELLI_INCLUDE_UTILS_LOGGER_HPP_
// TRACE < DEBUG < INFO < WARN < ERROR < FATAL
#include <iostream>

enum DebugLevel { LOG_NONE, LOG_WARNING, LOG_DEBUG, LOG_INFO, LOG_TRACE };

static std::string getDebugLevelAsString(DebugLevel level) {
  if (level == LOG_NONE) {
    return "LOG_NONE";
  } else if (level == LOG_WARNING) {
    return "LOG_WARNING";
  } else if (level == LOG_DEBUG) {
    return "LOG_DEBUG";
  } else if (level == LOG_INFO) {
    return "LOG_INFO";
  } else if (level == LOG_TRACE) {
    return "LOG_TRACE";
  } else {
    return "UNKNOWN";
  }
}

static DebugLevel getStringAsDebugLevel(std::string level) {
  if (level == "LOG_NONE") {
    return LOG_NONE;
  } else if (level == "LOG_WARNING") {
    return LOG_WARNING;
  } else if (level == "LOG_DEBUG") {
    return LOG_DEBUG;
  } else if (level == "LOG_INFO") {
    return LOG_INFO;
  } else if (level == "LOG_TRACE") {
    return LOG_TRACE;
  } else {
    throw std::runtime_error("Logger: Debug level unknown: " + level);
  }
}

// TRACE < DEBUG < INFO < WARN < ERROR < FATAL
#define LEVEL_TRACE 6
#define LEVEL_DEBUG 5
#define LEVEL_INFO 4
#define LEVEL_WARN 3
#define LEVEL_ERROR 2
#define LEVEL_FATAL 1

#ifdef INTELLI_LOGGING_LEVEL
#if INTELLI_LOGGING_LEVEL >= LEVEL_TRACE
#define INTELLI_TRACE(TEXT)                                                                                                          \
    do {                                                                                                                         \
        LOG4CXX_TRACE(INTELLILogger, TEXT);                                                                                          \
    } while (0)
#else
#define INTELLI_TRACE(TEXT)                                                                                                          \
    do {                                                                                                                         \
        std::ostringstream oss;                                                                                                  \
        ((void) (oss << TEXT));                                                                                                  \
    } while (0)
#endif

#if INTELLI_LOGGING_LEVEL >= LEVEL_DEBUG
#define INTELLI_DEBUG(TEXT)                                                                                                          \
    do {                                                                                                                         \
        LOG4CXX_DEBUG(INTELLILogger, TEXT);                                                                                          \
    } while (0)
#else
#define INTELLI_DEBUG(TEXT)                                                                                                          \
    do {                                                                                                                         \
        std::ostringstream oss;                                                                                                  \
        ((void) (oss << TEXT));                                                                                                  \
    } while (0)
#endif

#if INTELLI_LOGGING_LEVEL >= LEVEL_INFO
#define INTELLI_INFO(TEXT)                                                                                                           \
    do {                                                                                                                         \
        LOG4CXX_INFO(INTELLILogger, TEXT);                                                                                           \
    } while (0)
#else
#define INTELLI_INFO(TEXT)                                                                                                           \
    do {                                                                                                                         \
        std::ostringstream oss;                                                                                                  \
        ((void) (oss << TEXT));                                                                                                  \
    } while (0)
#endif

#if INTELLI_LOGGING_LEVEL >= LEVEL_WARN
#define INTELLI_WARNING(TEXT)                                                                                                        \
    do {                                                                                                                         \
        LOG4CXX_WARN(INTELLILogger, TEXT);                                                                                           \
    } while (0)
#else
#define INTELLI_WARNING(TEXT)                                                                                                        \
    do {                                                                                                                         \
        std::ostringstream oss;                                                                                                  \
        ((void) (oss << TEXT));                                                                                                  \
    } while (0)
#endif

#if INTELLI_LOGGING_LEVEL >= LEVEL_ERROR
#define INTELLI_ERROR(TEXT)                                                                                                          \
    do {                                                                                                                         \
        LOG4CXX_ERROR(INTELLILogger, TEXT);                                                                                          \
    } while (0)
#else
#define INTELLI_ERROR(TEXT)                                                                                                          \
    do {                                                                                                                         \
        std::ostringstream oss;                                                                                                  \
        ((void) (oss << TEXT));                                                                                                  \
    } while (0)
#endif

#if INTELLI_LOGGING_LEVEL >= LEVEL_FATAL
#define INTELLI_FATAL_ERROR(TEXT)                                                                                                    \
    do {                                                                                                                         \
        LOG4CXX_ERROR(INTELLILogger, TEXT);                                                                                          \
    } while (0)
#else
#define INTELLI_FATAL_ERROR(TEXT)                                                                                                    \
    do {                                                                                                                         \
        std::ostringstream oss;                                                                                                  \
        ((void) (oss << TEXT));                                                                                                  \
    } while (0)
#endif
#else
#ifdef USELOG4CXX
#define INTELLI_TRACE(TEXT)                                                                                                          \
        do {                                                                                                                            \
            LOG4CXX_TRACE(INTELLILogger, TEXT);                                                                                          \
        } while (0)
#define INTELLI_DEBUG(TEXT)                                                                                                          \
        do {                                                                                                                         \
            LOG4CXX_DEBUG(INTELLILogger, TEXT);                                                                                          \
        } while (0)
#define INTELLI_TRACE(TEXT)                                                                                                          \
        do {                                                                                                                         \
            LOG4CXX_TRACE(INTELLILogger, TEXT);                                                                                          \
        } while (0)
#define INTELLI_INFO(TEXT)                                                                                                           \
        do {                                                                                                                         \
            LOG4CXX_INFO(INTELLILogger, TEXT);                                                                                           \
        } while (0)
#define INTELLI_WARNING(TEXT)                                                                                                        \
        do {                                                                                                                         \
            LOG4CXX_WARN(INTELLILogger, TEXT);                                                                                           \
        } while (0)
#define INTELLI_ERROR(TEXT)                                                                                                          \
        do {                                                                                                                         \
            LOG4CXX_ERROR(INTELLILogger, TEXT);                                                                                          \
        } while (0)
#define INTELLI_FATAL_ERROR(TEXT)                                                                                                    \
        do {                                                                                                                         \
            LOG4CXX_ERROR(INTELLILogger, TEXT);                                                                                          \
        } while (0)

#endif
#ifndef USELOG4CXX
#define INTELLI_TRACE(TEXT)                                                                                                          \
      std::cout << TEXT << std::endl;
#define INTELLI_DEBUG(TEXT)                                                                                                          \
      std::cout << TEXT << std::endl;
#define INTELLI_TRACE(TEXT)                                                                                                          \
      std::cout << TEXT << std::endl;
#define INTELLI_INFO(TEXT)                                                                                                           \
      std::cout << TEXT << std::endl;
#define INTELLI_WARNING(TEXT)                                                                                                        \
      std::cout << TEXT << std::endl;
#define INTELLI_ERROR(TEXT)                                                                                                          \
      std::cout << TEXT << std::endl;
#define INTELLI_FATAL_ERROR(TEXT)                                                                                                    \
      std::cout << TEXT << std::endl;

#endif
#endif

#ifdef INTELLI_DEBUG
#define INTELLI_VERIFY(CONDITION, TEXT)                                                                                              \
    do {                                                                                                                         \
        if (!(CONDITION)) {                                                                                                      \
            AllianceDB::collectAndPrintStacktrace();                                                                                    \
            INTELLI_FATAL_ERROR(TEXT);                                                                                               \
            throw std::runtime_error("AllianceDB Runtime Error on condition " #CONDITION);                                              \
        }                                                                                                                        \
    } while (0)
#else
#define INTELLI_VERIFY(CONDITION, TEXT) ((void) 0)
#endif

static void setupLogging(std::string logFileName, DebugLevel level) {
  std::cout << "LogFileName: " << logFileName << ", and DebugLevel: " << level << std::endl;
}

#define INTELLI_NOT_IMPLEMENTED()                                                                                                    \
    do {                                                                                                                         \
        INTELLI_ERROR("Function Not Implemented!");                                                                                  \
        throw Exception("not implemented");                                                                                      \
    } while (0)

#endif //INTELLI_INCLUDE_UTILS_LOGGER_HPP_
