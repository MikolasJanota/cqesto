# cqesto
A circuit-based QBF solver. The underlying algorithm is explained in [1].  It
is a continuation of the  clause-selection algorithm [2].

# Building
    `cd src && make`

Somethings are configurable upon build. Please see the `Makefile`.

##  Back-end SAT solver

`cqesto`  comes with `minisat`  prepackaged.  However, you may choose to use cadical  (communicating through the ipasir interface).
To use cadical: `cd src/cadical && ./setmeup.sh && cd - && make USE_IPASIR=1 clean all`

#  REFERENCES

[1] Circuit-based Search Space Pruning in QBF, Mikoláš Janota in SAT '18
[2] Solving QBF by Clause Selection, Mikoláš Janota, Joao Marques-Silva, in IJCAI '15
