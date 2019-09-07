//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_JOINPREDICATE_H
#define ALLIANCEDB_JOINPREDICATE_H


class JoinPredicate {

public:
    JoinPredicate(int field1, int op_type, int field2);

public:
    int _field1;
    int _field2;
    int _op_type;
};


#endif //ALLIANCEDB_JOINPREDICATE_H
