#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
#include <Common/Types.hpp>
#include <Common/Tuple.hpp>

namespace INTELLI {
typedef std::shared_ptr<class Relations> RelationsPtr;
/**
 * @typedef Relation
 * @brief A vector of TuplePtr
 * @warning This is not thread-safe, only used for local data
 */
typedef std::vector<TuplePtr> Relation;

class Relations {
 public:
  Relation relationR;
  Relation relationS;
  static RelationsPtr create();
};
}

#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
