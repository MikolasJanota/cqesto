/*
 * File:  dbg_cmp_eval.h
 * Author:  mikolas
 * Created on:  Thu Feb 1 15:04:15 CET 2024
 * Copyright (C) 2024, Mikolas Janota
 */
#pragma once
#include "expressions.h"
#include "visitor.h"
#include <unordered_map>
namespace qesto {

template <class EvalT1, class EvalT2>
class DbgCmpEval : MemoizedExpressionVisitor<bool> {
  public:
    typedef std::unordered_map<Var, bool> Vals;

    DbgCmpEval(Expressions &factory, NiceExpressionPrinter &prn, EvalT1 &vals1,
               EvalT2 &vals2)
        : MemoizedExpressionVisitor<bool>(factory), m_prn(prn), m_ev1(vals1),
          m_ev2(vals2) {}

    bool operator()(ID n) { return visit(n); }

    virtual bool visit_lit(ID node, Lit) { return chk(node); }

    virtual bool visit_not(ID node, ID operand) {
        return chk(node) & visit(operand);
    }

    virtual bool visit_or(ID node, IDVector operands) {
        return chk(node) & visit_ops(operands);
    }

    virtual bool visit_and(ID node, IDVector operands) {
        return chk(node) & visit_ops(operands);
    }

    virtual bool visit_false(ID node) { return chk(node); }
    virtual bool visit_true(ID node) { return chk(node); }

  private:
    NiceExpressionPrinter &m_prn;
    EvalT1 &m_ev1;
    EvalT2 &m_ev2;
    bool chk(ID n) {
        const auto v1 = m_ev1(n);
        const auto v2 = m_ev2(n);
        if (v1 != v2)
            m_prn << "differ " << n << ":" << v1 << ":" << v2 << '\n';
        return v1 == v2;
    }

    inline bool visit_ops(IDVector ops) {
        bool rv = true;
        for (const auto &i : ops)
            rv &= visit(i);
        return rv;
    }
};
} // namespace qesto
