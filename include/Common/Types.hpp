#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef _INTELLISTREAM_TYPES_H
#define _INTELLISTREAM_TYPES_H

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
#define TIME_STEP 40// US
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
#include <barrier>
#include <Utils/concurrentqueue.h>
#include <Utils/DupicatedHashTable.hpp>
#include <Utils/SPSCQueue.hpp>
#include <iostream>
#include <iomanip>

#ifndef BAR
#define BAR "-------------------------------------------"
#endif

/**
 * @mainpage Introduction
 * The AllianceDB offers a wide range of stream join algorithms, supporting both inter and
 * intra window join.
 * @section sec_mj Major Parts
 * Three major parts are composed to achieve ultra-fast stream window join, and they can be found in corresponding named
 * folder under include or src
 *
 * @subsection JoinAlgo
 * Here are specific algorithms to eventually deal with stream join, including hash or radix.
 * All of these algos assume there is no window, in another word,
 * they achieve the intra-window join.
 * Please refer to the @ref INTELLI_JOINALGOS module.
 *
 * @subsection JoinProcessor
 * Here is the middle layer of the whole stream window join, i.e., to bridge the "window" and "join".
 * One JoinProcessor may either eagerly do stream join,
 * or just accumulate the tuples and evoke JoinAlgo  for lazy join. JoinProcessors are managed by upper windowslider.
 * Please refer to the @ref JOINPROCESSOR module.
 *
 * @subsection WindowSlider
 * Here is the top layer on all. Typically, the WindowSliders will:
 * \li Directly receive user input by feedTupleS/R
 * \li Globally manage the sliding window
 * \li Pass tuple to and control its JoinProcessors.
 *
 * Please find them in @ref WINDOWSLIDER module
 * @section sec_other Other Parts
 * Besides the 3 above, Aliance DB has other parts to support its work, they are:
 * @subsection subsec_common Common
 * Common functions, data types for all aliance db component, and they are especially designed for aliance db.
 * Please refer to @ref Common module.
 * @subsection subsec_utils Utils
 * Similar to @ref subsec_common, but they are not specialized for aliance db. On the contrary, they may be shared by other InteliStream
 * programs or third-party programs. Please refer to @ref INTELLI_UTIL module.
 */

namespace INTELLI {
/**
 * @defgroup Common Common Datastructure and Functions
 *  @{
 */
/**
* @defgroup INTELLI_COMMON_BASIC Basic definitions
* @{
* We define the classes of Tuple, window, queue, etc. here
*/

//Alias
typedef uint64_t keyType;    /*!< Type of the join key, default uint64_t */
typedef uint64_t valueType;  /*!< Type of the payload, default uint64_t */
typedef int numberType; //for counting the number of datagram (eg: tuples) in a struct
typedef std::mutex mutex;
typedef INTELLI::DupicatedHashTable<keyType, keyType> hashtable; /*!< allow key duplication */
typedef std::queue<numberType> tupleKeyQueue;
typedef std::shared_ptr<std::barrier<>> BarrierPtr;

/**
 * @cite newTuplePtrQueue
 * @brief To create a new TuplePtrQueue
 * @param n The length of queue
 */
#define  newTuplePtrQueue(n) make_shared<INTELLI::SPSCQueue<INTELLI::TuplePtr>>(n)
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
/**
 * @}
 */
#endif //INTELLISTREAM_TYPES_H
/**@}*/