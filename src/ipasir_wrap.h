/*
 * File:  ipasir_wrap.h
 * Author:  mikolas
 * Created on:  Wed Jul 10 16:42:31 DST 2019
 * Copyright (C) 2019, Mikolas Janota
 */
#pragma once
#include "ipasir.h"
#include "minisat/core/SolverTypes.h"
#include <cassert>
#include <cstdlib>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>
namespace SATSPC {
std::ostream &operator<<(std::ostream &outs, Lit lit);
class IPASIRWrap {
  public:
    inline const Minisat::LSet &get_conflict() { return _conflict; }
    inline const Minisat::vec<Minisat::lbool> &model() { return _model; }
    IPASIRWrap() : _nvars(0) { _s = ipasir_init(); }

    virtual ~IPASIRWrap() { ipasir_release(_s); }

    inline void setFrozen(Var, bool) {}
    inline void bump(Var) {}
    inline void releaseVar(Lit l) { addClause(l); }
    inline bool simplify() { return true; }

    bool addClause(Minisat::vec<Minisat::Lit> &cl) {
        for (int i = 0; i < cl.size(); ++i)
            add(cl[i]);
        return f();
    }

    inline bool addClause_(Minisat::vec<Minisat::Lit> &cl) {
        return addClause(cl);
    }

    inline bool addClause(const std::vector<Minisat::Lit> &cl) {
        for (const auto &l : cl)
            add(l);
        return f();
    }

    bool addClause(Minisat::Lit p) {
        add(p);
        return f();
    }

    bool addClause(Minisat::Lit p, Minisat::Lit q) {
        add(p);
        add(q);
        return f();
    }

    bool addClause(Minisat::Lit p, Minisat::Lit q, Minisat::Lit r) {
        add(p);
        add(q);
        add(r);
        return f();
    }

    bool addClause(Minisat::Lit p, Minisat::Lit q, Minisat::Lit r,
                   Minisat::Lit s) {
        add(p);
        add(q);
        add(r);
        add(s);
        return f();
    }

    int nVars() const { return _nvars; }

    void new_variables(Var v) {
        if (_nvars < v)
            _nvars = v + 1;
    }

    inline Minisat::lbool get_model_value(Minisat::Var v) const {
        return v < _model.size() ? _model[v] : l_Undef;
    }

    bool solve(const Minisat::vec<Minisat::Lit> &assumps);
    bool solve();

  private:
    /* const int           _verb = 1; */
    int _nvars;
    void *_s;
    Minisat::vec<Minisat::Lit> _assumps;
    Minisat::LSet _conflict;
    Minisat::vec<Minisat::lbool> _model;
    inline int lit2val(const Minisat::Lit &p) {
        return Minisat::sign(p) ? -Minisat::var(p) : Minisat::var(p);
    }

    inline void add(const Minisat::Lit &p) { ipasir_add(_s, lit2val(p)); }

    inline bool f() {
        ipasir_add(_s, 0);
        return true;
    }
};

inline bool IPASIRWrap::solve(const Minisat::vec<Minisat::Lit> &assumps) {
    for (int i = 0; i < assumps.size(); ++i) {
        ipasir_assume(_s, lit2val(assumps[i]));
    }
    const auto rv = solve();
    if (!rv) {
        _conflict.clear();
        for (int i = 0; i < assumps.size(); ++i) {
            const auto val = lit2val(assumps[i]);
            if (ipasir_failed(_s, val))
                _conflict.insert(~assumps[i]);
        }
    }
    return rv;
}

inline bool IPASIRWrap::solve() {
    const int r = ipasir_solve(_s);
    assert(r == 10 || r == 20);
    if (r != 10 && r != 20) {
        std::cerr << "Something went wrong with ipasir_solve call, retv: " << r
                  << std::endl;
        exit(1);
    }
    _model.clear();
    if (r == 10) {
        _model.growTo(_nvars + 1, Minisat::l_Undef);
        for (int v = _nvars; v; v--) {
            const int vval = ipasir_val(_s, v);
            _model[v] = (vval == 0)
                            ? Minisat::l_Undef
                            : (vval < 0 ? Minisat::l_False : Minisat::l_True);
        }
    }
    return r == 10;
}
} // namespace SATSPC
