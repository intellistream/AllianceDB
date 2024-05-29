//
// Created by tony on 19/03/22.
//

#include <JoinAlgos/NestedLoopJoin.h>
#include <Utils/UtilityFunctions.hpp>

using namespace INTELLI;
using namespace OoOJoin;

size_t NestedLoopJoin::join(C20Buffer<OoOJoin::TrackTuplePtr> windS, OoOJoin::TrackTuplePtr tr, int threads) {
  assert(threads > 0);
  size_t result = 0;
  size_t tsLen = windS.size();
  for (size_t i = 0; i < tsLen; i++) {
    if (windS.data(i)[0]->key == tr->key) {
      /**
       * @brief handle join sum here
       */
      if (joinSum) {
        result += tr->payload;
      } else {
        result++;
      }

    }
  }
  return result;
}

size_t NestedLoopJoin::join(C20Buffer<OoOJoin::TrackTuplePtr> windS,
                            C20Buffer<OoOJoin::TrackTuplePtr> windR,
                            int threads) {
  size_t result = 0;
  size_t trLen = windR.size();
  for (size_t i = 0; i < trLen; i++) {
    OoOJoin::TrackTuplePtr tr = windR.data(i)[0];
    result += join(windS, tr, threads);
    //tr->processedTime = UtilityFunctions::timeLastUs(timeBaseStruct);
  }
  processedTime= UtilityFunctions::timeLastUs(timeBaseStruct);
  return result;
}