# Optimizations report 2

## Задачи

1. Векторизовать программу из лабораторной 1:
    1. с помощью компилятора: ключи, прагмы, openmp
    2. с помощью Intel vector intrinsics
2. Проанализировать производительность лучшего варианта аналогично лабораторной 1
    1. Построить roofline 
    2. Сравнить производительность

## 0. Environment

- task: `Jacobi method` using `float`s
- input params  : 10000 10000 100
- time measuring: least of 10 samples  
- CPU: i7-4712MQ  
- Compiler: GNU GCC 11.2.0
- other load on cpu: docker, containerd, postgres daemons

каждой следующей оптимизации соответствует отдельный коммит в репозитории - для удобства сравнения

## 1. Optimisation times

- reference (лаб.1)     : 27.30s
- flags                 : 28.74s

## 1. Optimisation descriptions

### 1.0 Reference:
Отчет по -fopt-info-vec-all:
векторизаций не производилось

### 1.1.1 Flags:
Флаги O2-> O3, -ftree-vectorize, -mavx2.  
Отчет по -fopt-info-vec-all:  
    ../lab2.cpp:162:26: optimized: loop vectorized using 32 byte vectors
    ../lab2.cpp:162:26: optimized:  loop versioned for vectorization because of possible aliasing
    ../lab2.cpp:162:26: optimized: loop vectorized using 16 byte vectors
    ../lab2.cpp:172:22: optimized: basic block part vectorized using 32 byte vectors
    ../lab2.cpp:113:14: optimized: basic block part vectorized using 32 byte vectors
    ../lab2.cpp:131:25: optimized: basic block part vectorized using 32 byte vectors
    /usr/include/c++/11.2.0/bits/basic_ios.h:462:2: optimized: basic block part vectorized using 32 byte vectors
    /usr/include/c++/11.2.0/fstream:868:9: optimized: basic block part vectorized using 32 byte vectors
    ../lab2.cpp:131:25: optimized: basic block part vectorized using 32 byte vectors
    ../lab2.cpp:97:51: optimized: basic block part vectorized using 32 byte vectors

### 1.1.2 Pragmas & OMP
Векторизация не применялась, т.к. основной цикл был уже автовекторизован, и время ухудшилось.

### 1.2 Intrinsics
интринсики для avx2, по схеме креста.

## 2. Анализ

### 2.0 mpipe & perf

### 2.1 roofline

## Вывод



