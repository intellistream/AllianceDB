/*! \file StateOfKey.h*/
//
// Created by tony on 02/12/22.
//

#ifndef INTELLISTREAM_STATEOFKEY_H
#define INTELLISTREAM_STATEOFKEY_H

#include <cstdint>
#include <string>
#include <memory>
#include <Common/Tuples.h>
#include <utility>
#include <vector>
#include <mutex>

namespace OoOJoin {

/**
* @ingroup INTELLI_COMMON_BASIC Basic Definitions and Data Structures
* @{
 * @defgroup INTELLI_COMMON_BASIC_STATE_OF_KEY The state of key group
 * @{
*
*/
/**
 * @class  AbstractStateOfKey Common/StateOfKey.h
 * @brief The statistics and prediction state of a key
 * @note normally inherited by more complicated states
 * @ingroup INTELLI_COMMON_BASIC_STATE_OF_KEY
 */
class AbstractStateOfKey {
 public:
  AbstractStateOfKey() = default;

  ~AbstractStateOfKey() = default;

  keyType key = 0;
};

/**
 * @typedef AbstractStateOfKeyPtr
 * @brief The class to describe a shared pointer to @ref AbstractStateOfKey
 */
typedef std::shared_ptr<class AbstractStateOfKey> AbstractStateOfKeyPtr;

class StateOfKeyBucket;

/**
* @typedef StateOfKeyBucketPtr
* @brief The shared pointer to a @ref StateOfKeyBucket
*/
typedef std::shared_ptr<class StateOfKeyBucket> StateOfKeyBucketPtr;

/**
 * @class StateOfKeyBucket Common/StateOfKey.h
 * @brief The multithread-supported bucket of @ref StateOfKey
 * @ingroup INTELLI_COMMON_BASIC_STATE_OF_KEY
 */
class StateOfKeyBucket {
 private:
  std::mutex m_mut;
  //vector<NPJTuplePtr>tuples;
  size_t bucketSize;

  /**
   * @class StateOfKeyBucket_Iterator
   * @brief The iterator class for bucket
   * @note Only support iter++ and ++iter
   */
  class StateOfKeyBucket_Iterator {
   private:
    AbstractStateOfKeyPtr cellPtr = nullptr;
    StateOfKeyBucket *bucketPtr = nullptr;
    size_t bucketPos = 0;
   public:
    explicit StateOfKeyBucket_Iterator(AbstractStateOfKeyPtr cp = nullptr, StateOfKeyBucket *bp = nullptr,
                                       size_t bkpos = 0)
        : cellPtr(std::move(cp)), bucketPtr(bp), bucketPos(bkpos) {}

    AbstractStateOfKeyPtr operator*() const {
      return cellPtr;
    }

    AbstractStateOfKeyPtr first() {
      return cellPtr;
    }

    StateOfKeyBucket *second() {
      return bucketPtr;
    }

    StateOfKeyBucket *operator->() const {
      return bucketPtr;
    }

    bool operator==(const StateOfKeyBucket_Iterator &t) const {
      return t.cellPtr == this->cellPtr;
    }

    bool operator!=(const StateOfKeyBucket_Iterator &t) const {
      return t.cellPtr != this->cellPtr;
    }

    StateOfKeyBucket_Iterator &operator++() {
      bucketPos++;
      if (bucketPos == bucketPtr->count) {
        bucketPtr = bucketPtr->next.get();
        bucketPos = 0;
      }
      if (bucketPtr) {
        cellPtr = bucketPtr->cells[bucketPos];
      } else {
        cellPtr = nullptr;
      }
      return *this;
    }
    /*StateOfKeyBucket_Iterator operator++(int) {
        AbstractStateOfKeyPtr tempCell=cellPtr;
        StateOfKeyBucket *tempBucket=bucketPtr;
        size_t tempPos=bucketPos;
        bucketPos++;
        if(bucketPos==bucketPtr->count){
            bucketPtr=bucketPtr->next.get();
            bucketPos=0;
        }
        if(bucketPtr) {
           cellPtr=bucketPtr->cells[bucketPos];
        } else {
            cellPtr= nullptr;
        }

        return StateOfKeyBucket_Iterator(tempCell,tempBucket,tempPos);
    }*/
  };

 public:
  typedef StateOfKeyBucket_Iterator iterator;
  size_t count = 0;
  std::vector<AbstractStateOfKeyPtr> cells;
  StateOfKeyBucketPtr next = nullptr;

  /**
   * @brief set how many cells are allowed in one bucket
   * @param cnt
   * @note Please set correct number for cache awareness
   */
  void setCellCount(size_t cnt) {
    cells = std::vector<AbstractStateOfKeyPtr>(cnt);
    bucketSize = cnt;
  }

  /**
   * @brief lock this bucket
   */
  void lock() {
    while (!m_mut.try_lock());
  }

  /**
   * @brief unlock this bucket
   */
  void unlock() {
    m_mut.unlock();
  }

  /**
   * @brief Insert a StateOfKey
   * @param ask The AbstractStateOfKey
   */
  void insert(AbstractStateOfKeyPtr ask);

  /**
   * @brief probe if we have something with the same key as ask
   * @param probeKey The key for probing
   * @return the stored AbstractStateOfKey, nullptr if nothing
   */
  AbstractStateOfKeyPtr getByKey(keyType probeKey);

  iterator begin() {
    return StateOfKeyBucket_Iterator(cells[0], this, 0);
  }

  //返回链表尾部指针
  iterator end() {
    return StateOfKeyBucket_Iterator(nullptr, this, 0);
  }
  /**
   * @brief get the next bucket
   * @return this->next
   */
  /*MtBucketPtr getNext(){
    return next;
  }*/
};

/**
* @class StateOfKeyHashTable Common/StateOfKey.h
* @brief The multithread-supported hash table, holding buckets of stateofkey
 * @ingroup INTELLI_COMMON_BASIC_STATE_OF_KEY
*/
class StateOfKeyHashTable {
 private:

 public:
  keyType hash_mask{};
  keyType skip_bits{};
  std::vector<StateOfKeyBucket> buckets;

  StateOfKeyHashTable() = default;

  /**
   * @brief pre-init with several buckets
   * @param bks the number of buckets
   * @param cells How many cells are allowed in one bucket
   */
  explicit StateOfKeyHashTable(size_t bks, size_t cells = 4);

  ~StateOfKeyHashTable() = default;

  /**
   * @brief Insert a StateOfKey
   * @param ask The AbstractStateOfKey
   * @note not thread safe, use insertLock instaead for thread safe
   */
  void insert(AbstractStateOfKeyPtr ask);

  /**
* @brief Insert a StateOfKey, a thread safe version
* @param ask The AbstractStateOfKey
*/
  void insertSafe(AbstractStateOfKeyPtr ask);

  /**
   * @brief probe if we have something with the same key as ask
   * @param probeKey The key for probing
   * @return the stored AbstractStateOfKey, nullptr if nothing
   */
  AbstractStateOfKeyPtr getByKey(keyType probeKey);

};

/**
 * @typedef StateOfKeyHashTablePtr
 * @brief The shared pointer to a @ref StateOfKeyHashTable
 */
typedef std::shared_ptr<StateOfKeyHashTable> StateOfKeyHashTablePtr;
/**
 * @def newStateOfKeyHashTable
 * @brief to creat a shared pointer to @ref StateOfKeyHashTable
 */
#define  newStateOfKeyHashTable std::make_shared<OoOJoin::StateOfKeyHashTable>
/**
 * @def ImproveStateOfKeyTo
 * @brief To improve the @ref AbstractStateOfKeyPtr to its child classes' shared pointer
 * @param n THe name of child class, just itself, not shared pointer
 * @param ru The shared pointer of AbstractStateOfKeyPtr to be improved
 */
#define ImproveStateOfKeyTo(n, ru) std::static_pointer_cast<n>(ru)

/**
 * @}
 */
/**
* @}
*/
}
#endif //INTELLISTREAM_STATEOFKEY_H
