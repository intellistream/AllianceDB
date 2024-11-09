#pragma once

#include "../utils/types.h"
class Batch{
    int size_;
    int current_loc_;
    constexpr static int default_size=1024;


public:
    intkey_t* keys_;
    value_t* values_;

    Batch(int size=default_size):size_(size),current_loc_(0){
        keys_= malloc(size*sizeof(intkey_t));
        values_ =malloc(size*sizeof(current_loc_));
    }
    Batch(const Batch& batch)= delete;
    Batch(Batch&& batch){
        keys_=batch.keys_;
        values_=batch.values_;
        current_loc_=batch.current_loc_;
        size_=batch.size_;
        batch.keys_= nullptr;
        batch.values_= nullptr;
    }
    inline bool add_tuple(const tuple_t* t){
        if(t== nullptr){
            return false;
        }
        keys_[current_loc_]=t->key;
        values_[current_loc_]=t->payloadID;
        current_loc_++;
        return current_loc_==size_;
    }

    inline int reset(){
        current_loc_=0;
    }
    ~Batch(){
        free(keys_);
        free(values_);
    }

};


class BatchCreator {
    Batch current_batch;

public:
    BatchCreator(){

    }

    bool add_tuple(const tuple_t* tuple){

    }

};


