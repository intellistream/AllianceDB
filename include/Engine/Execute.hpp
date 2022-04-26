#ifndef _EXECUTE_H_
#define _EXECUTE_H_

#include <Common/Types.hpp>
#include <Common/RelationCouple.hpp>
#include <WindowSlider/AbstractEagerWS.h>

using namespace INTELLI;
namespace INTELLI {
template<class wsType=AbstractEagerWS>
class Execute {
 public:
  void Run(Result &joinResult,
           RelationCouple &relationCouple,
           size_t threads = THREAD_NUMBER,
           size_t windowLen = WINDOW_SIZE,
           size_t slideLen = WINDOW_SIZE);
};
}

#endif
