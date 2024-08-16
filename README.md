
Light Finder sketch
============

Introduction
--------
The source codes of LightFinder and other related algorithms.

Repository structure
--------------------
*  `common/`: the hash function and bitset data structure used by many algorithms
*  `sketch/`: the implementation of algorithms in our experiments
*  `benchmark.h`: C++ header of some benchmarks about AAE, F1 Score, and insert throughput

Requirements
-------
- cmake
- g++

How to run
-------

```bash
$ cmake .
$ make
$ ./bench your-dataset
```
