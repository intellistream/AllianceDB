#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_

#include "Common/Types.hpp"

namespace AllianceDB
{
typedef std::shared_ptr<class Tuple> TuplePtr;
/**
 * @class Tuple Common/Types.h
 * @brief The class to describe a tuple
 */
class Tuple
{
public:
    TsType ts = 0; /*!< timestamp is preserved for join system, e.g., it can be the time stamp or
                      tuple count*/
    StreamType st; /*!< whether it is from stream R =true  or S = false*/
    KeyType key;   /*!< The key used for relational join*/
    ValType val;   /*!< The val, can also be pointer*/
    std::string toString();
    Tuple(KeyType k);
    Tuple(KeyType k, ValType v);
    Tuple(KeyType k, ValType v, StreamType st, TsType ts);
    ~Tuple();
};
}  // namespace AllianceDB
#endif  // ALIANCEDB_SRC_COMMON_TYPES_CPP_TUPLE_HPP_
