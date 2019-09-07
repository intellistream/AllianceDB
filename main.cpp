#include <iostream>
#include "Utility.h"
#include "joins/JoinPredicate.h"
#include "joins/Predicate.h"
#include "joins/SymmetricHashJoin.h"
#include "storage/TupleDescription.h"

int *data1;
int *data2;
int *data3;
TupleIterator *R;
TupleIterator *S;
TupleIterator *Expected_Rt;

void clean_up() {

    delete[] data1;
    delete[] data2;
    delete[] data3;
}

void createTupleLists() {


    data1 = new int[8]{1, 2, // 4 * 2
                       3, 4,
                       5, 6,
                       7, 8};

    data2 = new int[15]{1, 2, 3, //5 * 3
                        2, 3, 4,
                        3, 4, 5,
                        4, 5, 6,
                        5, 6, 7};

    data3 = new int[15]{1, 2, 1, 2, 3,//3 * 5
                        3, 4, 3, 4, 5,
                        5, 6, 5, 6, 7};

    R = Utility::createTupleList(2, data1
    );

    S = Utility::createTupleList(3, data2
    );

    Expected_Rt = Utility::createTupleList(2 + 3, data3
    );
}


void symHashJoin() {
//    JoinPredicate pred = new JoinPredicate(0, Predicate.Op.EQUALS, 0);
//    SymmetricHashJoin op = new SymmetricHashJoin(pred, scan1, scan2);
//    op.open();
//    Tuple a = op.fetchNext();
//
//    symHashJoin.open();
//    TestUtil.matchAllTuples(symHashJoin, op);

    JoinPredicate pred(0, 0, 0);

    SymmetricHashJoin op(pred, R, S, TupleDescription::merge(R->getTupleDesc(), S->getTupleDesc()),
                         JoinPredicate(0, 0, 0));


}

int main() {
    std::cout << "Welcome to AllianceDB!" << std::endl;

    std::cout << "Generates input relations for testing purpose.." << std::endl;

    createTupleLists();

    symHashJoin();


    return 0;
}
