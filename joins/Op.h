//
// Created by Shuhao Zhang on 7/9/19.
//

#ifndef ALLIANCEDB_OP_H
#define ALLIANCEDB_OP_H


class Op {

public:
    enum Type{
        EQUALS, GREATER_THAN, LESS_THAN, LESS_THAN_OR_EQ, GREATER_THAN_OR_EQ, LIKE, NOT_EQUALS
    };

    /**
     * Interface to access operations by integer value for command-line
     * convenience.
     *
     * @param i
     *            a valid integer Op index
     */
    static Type getOp(int i) {
        return Type(i);
    }


};


#endif //ALLIANCEDB_OP_H
