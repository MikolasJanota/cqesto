#pragma once
#include "data_structures.h"
#include "idvector.h"
#include "visitor.h"
namespace qesto {
class Transform : public MemoizedExpressionVisitor<ID> {
  public:
    using ExpressionVisitor<ID>::factory;
    using ExpressionVisitor<ID>::get_factory;

    Transform(Expressions &factory) : MemoizedExpressionVisitor<ID>(factory) {}

    virtual ID visit_true(ID node) { return node; }

    virtual ID visit_false(ID node) { return node; }

    virtual ID visit_lit(ID node, Lit) { return node; }

    virtual ID visit_and(ID node, IDVector operands) {
        return visit_lin(AND, node, operands);
    }

    virtual ID visit_or(ID node, IDVector operands) {
        return visit_lin(OR, node, operands);
    }

    virtual ID visit_not(ID node, ID operand) {
        const ID recursive_result = visit(operand);
        const bool changed = recursive_result != operand;
        return changed ? factory.make_not(recursive_result) : node;
    }

  private:
    ID visit_lin(NodeType op, ID node, IDVector operands) {
        assert(op == node.get_type());
        assert(op == AND || op == OR);
        std::vector<ID> d;
        const ID absorb =
            op == AND ? factory.make_false() : factory.make_true();
        bool changed = false;

        for (const auto &i : operands) {
            const ID nv = visit(i);
            if (nv == absorb)
                return absorb;
            changed |= nv != i;
            changed |= factory.add_operand(d, nv, op);
        }
        if (!changed)
            return node;
        return op == AND ? factory.make_and(IDVector(d))
                         : factory.make_or(IDVector(d));
    }
};
} // namespace qesto
