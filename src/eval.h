/*
 * File:   eval.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 14:22
 */

#pragma once
#include "expressions.h"
#include "sat_interface.h"
#include "visitor.h"
#include <unordered_map>
namespace qesto {
class Eval : MemoizedExpressionVisitor<SATSPC::lbool> {
  public:
    Eval(Expressions &factory, const Substitution &vals)
        : MemoizedExpressionVisitor<SATSPC::lbool>(factory), vals(vals) {}

    SATSPC::lbool operator()(ID n) { return visit(n); }

    virtual SATSPC::lbool visit_lit(ID, Lit lit) {
        const auto i = vals.find(var(lit));
        if (i == vals.end())
            return SATSPC::l_Undef;
        return SATSPC::sign(lit) != i->second ? SATSPC::l_True
                                              : SATSPC::l_False;
    }

    virtual SATSPC::lbool visit_and(ID, IDVector operands) {
        bool allt = true;
        for (const auto &i : operands) {
            const SATSPC::lbool ri = visit(i);
            if (ri == SATSPC::l_False)
                return SATSPC::l_False;
            if (ri == SATSPC::l_Undef)
                allt = false;
        }
        return allt ? SATSPC::l_True : SATSPC::l_Undef;
    }

    virtual SATSPC::lbool visit_not(ID, ID operand) {
        const auto v = visit(operand);
        if (v == SATSPC::l_False)
            return SATSPC::l_True;
        if (v == SATSPC::l_True)
            return SATSPC::l_False;
        assert(v == SATSPC::l_Undef);
        return SATSPC::l_Undef;
    }

    virtual SATSPC::lbool visit_or(ID, IDVector operands) {
        bool allf = true;
        for (const auto &i : operands) {
            const SATSPC::lbool ri = visit(i);
            if (ri == SATSPC::l_True)
                return SATSPC::l_True;
            if (ri == SATSPC::l_Undef)
                allf = false;
        }
        return allf ? SATSPC::l_False : SATSPC::l_Undef;
    }

    virtual SATSPC::lbool visit_false(ID) { return SATSPC::l_False; }
    virtual SATSPC::lbool visit_true(ID) { return SATSPC::l_True; }

  private:
    const Substitution &vals;
};
} // namespace qesto
