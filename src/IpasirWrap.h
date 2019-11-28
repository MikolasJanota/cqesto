/*
 * File:  IpasirWrap.h
 * Author:  mikolas
 * Created on:  Wed Jul 10 16:42:31 DST 2019
 * Copyright (C) 2019, Mikolas Janota
 */
#ifndef IPASIRWRAP_H_24438
#define IPASIRWRAP_H_24438
#include"ipasir.h"
#include<iostream>
#include"minisat/core/SolverTypes.h"
#define SATSPC Minisat
namespace SATSPC {
    class MiniSatExt {
    public:
        LSet         conflict;
        MiniSatExt()
            : _nvars(1)
        {
            _s = ipasir_init();
        }
        virtual ~MiniSatExt() { ipasir_release(_s); }
        bool addClause(Minisat::vec<Minisat::Lit>& cl) {
            for (int i = 0; i < cl.size(); ++i) add(cl[i]);
            return f();
        }
        inline void bump(Var) { /* unsupported */ }
        inline void setPolarity  (Var, lbool) {  /* unsupported */ }
        inline bool addClause_(Minisat::vec<Minisat::Lit>& cl) { return addClause(cl); }
        bool addClause(Minisat::Lit p) {  add(p); return f(); }
        bool addClause(Minisat::Lit p, Minisat::Lit q) {  add(p); add(q); return f(); }
        bool addClause(Minisat::Lit p, Minisat::Lit q, Minisat::Lit r) {  add(p); add(q);  add(r); return f(); }

        inline bool solve(const vec<Lit>& assumps) {
            for (int i = 0; i < assumps.size(); ++i) {
                ipasir_assume(_s, lit2val(assumps[i]));
            }
            const auto rv = solve();
            if (!rv) {
                conflict.clear();
                for (int i = 0; i < assumps.size(); ++i) {
                    const auto val = lit2val(assumps[i]);
                    if (ipasir_failed(_s, val))
                        conflict.insert(~assumps[i]);
                }
            }
            return rv;
        }

        inline bool solve() {
            const int r = ipasir_solve(_s);
            assert(r==10 || r==20);
            if (r != 10 && r != 20) {
                std::cerr<<"Something went wrong with ipasir_solve call, retv: "<<r<<std::endl;
                exit(1);
            }
            return r==10;
        }
        int nVars() const {return _nvars;}

        inline Minisat::lbool get_model_value(Minisat::Var v) const {
            const int val = ipasir_val(_s, v);
            if (val==0) return Minisat::l_Undef;
            return val > 0 ? Minisat::l_True : Minisat::l_False;
        }

        inline bool is_ok_var(int v) { return 1 <= _nvars && v <= _nvars; }

        inline void new_variables(Var max_id) { if (_nvars < max_id) _nvars = max_id; }

        inline void new_variables(const std::vector<Var>& variables) {
            Var max_id = 0;
            for (Var v : variables)
                if (max_id < v) max_id = v;
            new_variables(max_id);
        }
    private:
        int          _nvars;
        void*        _s;
        vec<Lit>     _assumps;

        inline int lit2val(const Minisat::Lit& p) {
            return Minisat::sign(p) ? -Minisat::var(p) : Minisat::var(p);
        }

        inline void add(const Minisat::Lit& p) { ipasir_add(_s, lit2val(p)); }

        inline bool f() {
            ipasir_add(_s, 0);
            return true;
        }
};
}
#endif /* IPASIRWRAP_H_24438 */
