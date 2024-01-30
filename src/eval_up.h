/*
 * File:  eval_up.h
 * Author:  mikolas
 * Created on:  Tue Jan 30 08:20:24 CET 2024
 * Copyright (C) 2024, Mikolas Janota
 */
#pragma once
#include "expressions.h"
#include "inversion.h"
#include "sat_interface.h"
#include "visitor.h"
#include <unordered_map>
#include <vector>
namespace qesto {

/** Bottom-up evaluation of the circuit given values of variables. */
class EvalUp {
  public:
    /** Given expression factory,  substitutions into (some) variables, and
     * inverted  formula graph. */
    EvalUp(Expressions &factory, const Substitution &var_vals,
           const std::unordered_map<ID, std::vector<ID>> &inv)
        : m_factory(factory), m_var_vals(var_vals), m_inv(inv) {
        run();
    }

    SATSPC::lbool operator()(const ID &n) const {
        assert(m_was_run);
        if (n.get_type()==FALSE) return SATSPC::l_False; 
        if (n.get_type()==TRUE) return SATSPC::l_True; 
        const auto i = m_vals.find(n);
        return i == m_vals.end()
                   ? SATSPC::l_Undef
                   : (i->second ? SATSPC::l_True : SATSPC::l_False);
    }

  private:
    Expressions &m_factory;
    const Substitution &m_var_vals;
    bool m_was_run = false;
    std::unordered_map<ID, bool> m_vals;
    const std::unordered_map<ID, std::vector<ID>> &m_inv;

    std::vector<pair<ID, bool>> m_todo; // stack of (node,prop_val)
    std::unordered_map<ID, size_t>
        m_score; //  for in AND how many children are propagated true, for OR
                 //  how many are false

    /** bump score of the given node, return true iff the number of children is
     * equal to the score */
    bool bump_score(const ID &p) {
        auto &f = m_factory;
        assert(p.get_type() == AND || p.get_type() == OR);
        auto i = m_score.find(p);
        const auto num_ops =
            p.get_type() == AND ? f.open_and(p).size() : f.open_or(p).size();
        if (i == m_score.end()) {
            m_score.insert(i, {p, 1});
            return num_ops == 1;
        }
        i->second = i->second + 1;
        return num_ops == i->second;
    }
    void push_to_parent(const ID &p, bool val);
    void run();
};
} // namespace qesto
