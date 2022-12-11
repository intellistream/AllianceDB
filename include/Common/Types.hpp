#ifndef ALIANCEDB_INCLUDE_COMMON_TYPES_HPP_
#define ALIANCEDB_INCLUDE_COMMON_TYPES_HPP_

#ifndef UNUSED
#define UNUSED(x) (void)(x)
#endif

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

// Constants
#ifndef WINDOW_SIZE
#define WINDOW_SIZE 500
#endif

#ifndef THREAD_NUMBER
#define THREAD_NUMBER 2
#endif

#ifndef TIME_STEP
#define TIME_STEP 40 // US
#endif

#include <barrier>
#include <cstdint>
#include <iomanip>
#include <iostream>
#include <memory>
#include <mutex>
#include <queue>
#include <unordered_map>
#include <vector>

#ifndef BAR
#define BAR "-------------------------------------------"
#endif

using TsType = size_t;      /*!< Type of the timestamp, default uint64_t */
using KeyType = uint64_t;   /*!< Type of the join key, default uint64_t */
using ValType = uint64_t; /*!< Type of the val, default uint64_t */
using uint32 = uint32_t;    /*!< Type of the uint32, default uint32_t */
enum StreamType { R, S };   /*!< Type of the stream, default R and S */

#endif