#include <iostream>
#include "Utility.h"

int *data1;
int *data2;
int *data3;

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

    TupleIterator *R = Utility::createTupleList(2,
                                                data1
    );

    TupleIterator *S = Utility::createTupleList(3,
                                                data2
    );

    TupleIterator *Expected_Rt = Utility::createTupleList(2 + 3,
                                                          data3
    );
}

int main() {
    std::cout << "Welcome to AllianceDB!" << std::endl;

    std::cout << "Generates input relations for testing purpose.." << std::endl;


    return 0;
}