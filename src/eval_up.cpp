/*
 * File:  eval_up.cpp
 * Author:  mikolas
 * Created on:  Tue Jan 30 08:25:26 CET 2024
 * Copyright (C) 2024, Mikolas Janota
 */
#include "eval_up.h"
namespace qesto {

void EvalUp::run() {
    auto &f = m_factory;

    // put all literals on stack according to the given substitution
    for (const auto &[var, val] : m_var_vals) {
        auto l = SATSPC::mkLit(var);
        m_todo.push_back({f.make_lit(l), val});
        m_todo.push_back({f.make_lit(~l), !val});
    }

    // recursive with explicit stack
    while (!m_todo.empty()) {
        auto [top, val] = m_todo.back();
        m_todo.pop_back();
        const auto [_, success] = m_vals.insert({top, val});
        if (!success)
            continue; // already propagated
        const auto it = m_inv.find(top);
        if (it == m_inv.end())
            continue; // no parents
        // go thru parents
        for (const auto &p : it->second)
            push_to_parent(p, val);
    }
    m_was_run = true;
}

void EvalUp::push_to_parent(const ID &p, bool val) {
    switch (p.get_type()) {
    case AND:
        if (val) {
            if (bump_score(p)) // all true
                m_todo.push_back({p, true});
        } else
            m_todo.push_back({p, false}); // false child
        break;
    case OR:
        if (!val) {
            if (bump_score(p)) // all false
                m_todo.push_back({p, false});
        } else
            m_todo.push_back({p, true}); // true child
        break;
    case NEGATION: m_todo.push_back({p, !val}); break;
    case LITERAL:
    case FALSE:
    case TRUE: assert(false); break;
    }
}

} // namespace qesto
