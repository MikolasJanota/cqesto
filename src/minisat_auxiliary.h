/*
 * File:   minisat_aux.hh
 * Author: mikolas
 *
 * Created on October 21, 2010, 2:10 PM
 */
#pragma once
#include "auxiliary.h"
#include "minisat_ext.h"
#include <ostream>
using SATSPC::lbool;
using SATSPC::Lit;
using SATSPC::mkLit;
using SATSPC::sign;
using SATSPC::var;
using SATSPC::Var;
using std::ostream;
using std::vector;
namespace SATSPC {
ostream &print_model(ostream &out, const SATSPC::vec<lbool> &lv);
ostream &print_model(ostream &out, const SATSPC::vec<lbool> &lv, int l, int r);
ostream &print(ostream &out, const SATSPC::vec<Lit> &lv);
ostream &print(ostream &out, const vector<Lit> &lv);
ostream &operator<<(ostream &outs, Lit lit);
ostream &operator<<(ostream &outs, lbool lb);

inline ostream &operator<<(ostream &outs, const SATSPC::vec<Lit> &lv) {
    return print(outs, lv);
}

inline Lit to_lit(Var v, lbool val) {
    return val == l_True ? mkLit(v) : ~mkLit(v);
}

inline Lit to_lit(const SATSPC::vec<lbool> &bv, Var v) {
    return (v < bv.size()) && (bv[v] == l_True) ? mkLit(v) : ~mkLit(v);
}

inline lbool eval(Lit l, const SATSPC::vec<lbool> &vals) {
    const Var v = var(l);
    if (v >= vals.size())
        return l_Undef;
    const auto vv = vals[v];
    if (vv == l_Undef)
        return l_Undef;
    return (vv == l_False) == (sign(l)) ? l_True : l_False;
}

inline void to_lits(const SATSPC::vec<lbool> &bv, SATSPC::vec<Lit> &output, int s,
                    const int e) {
    for (int index = s; index <= e; ++index) {
        if (bv[index] == l_True)
            output.push(mkLit((Var)index));
        else if (bv[index] == l_False)
            output.push(~mkLit((Var)index));
    }
}

inline std::ostream &operator<<(std::ostream &out, Lit l) {
    if (l == SATSPC::lit_Undef)
        return out << "lit_Undef";
    return out << (sign(l) ? "-" : "+") << var(l);
}

inline std::ostream &operator<<(std::ostream &outs, SATSPC::lbool val) {
    if (val == l_False)
        return std::cerr << "f";
    if (val == l_True)
        return std::cerr << "t";
    if (val == l_Undef)
        return std::cerr << "u";
    assert(0);
    return outs << "ERROR";
}

inline SATSPC::lbool neg(SATSPC::lbool v) {
    if (v == l_False)
        return l_True;
    if (v == l_True)
        return l_False;
    assert(v == l_Undef);
    return l_Undef;
}

inline lbool eval_(Lit literal, const SATSPC::vec<lbool> &vals) {
    const auto variable = var(literal);
    assert(variable < vals.size());
    assert(0 <= variable);
    return sign(literal) ? neg(vals[variable]) : vals[variable];
}

// representative -> AND l_i
inline void encode_and_pos(SATSPC::MiniSatExt &solver, Lit representative,
                           const vector<Lit> &rhs) {
    for (size_t i = 0; i < rhs.size(); ++i)
        solver.addClause(~representative, rhs[i]);
}

class Lit_equal {
  public:
    inline bool operator()(const Lit &l1, const Lit &l2) const {
        return l1 == l2;
    }
};

class Lit_hash {
  public:
    inline size_t operator()(const Lit &l) const { return SATSPC::toInt(l); }
};

inline size_t literal_index(Lit l) {
    assert(var(l) > 0);
    const size_t v = (size_t)var(l);
    return sign(l) ? v << 1 : ((v << 1) + 1);
}

inline Lit index2literal(size_t l) {
    const bool positive = (l & 1);
    const Var variable = l >> 1;
    return positive ? mkLit(variable) : ~mkLit(variable);
}
} // namespace SATSPC
