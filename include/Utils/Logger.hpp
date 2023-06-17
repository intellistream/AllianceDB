// Copyright (C) 2021 by the INTELLI team (https://github.com/INTELLI)

#ifndef INTELLI_INCLUDE_UTILS_LOGGER_HPP_
#define INTELLI_INCLUDE_UTILS_LOGGER_HPP_

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>

extern FILE* g_log;

#define DEBUG(msg, ...)                                                             \
    {                                                                             \
        std::time_t t = std::time(nullptr);                                       \
        char buf[12];                                                             \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));          \
        fprintf(g_log, "[DEBUG] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
        fflush(g_log);                                                            \
    }

#define INFO(msg, ...)                                                              \
    {                                                                               \
        std::time_t t = std::time(nullptr);                                         \
        char buf[12];                                                               \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));            \
        fprintf(stderr, "[INFO] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
    }

#define ERROR(msg, ...)                                                              \
    {                                                                                \
        std::time_t t = std::time(nullptr);                                          \
        char buf[12];                                                                \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));             \
        fprintf(stderr, "[ERROR] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
    }

#define FATAL(msg, ...)                                                              \
    {                                                                                \
        std::time_t t = std::time(nullptr);                                          \
        char buf[12];                                                                \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));             \
        fprintf(stderr, "[ERROR] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
        exit(1);                                                                     \
    }

#ifdef DEBUG
#    define TRACE(TEXT) std::cerr << TEXT << std::endl;
#else
#    define TRACE(TEXT) ((void)0)
#endif

#ifdef DEBUG
#    define VERIFY(CONDITION, TEXT)                                                            \
        do                                                                                     \
        {                                                                                      \
            if (!(CONDITION))                                                                  \
            {                                                                                  \
                AllianceDB::collectAndPrintStacktrace();                                       \
                INTELLI_FATAL_ERROR(TEXT);                                                     \
                throw std::runtime_error("AllianceDB Runtime Error on condition " #CONDITION); \
            }                                                                                  \
        } while (0)
#else
#    define VERIFY(CONDITION, TEXT) ((void)0)
#endif

#endif  // INTELLI_INCLUDE_UTILS_LOGGER_HPP_
