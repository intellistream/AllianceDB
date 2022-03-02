/*! \file UtilityFunctions.hpp*/
// Copyright (C) 2021 by the INTELLI team (https://github.com/intellistream)

#ifndef IntelliStream_SRC_UTILS_UTILITYFUNCTIONS_HPP_
#define IntelliStream_SRC_UTILS_UTILITYFUNCTIONS_HPP_

#include <string>
#include <experimental/filesystem>
#include <barrier>
#include <functional>
#include <JoinMethods/Types.h>


/* Period parameters */
#define N 624
#define M 397
#define MATRIX_A 0x9908b0dfUL   /* constant vector a */
#define UPPER_MASK 0x80000000UL /* most significant w-r bits */
#define LOWER_MASK 0x7fffffffUL /* least significant r bits */
#define TRUE 1
#define FALSE 0
#include <sys/time.h>
namespace INTELLI {
typedef std::shared_ptr<std::barrier<>> BarrierPtr;
#define TIME_LAST_UNIT_MS 1000
#define TIME_LAST_UNIT_US 1000000
class UtilityFunctions {

 public:
  UtilityFunctions();

  static void init_genrand(unsigned long s);

  static double genrand_real3();

  static long genrand_int31(void);

  static unsigned long genrand_int32(void);

  static std::shared_ptr<std::barrier<>> createBarrier(int count);

  static void timerStart(Result &result);

  static void timerEnd(Result &result);

  static  size_t timeLast(size_t past,size_t unitTime);
  static  size_t timeLastUs( struct timeval past);
  static void printTest(char const *name, int context);
  //bind to CPU
  /*!
   bind to CPU
   \li bind the thread to core according to id
   \param id the core you plan to bind, -1 means let os decide
   \return cpuId, the real core that bind to
   \todo unsure about hyper-thread
    */
  static int bind2Core(int id);
  //partition
  static vector<size_t> avgPartitionSizeFinal(size_t inS,std::vector<size_t> partitionWeight);
  static vector<size_t> weightedPartitionSizeFinal(size_t inS, std::vector<size_t> partitionWeight);
};
}
#endif //IntelliStream_SRC_UTILS_UTILITYFUNCTIONS_HPP_
