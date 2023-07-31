/*
 * File:   minisat_ext.h
 * Author: mikolas
 *
 * Created on November 29, 2010, 5:40 PM
 */
#pragma once

#include <cstdint>
#if USE_IPASIR

#define SATSPC Minisat
#include "ipasir_wrap.h"
#define SATSOLVER SATSPC::MiniSatExt
typedef SATSPC::vec<SATSPC::Lit> SATCLS;
#define SATCLS_PUSH(c, l)       c.push(l)
#define SATCLS_CAPACITY(c, cap) c.capacity(cap)

#elif USE_CMS

#include <cryptominisat5/cryptominisat.h>

#define SATSPC CMSat
namespace CMSat {
typedef uint32_t Var;
/* constexpr const uint32_t &toInt(const Lit &l) { return l.toInt(); } */
constexpr uint32_t var(const Lit &l) { return l.var(); }
constexpr bool sign(const Lit &l) { return l.sign(); }
constexpr Lit mkLit(uint32_t var, bool is_inverted = false) {
    return Lit(var, is_inverted);
}

class CMSat1 : public SATSolver {
  public:
    bool solve(const std::vector<Lit> &assumps) {
        const auto res = SATSolver::solve(&assumps);
        if (res == l_True)
            return true;
        if (res == l_False)
            return false;
        std::cerr << " Fatal error cryptominisat failed to solve the problem.";
        exit(100);
    }
    bool addClause(Lit a, Lit b) {
        assert(aux2.size() == 2);
        aux2[0] = a;
        aux2[1] = b;
        return add_clause(aux2);
    }
    bool addClause(Lit l) {
        assert(aux1.size() == 1);
        aux1[0] = l;
        return add_clause(aux1);
    }
    bool addClause(const std::vector<Lit> &lits) { return add_clause(lits); }
    bool addClause_(std::vector<Lit> lits) { return add_clause(lits); }
    inline void new_variables(Var max_var) {
        if (nVars() > max_var)
            return;
        new_vars(max_var + 1 - nVars());
    }
    inline lbool get_model_value(Var v) const {
        const auto &model = get_model();
        return v < model.size() ? model[v] : l_Undef;
    }

  private:
    std::vector<Lit> aux2 = {lit_Error, lit_Error};
    std::vector<Lit> aux1 = {lit_Error};
};
} // namespace CMSat

#define SATSOLVER               SATSPC::CMSat1
typedef std::vector<SATSPC::Lit> SATCLS;
#define SATCLS_PUSH(c, l)       c.push_back(l)
#define SATCLS_CAPACITY(c, cap) c.reserve(cap)

#else

#define SATSPC Minisat
#include "minisat_ext_minisat.h"

#define SATSOLVER               SATSPC::MiniSatExt
typedef SATSPC::vec<SATSPC::Lit> SATCLS;
#define SATCLS_PUSH(c, l)       c.push(l)
#define SATCLS_CAPACITY(c, cap) c.capacity(cap)

#endif
