/*! \file SplitJoinWS.h*/
//
// Created by tony on 22/03/22.
//

#ifndef _WINDOWSLIDER_SPLITIRWS_H_
#define _WINDOWSLIDER_SPLITIRWS_H_

#include <WindowSlider/SplitJoinWS.h>
#include <Utils/Executor.h>
#include <JoinProcessor/SplitJoinIRJP.h>
using namespace AllianceDB;
using namespace std;
namespace AllianceDB {
/**
 * @ingroup WINDOWSLIDER_EAGER
* @class SplitJoinIRWS WindowSlider/SplitJoinIRWS.h
* @brief The eager window slider of split join, which also shares the intermediate results (IR)
* @note
* detailed description:
To init and run, follow the functions below to start a WS
  \li Configure the window type, time or count, @ref setTimeBased
  \li Configure window length: @ref setWindowLen
  \li Configure slide length: @ref setSlideLen (default is 1 if not called)
  \li Set parallel executing behavior on SMP,@ref setParallelSMP
  \li Optional, (@ref setRunTimeScheduling)
  \li To make the parallel join processors started, @ref initJoinProcessors
  \li Feed tuples @ref feedTupleS or @ref feedTupleR
  \li Terminate, by @ref terminateJoinProcessors
*
*/
class SplitJoinIRWS : public SplitJoinWS {
 private:

 public:
  SplitJoinIRWS() {
    reset();
    nameTag = "SplitJoinIR";
  }
  /**
* @brief to init the slider with specific length of queue
 * @param sLen the length of S queue
  * @param rLen the length of R queue
*/
  SplitJoinIRWS(size_t sLen, size_t rLen);

  ~SplitJoinIRWS() {}
  /**
* @brief to init the initJoinProcessors
 * @note only after this is called can we start to feed tuples
*/
  void initJoinProcessors();

};
}
#endif //ALIANCEDB_INCLUDE_WINDOWSLIDER_SPLITWS_H_
