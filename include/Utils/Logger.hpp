// Copyright (C) 2021 by the INTELLI team (https://github.com/INTELLI)

#ifndef INTELLI_INCLUDE_UTILS_LOGGER_HPP_
#define INTELLI_INCLUDE_UTILS_LOGGER_HPP_
#include <iostream>

#define ERROR(TEXT)                                                                                             \
      std::cerr << TEXT << std::endl;

#define INFO(TEXT)                                                                                             \
      std::cout << TEXT << std::endl;

#ifdef DEBUG
#define TRACE(TEXT)                                                                                             \
      std::cout << TEXT << std::endl;
#else
#define TRACE(TEXT) ((void) 0)
#endif

#ifdef DEBUG
#define VERIFY(CONDITION, TEXT)                                                                                 \
    do {                                                                                                                \
        if (!(CONDITION)) {                                                                                             \
            AllianceDB::collectAndPrintStacktrace();                                                                    \
            INTELLI_FATAL_ERROR(TEXT);                                                                                  \
            throw std::runtime_error("AllianceDB Runtime Error on condition " #CONDITION);                              \
        }                                                                                                               \
    } while (0)
#else
#define VERIFY(CONDITION, TEXT) ((void) 0)
#endif


#endif //INTELLI_INCLUDE_UTILS_LOGGER_HPP_
