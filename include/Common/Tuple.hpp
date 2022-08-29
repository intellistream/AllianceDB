#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
#include <Common/Types.hpp>
namespace AllianceDB {

typedef std::shared_ptr<class Tuple> TuplePtr;
/**
 * @class Tuple Common/Types.h
 * @brief The class to describe a tuple
 */
class Tuple {
 public:
  tsType timestamp = 0;/*!< timestamp is preserved for join system, e.g., it can be the time stamp or tuple count*/
  keyType key; /*!< The key used for relational join*/
  valueType payload; /*!< The payload, can also be pointer*/
  std::string toString();
  Tuple(keyType k);
  Tuple(keyType k, valueType v);
  Tuple(tsType t, keyType k, valueType v);
  ~Tuple();
};
}
#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
