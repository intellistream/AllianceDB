#pragma once

#include "../utils/types.h"
class Batch{
    int max_buffer_;
    int current_loc_;
    int size_;
    int batch_cnt_;
    constexpr static int default_size=4096;


public:
    intkey_t* keys_;
    value_t* values_;

    Batch(int size=default_size):size_(size),current_loc_(0),batch_cnt_(0){
        keys_= (intkey_t*)malloc(size*sizeof(intkey_t));
        values_ =(value_t*)malloc(size*sizeof(current_loc_));
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
        if(t==nullptr){
            return false;
        }
        keys_[current_loc_]=t->key;
        values_[current_loc_]=t->payloadID;
        current_loc_++;
        return current_loc_==size_;
    }

    inline void reset(){
        batch_cnt_++;
        current_loc_=0;
    }
    inline int size() const{
        return current_loc_;
    }
    inline int batch_cnt() const{
        return batch_cnt_;
    }
    ~Batch(){
        free(keys_);
        free(values_);
    }

};



