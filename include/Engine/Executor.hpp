#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include <Common/Types.hpp>
#include <Common/Relations.hpp>
#include <WindowSlider/AbstractEagerWS.h>

using namespace INTELLI;
namespace INTELLI {
template<class wsType=AbstractEagerWS>
class Executor {
 public:
  void Run(
      ResultPtr joinResult,
      RelationsPtr relations,
      size_t threads = THREAD_NUMBER,
      size_t windowLen = WINDOW_SIZE,
      size_t slideLen = WINDOW_SIZE);
  void Initialize(
      wsType &windowSlider,
      const RelationsPtr &relations,
      size_t threads,
      size_t windowLen,
      size_t slideLen,
      ResultPtr &joinResult,
      timeval &timeSys) const;
};
}

#endif
