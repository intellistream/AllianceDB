#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
#include <Common/Types.hpp>
#include <Common/Tuple.hpp>

namespace INTELLI {
typedef std::shared_ptr<class Relations> RelationsPtr;

class Relations {
 public:
  Relation relationS;
  Relation relationR;
  static std::shared_ptr<Relations> create();
};
}

#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
