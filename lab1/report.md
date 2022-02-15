## Optimisations report

- input params  : 10000 10000 100
- time measuring: average of 10 samples  
on i7-4712MQ  
with running docker, containerd, postgres daemons (лениво отключать было)

### Times
- reference     :  1010 s
- reference + 01:   116 s
- reference + O2: 112.5 s
- k precount    :    49 s
- betterprecount:    46 s
- grouping      :  48.5 s
- array decoup  :  50.5 s
- row decoup    : scrapped
- index dedup   :    46 s
- heat precount :    35 s
- inc index     :    33 s
- march=native  :  27.5 s
- affinity      : 33.5 vs 33.52 s*

## Optimisations

### 1. k precount:

Предвычислил 4 коэффициента при слагаемых при инициализации, поскольку они не меняются.

### 2. better precount:

Исключил остальные умножения в коэффициентах.

### 3. Memory access grouping

Группировка запросов к данным на одной строке матрицы;  
*неудачные +2.5s*

### 4. Array cycle decoupling

1 цикл -> 2, по heat_sources и по current_model - локализация запросов  
*еще +2s, откат к 2*

### Row cycle decoupling

1 цикл -> 3, строчка выше, эта, ниже - дополнительная локализация  
*идея отменена в силу невыгодности на основании предыдущих*

### 5. Index deduplication

Индекс вычисляется один раз за итерацию, вместо 14 в инлайн-варианте  
*никакой разницы с 3, откат с целью читаемости*

### 6. heat pre-count

Предподсчет хвоста от источников, поскольку тот статичен.  
*Спорно, базовая ли это оптимизация*,   
К тому же требует в самом начале еще одного массива на короткое время,  
но можем воспользоваться current_model для инициализации и затем подменить указатели аналогично `JModel::switch_models()`.

### 7. Incremental indexes

Вместе с оптимизацией 5., вместо перевычисления индекса каждую итерацию с умножением - только суммирование.

### 8. march=native

Совсем забыл про оптимизацию под архитектуру, к сожалению - выигрыш заметен.

### 9. core affinity

Фиксируем core affinity на, например, первое ядро.
*Похоже, линковка lpthread нивелирует весь выигрыш от предыдущей оптимизации;*  
*Однако, если сравнивать с 7 версией при том же Makefile,*  
есть скромный выигрыш в 2 сотых секунды на среднем от выборки из 20*  

## Profiling

### Call graph
```
    $ make jprof
    $ ../scripts/20times.sh
    $ make jcalls
```
![call graph](gprof-profile/callgraph.png?raw=true "Call graph")

### default gcc (O0) flags call graph
```
    $ make jprof0
    $ ../build/lab1pg0 10000 10000 20
    $ make jcalls0
```

![call graph](gprof-profile-o0/callgraph.png?raw=true "Call graph")
![lh call graph](gprof-profile-o0/left-callgraph.png?raw=true "Left half Call graph")

// TODO:
- perf profiling and 2 listings
- roofline model