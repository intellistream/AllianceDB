//
// Created by Shuhao Zhang on 7/9/19.
//

#include "JoinPredicate.h"
#include "Predicate.h"

JoinPredicate::JoinPredicate(int field1, int op_type, int field2) {
    _field1 = field1;
    _field2 = field2;
    _op_type = op_type;
}
