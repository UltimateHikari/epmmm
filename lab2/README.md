# Lab1

*2nd task, float version*  

```
    $ make -B && ./build/lab2.a 100 100 5000 ; gnuplot -c gnuplot-script-100
    $ make -B && ./build/lab2.a 100 100 5000 1; gnuplot -c gnuplot-script-100
    $ make -B && ./build/lab2.a 5000 5000 100
    $ make -B && ./scripts/10times.sh lab2.a > ./timetest
    $ make cleanup
```