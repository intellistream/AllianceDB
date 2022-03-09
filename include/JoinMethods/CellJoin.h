//
// Created by Wang Chenyu on 13/10/21.
//


#ifndef HYBRID_JOIN_CELLJOIN_H
#define HYBRID_JOIN_CELLJOIN_H

#include <Common/Types.h>
#include <JoinMethods/AbstractJoinMethod.h>
#include <WindowSlider/AbstractEagerWS.h>
namespace INTELLI {
class CellJoin : public AbstractJoinMethod<AbstractEagerWS> {
 public:
  static void execute(Result &joinResult, RelationCouple &relationCouple);
  //static void test(Result &joinResult, RelationCouple &relationCouple);
  static void threadWork(int id, numberType windowSizeS);
};
}

#endif //HYBRID_JOIN_CELLJOIN_H
