/*
 * File:  eval_up.cpp
 * Author:  mikolas
 * Created on:  Tue Jan 30 08:25:26 CET 2024
 * Copyright (C) 2024, Mikolas Janota
 */
#include "eval_up.h"
#include <unordered_set>
namespace qesto {

void EvalUp::run(const Substitution &var_vals) {
    auto &f = m_factory;
    m_todo.clear();
    m_vals.clear();

    // put all literals on stack according to the given substitution
    for (const auto &[var, val] : var_vals) {
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
        /* m_np << "top,val:" << top << " " << val << '\n'; */
        // go through parents
        push_all(m_push[ix(val)], top, val);
        push_all(m_push_neg, top, !val);
        push_may(m_may[ix(val)], top, val);
    }
    m_was_run = true;
}

void EvalUp::push_may(std::unordered_map<ID, std::unordered_set<ID>> &push,
                      ID n, bool val) {
    auto &f = m_factory;
    const auto i = push.find(n);
    if (i == push.end())
        return;
    auto &witnessed = i->second;
    std::vector<ID> to_rm;
    for (const auto &p : witnessed) {
        // find a different witness among p's operands
        const auto &pops = p.get_type() == AND ? f.open_and(p) : f.open_or(p);
        bool found = false;
        ID new_witness;
        for (const auto &pop : pops) {
            if (pop == n)
                continue;
            if (const auto j = m_vals.find(pop);
                j == m_vals.end() || j->second != val) {
                found = true;
                new_witness = pop;
                break;
            }
        }
        if (found) {
            push[new_witness].insert(p);
            to_rm.push_back(p);
        } else {
            m_todo.push_back({p, val}); // no more witnesses, propagate
        }
    }
    if (to_rm.size() == witnessed.size())
        witnessed.clear(); // n no longer witnesses anything
    else
        for (const auto &p : to_rm)
            witnessed.erase(p);
}

void EvalUp::push_all(const std::unordered_map<ID, std::vector<ID>> &push, ID n,
                      bool val) {
    const auto i = push.find(n);
    if (i == push.end())
        return;
    for (const auto &p : i->second)
        m_todo.push_back({p, val});
}

inline void add_edge(std::unordered_map<ID, std::unordered_set<ID>> &g,
                     const ID &from, const ID &to) {
    const auto [i, _] = g.insert({from, std::unordered_set<ID>()});
    i->second.insert(to);
}

inline void add_edge(std::unordered_map<ID, std::vector<ID>> &g, const ID &from,
                     const ID &to) {
    const auto [i, _] = g.insert({from, std::vector<ID>()});
    i->second.push_back(to);
}

void EvalUp::init(const ID &node) {
    std::vector<ID> todo{node};
    m_was_run = false;
    do {
        ID top = todo.back();
        todo.pop_back();
        switch (top.get_type()) {
        case NEGATION: {
            const auto [_, succ] = m_seen.insert(top);
            if (!succ)
                continue;
            const auto nn = m_factory.open_not(top);
            add_edge(m_push_neg, nn, top);
            todo.push_back(nn);
        } break;
        case AND:
        case OR: {
            const auto [_, succ] = m_seen.insert(top);
            if (!succ)
                continue;
            const auto &ops = top.get_type() == AND ? m_factory.open_and(top)
                                                    : m_factory.open_or(top);
            auto &push = m_push[ix(top.get_type() == OR)];
            for (const auto &op : ops) {
                add_edge(push, op, top);
                todo.push_back(op);
            }
            auto &may = m_may[ix(top.get_type() == AND)];
            add_edge(may, ops[0], top);
        } break;
        case FALSE:
        case TRUE:
        case LITERAL: /*nop*/ break;
        }
    } while (!todo.empty());
}

} // namespace qesto
