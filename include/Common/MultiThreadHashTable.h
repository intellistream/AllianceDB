/*! \file MultiThreadHashTable.h*/
#ifndef _JOINALGO_NPJ_MULTITHREADHASHTABLE_H_
#define _JOINALGO_NPJ_MULTITHREADHASHTABLE_H_
#include "Types.h"
#include <stdint.h>
#include <memory>
#include <condition_variable>
#include <mutex>
namespace INTELLI{

 /**
  * @ingroup Common
  * @defgroup INTELLI_COMMON_TB The multithread hashtable
  * @{
  * The hashtable achieves thread-safe by std::mutex. Instead of locking the whole table, only the bucketed to be inserted will be locked, which
  * reduces waiting
  */
#define  BUCKET_SIZE 4
class MtBucket;
typedef std::shared_ptr<MtBucket>MtBucketPtr;
//typedef vector<TuplePtr> MtTuplePtr;
typedef TuplePtr MtTuplePtr;
/**
 * @class MtBucket Common/MultiThreadHashTable.h
 * @brief The multithread-supported bucket
 * @note In C++, package the attributes of MtBucket in functions is faster than directly address them,
 * maybe this is a more cache-aware way
 *
 */
class MtBucket{
 private:
  std::mutex m_mut;
  //vector<MtTuplePtr>tuples;
  TuplePtr tuples[BUCKET_SIZE];
  MtBucketPtr next= nullptr;
  size_t count=0;
 public:
  /*MtBucket(){
  }
  ~MtBucket(){}*/
  /**
   * @brief lock this bucket
   */
  void lock(){
    while (!m_mut.try_lock());
  }
  /**
   * @brief unlock this bucket
   */
  void unlock(){
    m_mut.unlock();
  }
  /**
   * @brief Insert a tuple, allowing key duplication
   * @param tp The tuple
   */
  void duplicatedInsert(TuplePtr tp);
  /**
   * @brief probe one tuple, just on the bucket
   */
  size_t probeTuple(TuplePtr tp);
  /**
   * @brief get the next bucket
   * @return this->next
   */
  /*MtBucketPtr getNext(){
    return next;
  }*/
};
/**
 * @class MultiThreadHashTable Common/MultiThreadHashTable.h
 * @brief The multithread-supported hash table, holding buckets
 * @todo Improve the efficiency of build phase
 */
class MultiThreadHashTable{
 private:

 public:
  uint32_t hash_mask;
  uint32_t skip_bits;
  vector<MtBucket>buckets;
  MultiThreadHashTable(){}
  /**
   * @brief pre-init with several buckets
   * @param bks the number of buckets
   */
  MultiThreadHashTable(size_t bks);
  ~MultiThreadHashTable(){}
  /**
   * @brief build the hashtable from tuple queue
   * @param tps the tuple queue
   * @note This is thread-safe
   */
  void buildTable(TuplePtrQueue tps);
  /**
 * @brief build the hashtable from tuple memory aray
 * @param tps the tuple aray
 * @param len the length
 * @note This is thread-safe
 * @warning This function with pointer is NOT intended for API, but only inline use
 */
  void buildTable(TuplePtr *tps,size_t len);
  /**
   * @brief probe one tuple
   * @param tp Tge tuple
   * @return Number of matches
   */
  size_t probeTuple(TuplePtr tp);

};
typedef std::shared_ptr<MultiThreadHashTable> MultiThreadHashTablePtr;
/**
 * @}
 */
}
#endif //ALIANCEDB_INCLUDE_JOINALGO_NPJ_MULTITHREADHASHTABLE_H_
