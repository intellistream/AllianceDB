//
// Created by Shuhao Zhang on 7/9/19.
//

#include "SymmetricHashJoin.h"

void SymmetricHashJoin::symHashJoin(TupleIterator R, TupleIterator S) {


}

SymmetricHashJoin::SymmetricHashJoin(JoinPredicate predicate, TupleIterator *R, TupleIterator *S)
        : _predicate(_predicate), _R(R), _S(S) {
    inner = R;
    outter = S;
    _comboTD = TupleDesc::merge(R->getTupleDesc(), S->getTupleDesc());
}

void SymmetricHashJoin::open() {
    _R->open();
    _S->open();
    Operator::open();//super.open()
}
