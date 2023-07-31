/*
 * File:  expressions.cpp
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 6:29:28 PM
 * Copyright (C) 2015, Mikolas Janota
 */

#include "expressions.h"
using namespace qesto;
// namespace qesto {

ID Expressions::make_false() { return ID(FALSE, 0); }

ID Expressions::make_true() { return ID(TRUE, 0); }

ID Expressions::make_lit(Lit l) { return ID::mkLit(l); }

ID Expressions::make_and(IDVector operands) {
    const auto sz = operands.size();
    if (sz == 0)
        return make_true();
    if (sz == 1)
        return operands[0];
    if (operands.contains_compl() || operands.contains_false())
        return make_false();
    return ID(AND, ands.lookup(operands));
}

ID Expressions::make_or(IDVector operands) {
    const auto sz = operands.size();
    if (sz == 0)
        return make_false();
    if (sz == 1)
        return operands[0];
    if (operands.contains_compl() || operands.contains_true())
        return make_true();
    if (!options.aig)
        return ID(OR, ors.lookup(operands));
    std::vector<ID> new_ops;
    new_ops.reserve(operands.size());

    for (const auto &n : operands) {
        if (n.get_type() == FALSE)
            continue;
        if (n.get_type() == TRUE)
            return make_true();
        new_ops.push_back(make_not(n));
    }
    return make_not(ID(AND, ands.lookup(IDVector(new_ops))));
}

ID Expressions::make_not(ID operand) {
    switch (operand.get_type()) {
    case FALSE: return make_true();
    case TRUE: return make_false();
    case LITERAL: return make_lit(~open_lit(operand));
    case NEGATION: return open_not(operand);
    default: return ID(NEGATION, nots.lookup(operand));
    }
}

const IDVector Expressions::open_and(ID node) const {
    assert(node.get_type() == AND);
    return ands.get(node.get_index());
}

const IDVector Expressions::open_or(ID node) const {
    assert(node.get_type() == OR);
    return ors.get(node.get_index());
}

const ID Expressions::open_not(ID node) const {
    assert(node.get_type() == NEGATION);
    return nots.get(node.get_index());
}
