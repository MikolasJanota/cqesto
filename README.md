# cqesto
A circuit-based QBF solver. The underlying algorithm is explained in [1].  It
is a continuation of the  clause-selection algorithm [2].

# Building

Run
     ./configure [OPTIONS] && cd build && make

See `./configure -h` for configurations.


#  REFERENCES

1. Circuit-based Search Space Pruning in QBF, Mikoláš Janota in SAT '18
2. Solving QBF by Clause Selection, Mikoláš Janota, Joao Marques-Silva, in IJCAI '15
