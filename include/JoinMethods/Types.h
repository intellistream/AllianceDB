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
#include <Utils/concurrentqueue.h>
#include <Utils/DupicatedHashTable.hpp>
#include <Utils/SafeQueue.hpp>
#include <Utils/SPSCQueue.hpp>
namespace INTELLI {
//Pointers
typedef std::shared_ptr<class Tuple> TuplePtr;
typedef std::shared_ptr<class RelationCouple> RelationCouplePtr;

//Array Pointers
typedef std::vector<TuplePtr> WindowOfTuples;
//typedef std::SafeQueue<TuplePtr> tupleQueue;
typedef std::SafeQueue<TuplePtr> tupleQueue;
typedef moodycamel::ConcurrentQueue<TuplePtr> concurrentTupleQueue;

//Alias
typedef int keyType;    //key of a tuple
typedef int valueType;  //value (aka. payload) of a tuple
typedef int numberType; //for counting the number of datagram (eg: tuples) in a struct
typedef std::mutex mutex;
typedef std::DupicatedHashTable<keyType, keyType> hashtable;

typedef std::queue<numberType> tupleKeyQueue;

class Tuple {
 public:
  keyType key;
  valueType payload;
  // The subKey can be either time-stamp or arrival count, which is assigned by join system
  // We use the subKey to do tuple expiration, which covers both count-based and time-based window
  size_t subKey = 0;
  explicit Tuple(keyType k);
  explicit Tuple(keyType k, valueType v);
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
