#ifndef ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
#define ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
#include <Common/Types.hpp>
#include <Common/Tuple.hpp>

namespace INTELLI {
typedef std::shared_ptr<class RelationCouple> RelationCouplePtr;

class RelationCouple {
 public:
  TuplePtrQueueIn relationS;
  TuplePtrQueueIn relationR;
  RelationCouple();
  ~RelationCouple();
};
}

#endif //ALIANCEDB_SRC_COMMON_TYPES_CPP_RELATIONCOUPLE_H_
