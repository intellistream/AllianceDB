//
// Created by tony on 28/02/22.
//

#ifndef _WINDOWSLIDER_HANDSHAKEWS_H_
#define _WINDOWSLIDER_HANDSHAKEWS_H_
#include <cstdint>
#include <vector>
#include <Common/Types.h>
#include <Utils/SPSCQueue.hpp>
#include <time.h>
#include <numeric>
#include <JoinProcessor/HandShakeHashJP.h>
#include <WindowSlider/AbstractEagerWS.h>

using namespace INTELLI;
using namespace std;
namespace INTELLI {
/**
 *  @ingroup WINDOWSLIDER_EAGER
* @class HandShakeWS WindowSlider/HandShakeWS.h
* @brief The window slider especially for handshake join
* @author Tony Zeng
* @note
* init and run:
 @see AbstractEagerWS
*/
/*! Class that is inherited using public inheritance */
class HandShakeWS : public AbstractEagerWS {
 private:
  /* data *
   */
 protected:
  std::vector<HandShakeHashJPPtr> jpPtr;
  INTELLI::BarrierPtr initBar;
 public:
  HandShakeWS(/* args */) {
    reset();
  }
  ~HandShakeWS() {

  }
  //init with length of queue
  HandShakeWS(size_t sLen, size_t rLen) : AbstractEagerWS(sLen, rLen) {

  }

  //init the join processors
  void initJoinProcessors();
  void terminateJoinProcessors();
  void waitAckFromJoinProcessors();
  //get the join result
  size_t getJoinResult();
  //input
  //note: S->, R<-
  void feedTupleS(TuplePtr ts);
  void feedTupleR(TuplePtr tr);
};

}
#endif //HYBRID_JOIN_INCLUDE_WINDOWSLIDER_HANDSHAKEWS_H_
