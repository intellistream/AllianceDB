// Copyright (C) 2021 by the INTELLI team (https://github.com/INTELLI)

#ifndef INTELLI_INCLUDE_UTILS_LOGGER_HPP_
#define INTELLI_INCLUDE_UTILS_LOGGER_HPP_

#include <cstdio>
#include <iostream>

// Define a macro for logging messages at the "info" level
#define INFO(msg, ...) printf("[INFO] %s: " msg "\n", __func__, ##__VA_ARGS__)

// Define a macro for logging messages at the "error" level
#define ERROR(msg, ...) fprintf(stderr, "[ERROR] %s: " msg "\n", __func__, ##__VA_ARGS__)


#ifdef DEBUG
#define TRACE(TEXT) std::cout << TEXT << std::endl;
#else
#define TRACE(TEXT) ((void)0)
#endif

#ifdef DEBUG
#define VERIFY(CONDITION, TEXT)                                                \
  do {                                                                         \
    if (!(CONDITION)) {                                                        \
      AllianceDB::collectAndPrintStacktrace();                                 \
      INTELLI_FATAL_ERROR(TEXT);                                               \
      throw std::runtime_error(                                                \
          "AllianceDB Runtime Error on condition " #CONDITION);                \
    }                                                                          \
  } while (0)
#else
#define VERIFY(CONDITION, TEXT) ((void)0)
#endif

#endif // INTELLI_INCLUDE_UTILS_LOGGER_HPP_
