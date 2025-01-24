/*
 * File:  IpasirWrap.h
 * Author:  mikolas
 * Created on:  Wed Jul 10 16:42:31 DST 2019
 * Copyright (C) 2019, Mikolas Janota
 */
#ifndef SMSWRAP_H_24438
#define SMSWRAP_H_24438
//#include"ipasir.h"
#include<iostream>
#include"minisat/core/SolverTypes.h"
#include "solveCadicalClass.hpp"
#define SATSPC Minisat
namespace SATSPC {
    class MiniSatExt {
    public:
        CadicalSolver sms; // SMS class
        LSet         conflict;
        MiniSatExt(configSolver config)
            : sms(config), _nvars(1)
        {
        }
        //virtual ~MiniSatExt() { ~sms(); }
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
            std::vector<int> assumps_int(assumps.size());
            for (int i = 0; i < assumps.size(); ++i) {
                assumps_int[i] = lit2val(assumps[i]);
                //ipasir_assume(sms.solver, lit2val(assumps[i]));
            }
            const auto rv = sms.solve(assumps_int);
            if (!rv) {
                conflict.clear();
                for (int i = 0; i < assumps.size(); ++i) {
                    const auto val = lit2val(assumps[i]);
                    if (sms.solver->failed(val))
                        conflict.insert(~assumps[i]);
                }
            }
            return rv;
        }

        int nVars() const {return _nvars;}

        inline Minisat::lbool get_model_value(Minisat::Var v) const {
            const int val = sms.solver->val(v);
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
        vec<Lit>     _assumps;

        inline int lit2val(const Minisat::Lit& p) {
            return Minisat::sign(p) ? -Minisat::var(p) : Minisat::var(p);
        }

        inline void add(const Minisat::Lit& p) { sms.solver->add(lit2val(p)); }

        inline bool f() {
            sms.solver->add(0);
            return true;
        }
};
}
#endif /* SMSWRAP_H_24438 */
