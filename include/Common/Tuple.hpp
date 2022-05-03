#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
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

namespace INTELLI {
typedef std::shared_ptr<class Tuple> TuplePtr;
typedef std::vector<TuplePtr> WindowOfTuples;

/**
 *  @typedef TuplePtrQueue
 * @brief To describe a queue of @ref TuplePtr under SPSCQueue
 * @note This one is thread-safe
 * @warning Must be inited by @ref newTuplePtrQueue before use
 */
typedef std::shared_ptr<INTELLI::SPSCQueue<INTELLI::TuplePtr>> TuplePtrQueue;
typedef std::shared_ptr<INTELLI::SPSCQueue<vector<INTELLI::TuplePtr>>> WindowQueue;

/**
 * @class Tuple Common/Types.h
 * @brief The class to describe a tuple
 */
class Tuple {
 public:
  uint64_t key; /*!< The key used for relational join*/
  uint64_t payload; /*!< The payload, can also be pointer*/
  // The subKey can be either time-stamp or arrival count, which is assigned by join system
  // We use the subKey to do tuple expiration, which covers both count-based and time-based window
  size_t subKey = 0;/*!< subkey is preserved for join system, e.g., it can be the time stamp or tuple count*/
  /**
   * @brief construct with key
   * @param k the key
   */
  Tuple(uint64_t k);
  /**
 * @brief construct with key and value
 * @param k the key
   *@param v the value of payload
 */
  Tuple(uint64_t k, uint64_t v);
  /**
 * @brief construct with key, value and subkey
 * @param k the key
   *@param v the value of payload
   * @param sk the subkey
 */
  Tuple(uint64_t k, uint64_t v, size_t sk);
  ~Tuple();
};
}
#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
