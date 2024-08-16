#ifndef OO_PE_H
#define OO_PE_H

/*
 * On-Off sketch on persistence estimation
 */

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class OO_PE : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    OO_PE(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        counters = new COUNT_TYPE* [hash_num];
        //counters = new int16_t * [hash_num];
        bitsets = new BitSet* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counters[i] = new COUNT_TYPE [length];
            //counters[i] = new int16_t [length];
            bitsets[i] = new BitSet(length);
            memset(counters[i], 0, length * sizeof(COUNT_TYPE));
            //memset(counters[i], 0, length * sizeof(int16_t));
        }
    }

    ~OO_PE(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] counters[i];
            delete bitsets[i];
        }
        delete [] counters;
        delete [] bitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            counters[i][pos] += (!bitsets[i]->SetNGet(pos));
        }
    }

    bool appearInCurWindow(DATA_TYPE item){
        for(int i = 0; i < hash_num; i++){
            uint32_t pos = this->hash(item, i) % length;
            if(!bitsets[i]->Get(pos)) return false;
        }
        return true;
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        int ret;
        if(sizeof(DATA_TYPE) == sizeof(int8_t)){
            ret = INT8_MAX;
        }else if(sizeof(DATA_TYPE) == sizeof(int16_t)){
            ret = INT16_MAX;
        }else{
            ret = INT32_MAX;
        }
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            ret = MIN(ret, counters[i][pos]);
        }
        return ret;
    }

    std::string getName(){
        return "OO";
    }

    void NewWindow(const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
        }
    }

    uint32_t getLength(){
        return length;
    }

    void reset(){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
            memset(counters[i], 0, length * sizeof(COUNT_TYPE));
        }
    }

private:
    const uint32_t hash_num;
    const uint32_t length;

    BitSet** bitsets;
    COUNT_TYPE** counters;
    //int16_t ** counters;
};

#endif //OO_PE_H
