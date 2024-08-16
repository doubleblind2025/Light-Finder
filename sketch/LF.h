#ifndef BENCH_LF_H
#define BENCH_LF_H

#include "bitset.h"
#include "Abstract.h"

template<typename DATA_TYPE,typename COUNT_TYPE>
class LF : public Abstract<DATA_TYPE, COUNT_TYPE>{
public:

    LF(uint32_t _hash_num, uint32_t _length):
            hash_num(_hash_num), length(_length){
        counters = new int16_t * [hash_num];
        bitsets = new BitSet* [hash_num];
        for(uint32_t i = 0;i < hash_num;++i){
            counters[i] = new int16_t [length];
            bitsets[i] = new BitSet(length);
            memset(counters[i], 0, length * sizeof(int16_t));
        }
    }

    ~LF(){
        for(uint32_t i = 0;i < hash_num;++i){
            delete [] counters[i];
            delete bitsets[i];
        }
        delete [] counters;
        delete [] bitsets;
    }

    void Insert(const DATA_TYPE item, const COUNT_TYPE window){
        int min = INT32_MAX;
        int j = -1;
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            if(!bitsets[i]->Get(pos)){
                if(counters[i][pos] < min){
                    min = counters[i][pos];
                    j = i;
                }
            }
        }
        if(j != -1){
            for(int i = 0; i < hash_num; i++){
                uint32_t pos = this->hash(item, i) % length;
                if(counters[i][pos] == min){
                    counters[i][pos] += (!bitsets[i]->SetNGet(pos));
                }
            }
        }
    }

    COUNT_TYPE Query(const DATA_TYPE item){
        int ret = INT32_MAX;
        for(uint32_t i = 0;i < hash_num;++i){
            uint32_t pos = this->hash(item, i) % length;
            ret = MIN(ret, counters[i][pos]);
        }
        return ret;
    }

    std::string getName(){
        return "LF";
    }

    void NewWindow(const COUNT_TYPE window){
        for(uint32_t i = 0;i < hash_num;++i){
            bitsets[i]->Clear();
        }
    }

    uint32_t getLength(){
        return length;
    }

private:
    const uint32_t hash_num;
    const uint32_t length;

    BitSet** bitsets;
    //COUNT_TYPE** counters;
    int16_t ** counters;
};

#endif //BENCH_LF_H
