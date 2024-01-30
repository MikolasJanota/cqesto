/*
 * File:  inversion.h
 * Author:  mikolas
 * Created on:  Mon Jan 29 13:03:32 CET 2024
 * Copyright (C) 2024, Mikolas Janota
 */
#pragma once
#include "expressions.h"
#include <unordered_map>
#include <unordered_set>
#include <vector>
namespace qesto {

/**  Reverses the edges in the formula graph. */
class MkInversion {
  public:
    MkInversion(Expressions &factory) : m_factory(factory) {}
    void operator()(const ID &n) { visit(n); }

    const std::unordered_map<ID, std::vector<ID>> &inv() { return m_inv; }

  private:
    Expressions &m_factory;
    std::unordered_map<ID, std::vector<ID>> m_inv;
    std::unordered_set<ID> m_seen;

    inline void add_edge(const ID &from, const ID &to) {
        const auto [i, _] = m_inv.insert({from, std::vector<ID>()});
        i->second.push_back(to);
    }

    void visit(const ID &node) {
        std::vector<ID> todo{node};
        do {
            ID top = todo.back();
            todo.pop_back();
            switch (top.get_type()) {
            case NEGATION: {
                const auto [_, succ] = m_seen.insert(top);
                if (!succ)
                    continue;
                const auto nn = m_factory.open_not(top);
                add_edge(nn, top);
                todo.push_back(nn);
            } break;
            case AND:
            case OR: {
                const auto [_, succ] = m_seen.insert(top);
                if (!succ)
                    continue;
                const auto &ops = top.get_type() == AND
                                      ? m_factory.open_and(top)
                                      : m_factory.open_or(top);
                for (const auto &op : ops) {
                    add_edge(op, top);
                    todo.push_back(op);
                }
            } break;
            case FALSE:
            case TRUE:
            case LITERAL: /*nop*/ break;
            }
        } while (!todo.empty());
    }
};
} // namespace qesto
