#pragma once

#include "../utils/types.h"
// TODO memory pool
class Batch{

    int size_;
    constexpr static int default_size=4096;

    // debugging field
    int batch_cnt_;



public:
    intkey_t* keys_;
    value_t* values_;

    static inline int max_size(){
        return default_size;
    }

    explicit Batch():size_(0),batch_cnt_(0){
        allocate();
    }


    Batch(const Batch& batch) = delete;
    Batch& operator=(const Batch& batch) = delete;

    Batch(Batch&& batch){
        keys_=batch.keys_;
        values_=batch.values_;
        size_=batch.size_;
        batch.size_=0;
        batch.batch_cnt_=0;
        batch.keys_= nullptr;
        batch.values_= nullptr;
    }

    Batch& operator=(Batch&& batch){
        //todo check self assign
        keys_=batch.keys_;
        values_=batch.values_;
        size_=batch.size_;
        batch.keys_= nullptr;
        batch.values_= nullptr;
        return *this;
    }

    inline bool add_tuple(const tuple_t* t){
        if(t==nullptr){
            return false;
        }
        keys_[size_]=t->key;
        values_[size_]=t->payloadID;
        size_++;
        return size_==default_size;
    }

    inline bool add_tuple(const tuple_t& t){
        keys_[size_]=t.key;
        values_[size_]=t.payloadID;
        size_++;
        return default_size==size_;
    }

    inline bool add_tuple(key_t key, value_t value){
        keys_[size_]=key;
        values_[size_]=value;
        size_++;
        return default_size==size_;
    }

    // called after moved
    inline void allocate(){
        size_=0;
        keys_= (intkey_t*)malloc(default_size*sizeof(intkey_t));
        values_ =(value_t*)malloc(default_size*sizeof(value_t));
    }

    inline void reset(){
        size_=0;
    }

    inline int size() const{
        return size_;
    }
    inline int batch_cnt() const{
        return batch_cnt_;
    }
    ~Batch(){
        free(keys_);
        free(values_);
    }

};

