//
// Created by Shuhao Zhang on 7/9/19.
//

#include <vector>
#include "TupleIterator.h"

TupleIterator::TupleIterator(TupleDesc desc, std::vector<Tuple> vector, TupleDesc td) : td(td) {


}

void TupleIterator::open() {
    DbIterator::open();
}
