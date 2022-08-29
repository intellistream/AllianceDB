#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

#ifndef _AllianceDB_TYPES_H
#define _AllianceDB_TYPES_H

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
#include <Utils/DupicatedHashTable.hpp>
#include <Utils/SPSCQueue.hpp>
#include <iostream>
#include <iomanip>

#ifndef BAR
#define BAR "-------------------------------------------"
#endif

namespace AllianceDB {
typedef size_t tsType;  /*!< Type of the timestamp, default uint64_t */
typedef uint64_t keyType;    /*!< Type of the join key, default uint64_t */
typedef uint64_t valueType;  /*!< Type of the payload, default uint64_t */
typedef std::shared_ptr<std::barrier<>> BarrierPtr;
}
/**
 * @}
 */
#endif //INTELLISTREAM_TYPES_H
/**@}*/