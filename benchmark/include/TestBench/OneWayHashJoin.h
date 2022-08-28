//
// Created by Wang Chenyu on 4/9/21.
//

#ifndef AllianceDB_ONEWAYHASHJOIN_H
#define AllianceDB_ONEWAYHASHJOIN_H

#include <Common/Types.hpp>
namespace AllianceDB {
class OneWayHashJoin {
 public:
  static void execute(Result &joinResult, RelationCouple &relationCouple);
};
}
#endif //INTELLISTREAM_ONEWAYHASHJOIN_H
