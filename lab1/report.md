## Optimisations report

params: 10000 10000 100

### Times
- reference     :  1010 s
- reference + 01:   116 s
- reference + O2: 112.5 s
- k precount    :    49 s
- grouping      :
- array decoup  :
- row decoup    :

## Optimisations

### k precount:

Предвычислил 4 коэффициента при слагаемых при инициализации, поскольку они не меняются

### Index deduplication

Индекс вычисляется один раз за итерацию, вместо 14 в инлайн-варианте

### Memory access grouping

Группировка запросов к данным на одной строке матрицы

### Array cycle decoupling

1 цикл -> 2, по heat_sources и по current_model - локализация запросов

### Row cycle decoupling

1 цикл -> 3, строчка выше, эта, ниже - дополнительная локализация
