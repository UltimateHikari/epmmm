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
- pragma simd           : 28.65s
- Ofast                 : 10.06s

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
Внутренний цикл итерации помечен `#pragma omp simd`, время незначительно улучшилось.
Векторизации не было из отчета и ассемблера

### 1.1.3 Ofast
Векторизация по отчету произошла (нужный цикл на 203 строке), время улучшилось.

```
../lab2.cpp:163:26: optimized: loop vectorized using 32 byte vectors
../lab2.cpp:163:26: optimized:  loop versioned for vectorization because of possible aliasing
../lab2.cpp:163:26: optimized: loop vectorized using 16 byte vectors
../lab2.cpp:148:26: optimized: loop vectorized using 32 byte vectors
../lab2.cpp:148:26: optimized:  loop versioned for vectorization because of possible aliasing
../lab2.cpp:148:26: optimized: loop vectorized using 16 byte vectors
../lab2.cpp:173:22: optimized: basic block part vectorized using 32 byte vectors
../lab2.cpp:114:14: optimized: basic block part vectorized using 32 byte vectors
../lab2.cpp:132:25: optimized: basic block part vectorized using 32 byte vectors
../lab2.cpp:203:25: optimized: loop vectorized using 32 byte vectors
../lab2.cpp:203:25: optimized: loop vectorized using 16 byte vectors
/usr/include/c++/11.2.0/bits/basic_ios.h:462:2: optimized: basic block part vectorized using 32 byte vectors
/usr/include/c++/11.2.0/fstream:868:9: optimized: basic block part vectorized using 32 byte vectors
../lab2.cpp:132:25: optimized: basic block part vectorized using 32 byte vectors
../lab2.cpp:98:51: optimized: basic block part vectorized using 32 byte vectors
```

### 1.2 Intrinsics
интринсики для avx2, по девятиточечной.  

Cхема:  
         
    0:         aaaaaaaa  
    0:          bbbbbbbb  
    0:           cccccccc  
    1:         dddddddd  
    1:  00000000eeeeeeee22222222  
    1:           ffffffff  
    2:         gggggggg  
    2:          hhhhhhhh  
    2:           eeeeeeee  

на каждом шаге + 3 вектора  
храним всего 9 основных как минимум? а их 16 всего; 9+6 вроде укладываемся

как организовать отдельный shuffle на 1 бит d и f из 0,e и e,2 соответственно?

    1:         dddddddd  
    1:  00000000eeeeeeee22222222  
    1:           ffffffff  


для сборки вот эти штуки?

    rotated_right = _mm256_permutevar8x32_ps(src, _mm256_set_epi32(6,5,4,3,2,1,0,7));
    _mm256_maskload_ps (float const * mem_addr, __m256i mask)
    _mm256_blend_ps(__m256 a, __m256 b, const int imm8)

для дельты:

    _mm256_sub_ps(f,f)
     _mm256_and_pd (__m256d a, _mm_set1_epi64x (0x7F7F7F7F7F7F7F7F)), 

Задачи:
- проверить подсчет модуля дельты
- реализовать тупую векторизацию
- проверить бленд
- profit

### 1.2.1 "dumb" simd
каждый раз грузить из памяти на каждый слой 1 выровненный и 2 почти выровненных вектора

### 1.2.2 single blend
по центральной линии грузим 3 выровненных (0,e,2), из них блендим d,f

### 1.2.3 all-blend
по всем линиям

### 1.2.4 microcore [идея]

зайдествуем l1-l2 кэш, обрабатывая строку не до конца, 
т.е. сначала половину всех строк сверху вниз, потом вторую половину строк сверху вних

## 2. Анализ

### 2.0 mpipe & perf

### 2.1 roofline

## Вывод

Автоматическая векторизация сработала с Ofast скорее вопреки, поскольку меняется исходный указатель на массив - 
видимо, эвристики Ofast



