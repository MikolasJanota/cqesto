/*
 * File:   find_cut.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 15:00
 */

#pragma once
#include "eval_up.h"
#include "expressions.h"
#include <unordered_map>
namespace qesto {
class FindCutNoRec {
  public:
    FindCutNoRec(Expressions &factory, const EvalUp &vals,
                 std::unordered_set<ID> &pcut)
        : m_factory(factory), m_vals(vals), m_cut(pcut) {}

    void operator()(ID n) { visit(n); }

  protected:
    Expressions &m_factory;
    const EvalUp &m_vals;
    std::unordered_set<ID> &m_cut;
    std::unordered_set<ID> m_seen;

    void visit(const ID &node) {
        auto &f = m_factory;
        std::vector<ID> todo{node};
        do {
            ID top = todo.back();
            todo.pop_back();
            const auto [_, succ] = m_seen.insert(top);
            if (!succ)
                continue;
            const SATSPC::lbool val = m_vals(top);

            if (val != SATSPC::l_Undef) {
                m_cut.insert(val == SATSPC::l_True ? top : f.make_not(top));
                continue;
            }

            switch (top.get_type()) {
            case NEGATION: todo.push_back(f.open_not(top)); break;
            case OR:
                for (const auto &op : f.open_or(top))
                    todo.push_back(op);
                break;
            case AND:
                for (const auto &op : f.open_and(top))
                    todo.push_back(op);
                break;
            case FALSE:
            case TRUE: m_cut.insert(top); break;
            case LITERAL:
                /*nop*/ break;
            }
        } while (!todo.empty());
    }
};
} // namespace qesto
