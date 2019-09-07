//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_PREDICATE_H
#define ALLIANCEDB_PREDICATE_H


#include "Op.h"
#include "../storage/Field.h"

class Predicate {

private:
    Op op;
    int field;
    Field operand;
};


#endif //ALLIANCEDB_PREDICATE_H
