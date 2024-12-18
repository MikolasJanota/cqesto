# cqesto
A circuit-based QBF solver. The underlying algorithm is explained in [1].  It
is a continuation of the clause-selection algorithm [2]. Further funcionality
was developed during [3].

# Building
Ideally, this should work out-of-the-box:
```
./configure [OPTIONS] && cd build && make
```

See `./configure -h` for configurations.

The configure script is responsible for downloading and compiling the selected SAT solver.

Remark on the cmake required version: it's at 3.24 at this point because it
lets me force static for zlib. If you don't care about static compilation,
lower versions of cmake should also work.


# REFERENCES

1. Circuit-based Search Space Pruning in QBF, Mikoláš Janota in SAT '18
2. Solving QBF by Clause Selection, Mikoláš Janota, Joao Marques-Silva, in IJCAI '15
3. Breaking Symmetries in Quantified Graph Search: A Comparative Study, M. Janota, M. Kirchweger, T. Peitl, S. Szeider in AAAI 2025

