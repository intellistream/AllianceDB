/*! \file Executor.h*/
#ifndef _INCLUDE_UTILS_ABSTRACTC20THREAD_H_
#define _INCLUDE_UTILS_ABSTRACTC20THREAD_H_
#pragma once
#include <thread>
#include <memory>
/**
 * @defgroup INTELLI_UTIL Shared Utils with other Intelli Stream programs
 * @{
 * This group provides common functions to support the Intelli Stream programs.
 */
/**
* @defgroup INTELLI_UTIL_OTHERC20 Other common class or package under C++20 standard
* @{
 * This package covers some common C++20 new features, such as std::thread to ease the programming
*/
namespace AllianceDB {
/**
 * @ingroup INTELLI_UTIL_OTHERC20
 * @class Executor  Utils/Executor.h
 * @brief The base class and abstraction of C++20 thread,
 * and it can be derived into other threads
 */
class Executor {
 protected:
  virtual void Process() = 0;
 private:
  std::unique_ptr<std::thread> threadPtr;
 public:
  Executor() = default;
  ~Executor() = default;

  void Start() {
    threadPtr = std::make_unique<std::thread>(&Executor::Process, this);
  }
  void Join() {
    threadPtr->join();
  }
};
}
/**
 * @}
 */
/**
 * @}
 */
#endif //ALIANCEDB_INCLUDE_UTILS_ABSTRACTTHREAD_H_
