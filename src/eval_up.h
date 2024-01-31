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
#include <unordered_set>
#include <vector>
namespace qesto {

/** Bottom-up evaluation of the circuit given values of variables. */
class EvalUp {
  public:
    /** Given expression factory,  substitutions into (some) variables, and
     * inverted  formula graph. */
    EvalUp(Expressions &factory) : m_factory(factory) {}

    void init(const ID &n);
    void run(const Substitution &var_vals);

    SATSPC::lbool operator()(const ID &n) const {
        assert(m_was_run);
        if (n.get_type() == FALSE)
            return SATSPC::l_False;
        if (n.get_type() == TRUE)
            return SATSPC::l_True;
        const auto i = m_vals.find(n);
        return i == m_vals.end()
                   ? SATSPC::l_Undef
                   : (i->second ? SATSPC::l_True : SATSPC::l_False);
    }

  private:
    Expressions &m_factory;
    bool m_was_run = false;
    std::unordered_map<ID, bool> m_vals;
    std::unordered_map<ID, std::vector<ID>> m_push_neg;
    std::unordered_map<ID, std::vector<ID>> m_push[2];
    std::unordered_map<ID, std::unordered_set<ID>> m_may[2];
    std::unordered_set<ID> m_seen;
    size_t ix(bool b) const { return b ? 1 : 0; }

    std::vector<pair<ID, bool>> m_todo; // stack of (node,prop_val)
                                        //
    void push_may(std::unordered_map<ID, std::unordered_set<ID>> &push, ID n,
                  bool val);
    void push_all(const std::unordered_map<ID, std::vector<ID>> &push, ID n,
                  bool val);
    void push_to_parent(const ID &p, bool val);
    void run();
};
} // namespace qesto
