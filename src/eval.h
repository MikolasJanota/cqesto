/*
 * File:   eval.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 14:22
 */

#pragma once
#include "expressions.h"
#include "minisat/core/SolverTypes.h"
#include "visitor.h"
#include <unordered_map>
namespace qesto {
class Eval : MemoizedExpressionVisitor<Minisat::lbool> {
  public:
    Eval(Expressions &factory, const Substitution &vals)
        : MemoizedExpressionVisitor<Minisat::lbool>(factory), vals(vals) {}

    Minisat::lbool operator()(ID n) { return visit(n); }

    virtual Minisat::lbool visit_lit(ID, Lit lit) {
        const auto i = vals.find(var(lit));
        if (i == vals.end())
            return Minisat::l_Undef;
        return Minisat::sign(lit) != i->second ? Minisat::l_True
                                               : Minisat::l_False;
    }

    virtual Minisat::lbool visit_and(ID, IDVector operands) {
        bool allt = true;
        for (const auto &i : operands) {
            const Minisat::lbool ri = visit(i);
            if (ri == Minisat::l_False)
                return Minisat::l_False;
            if (ri == Minisat::l_Undef)
                allt = false;
        }
        return allt ? Minisat::l_True : Minisat::l_Undef;
    }

    virtual Minisat::lbool visit_not(ID, ID operand) {
        const auto v = visit(operand);
        if (v == Minisat::l_False)
            return Minisat::l_True;
        if (v == Minisat::l_True)
            return Minisat::l_False;
        assert(v == Minisat::l_Undef);
        return Minisat::l_Undef;
    }

    virtual Minisat::lbool visit_or(ID, IDVector operands) {
        bool allf = true;
        for (const auto &i : operands) {
            const Minisat::lbool ri = visit(i);
            if (ri == Minisat::l_True)
                return Minisat::l_True;
            if (ri == Minisat::l_Undef)
                allf = false;
        }
        return allf ? Minisat::l_False : Minisat::l_Undef;
    }

    virtual Minisat::lbool visit_false(ID) { return Minisat::l_False; }
    virtual Minisat::lbool visit_true(ID) { return Minisat::l_True; }

  private:
    const Substitution &vals;
};
} // namespace qesto
