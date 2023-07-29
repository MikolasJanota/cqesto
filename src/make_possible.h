/*
 * File:   make_possible.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 17:02
 */
#pragma once
#include "visitor.h"
namespace qesto {

class MakePossible : protected MemoizedExpressionArgumentVisitor<ID, bool> {
  public:
    MakePossible(Expressions &factory, const std::unordered_set<Var> &domain,
                 const Substitution &vals)
        : MemoizedExpressionArgumentVisitor<ID, bool>(factory), domain(domain),
          vals(vals) {}

    virtual ~MakePossible();

    ID operator()(ID n) { return visit(n, true); }

    virtual ID visit_lit(ID, bool argument, Lit lit) override {
        const auto v = var(lit);
        if (domain.count(v))
            return factory.make_lit(argument ? lit : ~lit);
        const auto j = vals.find(v);
        if (j == vals.end())
            return factory.make_true();
        const auto rv = (sign(lit) != j->second) == argument;
        return rv ? factory.make_true() : factory.make_false();
    }

    virtual ID visit_and(ID node, bool argument, IDVector operands) override {
        return argument ? join_ops(node, AND, true, operands)
                        : join_ops(node, OR, false, operands);
    }

    virtual ID visit_not(ID, bool argument, ID operand) override {
        return visit(operand, !argument);
    }

    virtual ID visit_or(ID node, bool argument, IDVector operands) override {
        return argument ? join_ops(node, OR, true, operands)
                        : join_ops(node, AND, false, operands);
    }

    virtual ID visit_false(ID, bool argument) override {
        return !argument ? factory.make_true() : factory.make_false();
    }

    virtual ID visit_true(ID, bool argument) override {
        return argument ? factory.make_true() : factory.make_false();
    }

  private:
    const std::unordered_set<Var> &domain;
    const Substitution &vals;

    ID join_ops(ID node, NodeType op, bool v, const IDVector &ops) {
        assert(op == AND || op == OR);
        std::vector<ID> d;
        const ID absorb =
            op == AND ? factory.make_false() : factory.make_true();
        bool changed = false;

        for (const auto &o : ops) {
            const ID nv = visit(o, v);
            if (nv == absorb)
                return absorb;
            changed |= nv != o;
            changed |= factory.add_operand(d, nv, op);
        }
        if (!changed && (node.get_type() == op))
            return node;
        const IDVector resops = changed ? IDVector(d) : ops;
        return op == AND ? factory.make_and(resops) : factory.make_or(resops);
    }
};
} // namespace qesto
