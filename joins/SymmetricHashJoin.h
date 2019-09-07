//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_SYMMETRICHASHJOIN_H
#define ALLIANCEDB_SYMMETRICHASHJOIN_H


#include "../storage/TupleIterator.h"

class SymmetricHashJoin {


public:
    void symHashJoin(TupleIterator R, TupleIterator S);

};


#endif //ALLIANCEDB_SYMMETRICHASHJOIN_H
