//
// Created by Wang Chenyu on 4/9/21.
//

#ifndef INTELLISTREAM_ONEWAYHASHJOIN_H
#define INTELLISTREAM_ONEWAYHASHJOIN_H

#include "Types.h"
namespace INTELLI {
class OneWayHashJoin {
 public:
  static void execute(Result &joinResult, RelationCouple &relationCouple);
};
}
#endif //INTELLISTREAM_ONEWAYHASHJOIN_H
