//
// Created by Shuhao Zhang on 7/9/19.
//

#include "Tuple.h"

Tuple::Tuple(TupleDesc td) : _td(td) {

}

void Tuple::setField(int i, Field f) {

}

Field Tuple::getField(int i) {
    return fields[i];
}

TupleDesc Tuple::getTupleDesc() {
    return _td;
}
