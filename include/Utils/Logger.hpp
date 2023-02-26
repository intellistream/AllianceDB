// Copyright (C) 2021 by the INTELLI team (https://github.com/INTELLI)

#ifndef INTELLI_INCLUDE_UTILS_LOGGER_HPP_
#define INTELLI_INCLUDE_UTILS_LOGGER_HPP_

#include <chrono>
#include <cstdio>
#include <ctime>
#include <iomanip>
#include <iostream>

extern FILE* g_log;

// Define a macro for logging messages at the "info" level
#define LOG(msg, ...)                                                             \
    {                                                                             \
        std::time_t t = std::time(nullptr);                                       \
        char buf[12];                                                             \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));          \
        fprintf(g_log, "[LOG] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
        fflush(g_log);                                                            \
    }

// Define a macro for logging messages at the "info" level
#define INFO(msg, ...)                                                              \
    {                                                                               \
        std::time_t t = std::time(nullptr);                                         \
        char buf[12];                                                               \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));            \
        fprintf(stderr, "[INFO] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
    }

// Define a macro for logging messages at the "error" level
#define ERROR(msg, ...)                                                              \
    {                                                                                \
        std::time_t t = std::time(nullptr);                                          \
        char buf[12];                                                                \
        std::strftime(buf, sizeof(buf), "%H:%M:%S", std::localtime(&t));             \
        fprintf(stderr, "[ERROR] [%s] %s: " msg "\n", buf, __func__, ##__VA_ARGS__); \
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
