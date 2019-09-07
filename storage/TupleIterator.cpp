//
// Created by Shuhao Zhang on 7/9/19.
//

#include <vector>
#include "TupleIterator.h"

TupleIterator::TupleIterator(TupleDescription desc, std::vector<Tuple> vector, TupleDescription td) : td(td) {


}

void TupleIterator::open() {
    DbIterator::open();
}
