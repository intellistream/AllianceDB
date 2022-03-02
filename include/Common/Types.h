/*! \file Types.h*/
//
// Created by Wang Chenyu on 1/9/21.
//
#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef INTELLISTREAM_TYPES_H
#define INTELLISTREAM_TYPES_H

#ifndef ALGO_NAME

//#define ALGO_NAME "OneWayHashJoin"
#define ALGO_NAME "CellJoin"
//#define ALGO_NAME "HandShakeJoin"

#endif

#ifndef ALGO_CLASS

//#define ALGO_CLASS OneWayHashJoin
#define ALGO_CLASS CellJoin
//#define ALGO_CLASS HandShakeJoin

#endif

//Constants
#ifndef WINDOW_SIZE
#define WINDOW_SIZE 500
#endif

#ifndef THREAD_NUMBER
#define THREAD_NUMBER 2
#endif

#ifndef TIME_STEP
#define TIME_STEP 5// US
#endif
#ifndef DATASET_NAME
#define DATASET_NAME "Test1" //dataset name should be DATASET_NAME + "-R.txt" and DATASET_NAME + "-S.txt"
//in Test2, we manually assigned duplicated keys
#endif

#include <cstdint>
#include <vector>
#include <memory>
#include <mutex>
#include <unordered_map>
#include <queue>
#include "Utils/concurrentqueue.h"
#include "Utils/DupicatedHashTable.hpp"
#include "Utils/SafeQueue.hpp"
#include "Utils/SPSCQueue.hpp"

/**
 * @mainpage Welcome
 * The AianceDB offers a wide range of stream join algorithms, supporting both inter and
 * intra window join.
 * @section Concepts
 * The AianceDB allows direct feeding of tuples, all you need is set up a WindowSlider
 * and then feed tuples.
 * You may wish to check @ref Common for data types, @ref WindowSliders for WindowSliders, these are all that
 * needed to be known as a starting user.
 */

namespace INTELLI {
/**
 * @defgroup Common Common Datastructure
 *  @{
 */
//Pointers
typedef std::shared_ptr<class Tuple> TuplePtr;  /*!< The shared pointer to @ref Tuple */
typedef std::shared_ptr<class RelationCouple> RelationCouplePtr;

//Array Pointers
typedef std::vector<TuplePtr> WindowOfTuples;
//typedef std::SafeQueue<TuplePtr> tupleQueue;
typedef std::SafeQueue<TuplePtr> tupleQueue;
typedef moodycamel::ConcurrentQueue<TuplePtr> concurrentTupleQueue;

//Alias
typedef int keyType;    /*!< Type of the join key, default int */
typedef int valueType;  /*!< Type of the payload, default int */
typedef int numberType; //for counting the number of datagram (eg: tuples) in a struct
typedef std::mutex mutex;
typedef std::DupicatedHashTable<keyType, keyType> hashtable; /*!< allow key duplication */

typedef std::queue<numberType> tupleKeyQueue;
/**
 * @class Tuple Common/Types.h
 * @brief The class to describe a tuple
 */
class Tuple {
 public:
  keyType key; /*!< The key used for relational join*/
  valueType payload; /*!< The payload, can also be pointer*/
  // The subKey can be either time-stamp or arrival count, which is assigned by join system
  // We use the subKey to do tuple expiration, which covers both count-based and time-based window
  size_t subKey = 0;/*!< subkey is preserved for join system, e.g., it can be the time stamp or tuple count*/
  /**
   * @brief construct with key
   * @param k the key
   */
  explicit Tuple(keyType k);
  /**
 * @brief construct with key and value
 * @param k the key
   *@param v the value of payload
 */
  explicit Tuple(keyType k, valueType v);
  /**
 * @brief construct with key, value and subkey
 * @param k the key
   *@param v the value of payload
   * @param sk the subkey
 */
  explicit Tuple(keyType k, valueType v, size_t sk);
  ~Tuple();
};

class RelationCouple {
 public:
  tupleQueue relationS;
  tupleQueue relationR;
  RelationCouple();
  ~RelationCouple();
};

class WindowCouple {
 public:
  //We use queue to implement a window, it stores the sequence of tuples.
  tupleQueue windowS;
  tupleQueue windowR;
  hashtable hashtableS;
  hashtable hashtableR;

  numberType windowSize;
  numberType windowSizeR;
  numberType windowSizeS;

  mutex windowLock;

  WindowCouple(numberType windowSize);
  WindowCouple(numberType windowSizeR, numberType windowSizeS);
};

class ConcurrentQWindowCouple {
 public:
  //We use queue to implement a window, it stores the sequence of tuples.
  //Concurrent Q but not concurrent hashtable
  //A sub-window that can be obtained from other threads
  concurrentTupleQueue windowS;
  concurrentTupleQueue windowR;
  hashtable hashtableS;
  hashtable hashtableR;

  numberType windowSize;
  numberType windowSizeR;
  numberType windowSizeS;

  mutex windowLock;

  ConcurrentQWindowCouple(numberType windowSize);
  ConcurrentQWindowCouple(numberType windowSizeR, numberType windowSizeS);
};

class Result {
 public:
  numberType joinNumber;
  numberType streamSize;
  double timeTaken;
  struct timeval timeBegin;
  explicit Result();
  Result operator++(int);
  void statPrinter();
};
//by tony zeng
typedef std::shared_ptr<INTELLI::SPSCQueue<INTELLI::TuplePtr>> TupleQueuePtr;
typedef std::shared_ptr<std::queue<INTELLI::TuplePtr>> TupleQueueSelfPtr;
typedef std::shared_ptr<INTELLI::SPSCQueue<vector<INTELLI::TuplePtr>>> WindowQueue;
#define  newTupleQueue(n) make_shared<INTELLI::SPSCQueue<INTELLI::TuplePtr>>(n)
#define  newWindowQueue(n) make_shared<INTELLI::SPSCQueue<WindowOfTuples>>(n)
typedef enum {
  CNT_BASED = 1,
  TIME_STAMP_BASED = 2,
} join_type_t;
typedef enum {
  CMD_ACK = 1,
  CMD_STOP = 2,
  //S is a window, R is a Tuple
  CMD_NEXT_WSTR,
  CMD_NEXT_WSTS,
  CMD_NEXT_TSWR,
  CMD_NEXT_TSTR,
  CMD_NEXT_TS_ONLY,
  CMD_NEXT_TR_ONLY,
} join_cmd_t;
typedef std::shared_ptr<INTELLI::SPSCQueue<INTELLI::join_cmd_t>> CmdQueuePtr;
#define  newCmdQueue(n) make_shared<INTELLI::SPSCQueue<INTELLI::join_cmd_t>>(n)
}

#endif //INTELLISTREAM_TYPES_H
/**@}*/