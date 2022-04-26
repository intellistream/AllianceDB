#ifndef _VERIFYBENCH_H_
#define _VERIFYBENCH_H_

#include <Common/Types.hpp>
#include <Common/Tuple.hpp>
#include <Common/Relations.hpp>
#include  <Common/Result.hpp>
#include <WindowSlider/AbstractEagerWS.h>
#include <WindowSlider/VerifyWS.h>
using namespace INTELLI;
namespace INTELLI {

/**
 * @ingroup INTELLI_COMMON_VERIFY
 * @class Verify  Common/Verify.h
 * @tparam wsType The class of window slider you want to test
 */
class Verify {
 public:
  /**
   *
   * @param joinResult  The detailed result you want to collect
   * @param relationCouple The input data of relationCouple
   * @param threads The parallel threads, default THREAD_NUMBER
   * @param windowLen  The length of sliding window, default WINDOW_SIZE (unit = TIME_STEP us)
   * @param slideLen The slide of window,  default WINDOW_SIZE (unit = TIME_STEP us)
   * @return bool, to indicate whether the verification can pass.
   */
  void Run(Result &joinResult,
           Relations &relationCouple,
           size_t windowLen = WINDOW_SIZE,
           size_t slideLen = WINDOW_SIZE);
};
}

#endif //HYBRID_JOIN_SRC_JOINMETHODS_ABSTRACTJOINMETHOD_H_
