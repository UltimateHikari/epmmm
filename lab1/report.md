## Optimisations report

params: 10000 10000 100

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

## Optimisations

### 1. k precount:

Предвычислил 4 коэффициента при слагаемых при инициализации, поскольку они не меняются

### 2. better precount:

Исключил остальные умножения в коэффициентах

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

предподсчет хвоста от источников, поскольку тот статичен.  
*Спорно, базовая ли это оптимизация*,   
к тому же требует в самом начале еще одного массива на короткое время, но можем воспользоваться current_model.
