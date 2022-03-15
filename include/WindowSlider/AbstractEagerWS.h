/*! \file AbstractEagerWS.h*/

/*!
    An abstraction of eager window slider


    \todo add the result collection in the future

*/

#ifndef WINDOWSLIDER_ABSTRACTEAGERWS_H_
#define WINDOWSLIDER_ABSTRACTEAGERWS_H_

#include <WindowSlider/AbstractWS.h>
#include <JoinProcessor/SimpleHashJP.h>
using namespace INTELLI;
using namespace std;
namespace INTELLI {
//Note: "Upon every arrival of a tuple, the opposing window is re-partitioned to perform a parallel scan"
/**
 *@defgroup WINDOWSLIDER WindowSliders
 * @{
 **/
/**
 * @defgroup WINDOWSLIDER_EAGER eager window slider
 * @{
 * The eager sliders that follow tuple-wide update, i.e., they process each tuples upon it arrives
 * @}
 * @}
 */
 /**
  * @ingroup WINDOWSLIDER_EAGER
* @class AbstractEagerWS WindowSlider/AbstractEagerWS.h
* @brief An abstraction of eager window slider, also inherited by other eager window slider
* @author Tony Zeng
* @note
* detailed description:
 To init and run, follow the functions below to start a WS
   \li Configure the window type, time or count, @ref setTimeBased
   \li Configure window length: @ref setWindowLen
   \li Set parallel executing behavior on SMP,@ref setParallelSMP
   \li Optional, (@ref setRunTimeScheduling)
   \li To make the parallel join processors started, @ref initJoinProcessors
   \li Feed tuples @ref feedTupleS or @ref feedTupleR
   \li Terminate, by @ref terminateJoinProcessors
*
*/
class AbstractEagerWS: public AbstractWS{
 private:
  /* data */
  std::vector<SimpleHashJPPtr> jps;
 protected:
  void expireS(size_t cond);
  void expireR(size_t cond);
 public:
  //generate the partition vector of offset
  vector<size_t> weightedPartitionSizeFinal(size_t inS); //reserved for AMP
  vector<size_t> avgPartitionSizeFinal(size_t inS); //for SMP

  AbstractEagerWS() {
    reset();
  }

  //init with length of queue
  /**
 * @brief to init the slider with specific length of queue
  * @param sLen the length of S queue
   * @param rLen the length of R queue
 */
  AbstractEagerWS(size_t sLen, size_t rLen);
  //feed the tuple S
  /**
* @brief to feed a tuple s
 * @param ts the tuple s
  * @note this function is thread-safe :)
*/
  virtual void feedTupleS(TuplePtr ts);
  //feed the tuple R
  /**
* @brief to feed a tuple R
 * @param tr the tuple r
  * @note this function is thread-safe :)
*/
  virtual void feedTupleR(TuplePtr tr);
  ~AbstractEagerWS();

  //init the join processors
  /**
 * @brief to init the initJoinProcessors
  * @note only after this is called can we start to feed tuples
 */
  void initJoinProcessors();
  /**
 * @brief to terminate the join processors
 */
  void terminateJoinProcessors();
  void waitAckFromJoinProcessors();
  //get the join result
  /**
* @brief to get the result of join
   *  @result how many tuples are joined
 * @note only called after all join processors are stopped
   * ,use @ref terminateJoinProcessors to achieve this
*/
  size_t getJoinResult();
  /**
  * @brief set the length of slide
  * @param sli The assigned length
  */
  void setSlideLen(size_t sli)
  {
   cout<<sli<<endl;
  }
  //startTime
};
}
#endif
/**
 * @}
 */
/**@}*/