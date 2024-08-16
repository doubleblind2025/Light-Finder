#include "benchmark.h"

int main(int argc, char *argv[]) {

    BenchMark<string ,int32_t> dataset("./data/sample.dat", 1600, 1000);
    dataset.SketchError(10);
    dataset.TopKError(0.00005);
    return 0;
}
