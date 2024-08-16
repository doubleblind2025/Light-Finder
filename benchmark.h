#ifndef BENCHMARK_H
#define BENCHMARK_H

#include <sys/stat.h>
#include <iostream>
#include <chrono>
#include <fstream>
#include <vector>

#include "CM_HT.h"
#include "CM_BF.h"
#include "OO_PE.h"
#include "LF.h"

using namespace std;

template<typename DATA_TYPE,typename COUNT_TYPE>
class BenchMark{
public:

    typedef vector<Abstract<DATA_TYPE, COUNT_TYPE>*> AbsVector;
    typedef unordered_map<DATA_TYPE, COUNT_TYPE> HashMap;

    int memory_sum;
    double tot_memory;
    int data_size = 13;
    int window_threshold = 50;

    int test_cycle = 1;

    unordered_map<string, double> insert_through;
    vector<string> packets;

    BenchMark(const char* _PATH, const COUNT_TYPE _T, const int memory):
            PATH(_PATH){
	    struct stat statbuf;
	    stat(PATH, &statbuf);
        LENGTH = _T;

        memory_sum = memory;
        tot_memory = memory;

        FILE* file = fopen(PATH, "rb");

        COUNT_TYPE number = 0;
        DATA_TYPE item;
        HashMap record;

        TOTAL = 0;
        T = 0;

        int max = 0;

        char tmp[105];
        while(fread(&tmp, data_size, 1, file) > 0){
            tmp[data_size] = '\0';
            item = string(tmp, data_size);
            packets.push_back(item);
            if(number % LENGTH == 0)
                T += 1;
            number += 1;

            if(record[item] != T){
                record[item] = T;
                mp[item] += 1;
                TOTAL += 1;
                if(mp[item] > max) max = mp[item];
            }
        }
        packet_num = number;

        fclose(file);
    }

    void SketchError(uint32_t section){
        AbsVector sketchs = {
                new OO_PE<DATA_TYPE, COUNT_TYPE>(3, memory_sum * 8 * 1024 / 3.0 / (BITSIZE + sizeof(COUNT_TYPE))),
                new LF<DATA_TYPE, COUNT_TYPE>(3, memory_sum * 8 * 1024 / 3.0 / (BITSIZE + sizeof(int16_t))),
        };

        BenchInsert(sketchs);

        for(auto sketch : sketchs){
            PECheckError(sketch);
            delete sketch;
        }
    }

    void TopKError(double alpha){
        AbsVector sketchs = {
                new LF<DATA_TYPE, COUNT_TYPE>(3, memory_sum * 8 * 1024 / 3.0 / (BITSIZE + sizeof(int16_t))),
                new OO_PE<DATA_TYPE, COUNT_TYPE>(3, memory_sum * 8 * 1024 / 3.0 / (BITSIZE + sizeof(COUNT_TYPE))),
	    };

        BenchInsert(sketchs);

        for(auto sketch : sketchs){
            FPICheckError(sketch, alpha * TOTAL);
            delete sketch;
        }
    }

private:

    double TOTAL;
    COUNT_TYPE T;
    COUNT_TYPE LENGTH;
    COUNT_TYPE packet_num;

    HashMap mp;
    const char* PATH;

    HashMap pre;

    typedef std::chrono::high_resolution_clock::time_point TP;

    inline TP now(){
        return std::chrono::high_resolution_clock::now();
    }

    inline double durationms(TP finish, TP start){
        return std::chrono::duration_cast<std::chrono::duration<double,std::ratio<1,1000000>>>(finish - start).count();
    }

    void BenchInsert(AbsVector sketches){
        COUNT_TYPE windowId = 0;

        timespec time_begin, time_end;
        for(auto sketch : sketches){
            clock_gettime(CLOCK_MONOTONIC, &time_begin);
            for(int j = 0; j < test_cycle; j++){
                for (int i = 0; i < packet_num; i++){
                    if (i % LENGTH == 0) {
                        windowId += 1;
                        sketch->NewWindow(windowId);
                    }
                    sketch->Insert(packets[i], windowId);
                }
            }
            clock_gettime(CLOCK_MONOTONIC, &time_end);
            int64_t elapsed_ns =
                    (int64_t)(time_end.tv_sec - time_begin.tv_sec) * 1000000000LL +
                    (time_end.tv_nsec - time_begin.tv_nsec);
            insert_through[sketch->getName()] = (double)1000.0 * test_cycle * packet_num / elapsed_ns;
        }
    }

    void FPICheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch, COUNT_TYPE HIT){
        double real = 0, estimate = 0, both = 0;
        double aae = 0, cr = 0, pr = 0, f1 = 0;
        double fp = 0, fn = 0;
        double tp = 0, tn = 0;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE value = sketch->Query(it->first);

            int per = value;
            if(it->second >= HIT){
                if(per >= HIT){
                    tp++;
                }else{
                    fn++;
                }
            }else{
                if(per >= HIT){
                    fp++;
                }else{
                    tn++;
                }
            }

            if(value > HIT){
                estimate += 1;
                if(it->second > HIT) {
                    both += 1;
                    aae += abs(it->second - value);
                }
            }
            if(it->second > HIT)
                real += 1;
        }

        if(both <= 0){
            std::cout << "Not Find Real Persistent" << std::endl;
        }
        else{
            aae /= both;
        }

        cr = both / real;

        if(estimate <= 0){
            std::cout << "Not Find Persistent" << std::endl;
        }
        else{
            pr = both / estimate;
        }

        if(cr == 0 && pr == 0)
            f1 = 0;
        else
            f1 = (2*cr*pr)/(cr+pr);

        double FRP = fp / (fp + tn + 0.0), FNR = fn / (fn + tp + 0.0);

	    std::cout << sketch->getName() << std::endl
            << "HIT Count: " << HIT << std::endl
		    << "AAE: " << aae << std::endl
		    << "F1: " << f1 << std::endl
            << "FRP: " << FRP << std::endl
            << "FNR: " << FNR << std::endl;
    }

    void PECheckError(Abstract<DATA_TYPE, COUNT_TYPE>* sketch){
        double aae = 0, are = 0, tp = 0, fn = 0, fp = 0, tn = 0;
        double recall = 0, f1_score, precision;

        for(auto it = mp.begin();it != mp.end();++it){
            COUNT_TYPE per = sketch->Query(it->first);
            COUNT_TYPE real = it->second;

            aae += abs(real - per);
            are += abs(real - per) / (real + 0.0);

            if(real >= window_threshold){
                if(per >= window_threshold){
                    tp++;
                }else{
                    fn++;
                }
            }else{
                if(per >= window_threshold){
                    fp++;
                }else{
                    tn++;
                }
            }
        }

        int size = mp.size();

        precision = tp / (tp + fp + 0.0);
        recall = tp / (tp + fn + 0.0);
        f1_score = 2.0 * precision * recall / (precision + recall);


        std::cout << sketch->getName() << std::endl
                    << "AAE: " << aae / size << std::endl
                    << "precision: " << precision << std::endl
                    << "recall: " << recall << std::endl
                    << "f1_score: " << f1_score << std::endl
                    << "insert_throughput: " << insert_through[sketch->getName()] << std::endl;
    }
};

#endif //BENCHMARK_H
