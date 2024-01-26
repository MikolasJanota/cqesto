/*
 * File:   find_cut.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 15:00
 */

#pragma once
#include "eval.h"
#include "expressions.h"
#include "visitor.h"
#include <unordered_map>
namespace qesto {
class FindCut : MemoizedExpressionVisitor<bool> {
  public:
    typedef std::unordered_map<Var, bool> Vals;

    FindCut(Expressions &factory, Eval &vals)
        : MemoizedExpressionVisitor<bool>(factory), vals(vals) {}

    void operator()(ID n) { visit(n); }
    const std::unordered_set<ID> &get_cut() const {
        return cut;
    }

    virtual bool visit_lit(ID node, Lit) {
        const SATSPC::lbool v = vals(node);
        if (v != SATSPC::l_Undef)
            cut.insert(v == SATSPC::l_True ? node : factory.make_not(node));
        return true;
    }

    virtual bool visit_not(ID node, ID operand) {
        const SATSPC::lbool v = vals(node);
        if (v != SATSPC::l_Undef) {
            cut.insert(v == SATSPC::l_True ? node : factory.make_not(node));
            return true;
        }
        return visit(operand);
    }

    virtual bool visit_or(ID node, IDVector operands) {
        const SATSPC::lbool v = vals(node);
        if (v != SATSPC::l_Undef) {
            cut.insert(v == SATSPC::l_True ? node : factory.make_not(node));
            return true;
        }
        return visit_ops(operands);
    }

    virtual bool visit_and(ID node, IDVector operands) {
        const SATSPC::lbool v = vals(node);
        if (v != SATSPC::l_Undef) {
            cut.insert(v == SATSPC::l_True ? node : factory.make_not(node));
            return true;
        }
        return visit_ops(operands);
    }

    virtual bool visit_false(ID) { return true; }
    virtual bool visit_true(ID node) {
        cut.insert(node);
        return true;
    }

  private:
    Eval &vals;
    std::unordered_set<ID> cut;
    inline bool visit_ops(IDVector ops) {
        for (const auto &i : ops)
            visit(i);
        return true;
    }
};
} // namespace qesto
