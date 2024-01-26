/*
 * File:  expressions.h
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 4:52:43 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#pragma once
#include "idvector.h"
#include "immutable_vector.h"
#include "options.h"
#include "table.h"
namespace qesto {
typedef ImmutableVector<Var, std::hash<int>> VarVector;
typedef ImmutableVector_hash<ID, std::hash<ID>> IDVector_hash;
typedef ImmutableVector_equal<ID, std::hash<ID>> IDVector_equal;
typedef std::unordered_map<Var, bool> Substitution;

class Expressions {
  public:
    Expressions(const Options &options) : options(options) {}

    ID make_false();
    ID make_true();
    ID make_lit(Lit l);
    ID make_and(IDVector operands);
    ID make_not(ID operand);
    ID make_or(IDVector operands);

    inline ID make_and(ID o1, ID o2) {
        if (o1.get_type() == TRUE)
            return o2;
        if (o2.get_type() == TRUE)
            return o1;
        if (o1.get_type() == FALSE)
            return make_false();
        if (o2.get_type() == FALSE)
            return make_false();
        std::vector<ID> dv(2);
        dv[0] = o1;
        dv[1] = o2;
        return make_and(IDVector(dv));
    }

    inline ID make_or(ID o1, ID o2) {
        if (o1.get_type() == FALSE)
            return o2;
        if (o2.get_type() == FALSE)
            return o1;
        if (o1.get_type() == TRUE)
            return make_true();
        if (o2.get_type() == TRUE)
            return make_true();
        std::vector<ID> dv(2);
        dv[0] = o1;
        dv[1] = o2;
        return make_or(IDVector(dv));
    }

    inline Lit open_lit(ID node) const {
        assert(node.get_type() == LITERAL);
        return ID::getLit(node);
    }

    inline bool is_complement_lit(ID o1, ID o2) {
        if (o1.get_type() != LITERAL)
            return false;
        if (o2.get_type() != LITERAL)
            return false;
        return open_lit(o1) == ~open_lit(o2);
    }

    const IDVector open_and(ID node) const;
    const IDVector open_or(ID node) const;
    const ID open_not(ID node) const;

    // returns true for the cases when it does not push o into operands
    bool add_operand(std::vector<ID> &operands, ID o, NodeType top_level) {
        assert((top_level == OR) || (top_level == AND));
        const NodeType ide = (top_level == OR) ? FALSE : TRUE;
        if (o.get_type() == ide)
            return true;                                    // NOP
        if (options.flatten && o.get_type() == top_level) { // flatten
            const IDVector &os = (top_level == OR) ? open_or(o) : open_and(o);
            for (const auto &co : os)
                add_operand(operands, co, top_level);
            return true;
        } else {
            operands.push_back(o);
            return false;
        }
    }

  private:
    const Options &options;
    Table<IDVector, IDVector_hash, IDVector_equal> ands;
    Table<IDVector, IDVector_hash, IDVector_equal> ors;
    Table<ID, std::hash<ID>, std::equal_to<ID>> nots;
};
} // namespace qesto
