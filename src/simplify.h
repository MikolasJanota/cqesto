#pragma once
#include "encoder_to_sat.h"
#include "reduce.h"
#include "transform.h"
#include <vector>
namespace qesto {
class Simplify : private Transform {
  public:
    Simplify(const Options &options, Expressions &factory, EncoderToSAT &enc)
        : Transform(factory), options(options), enc(enc) {}

    ID operator()(ID expression) { return visit(expression); }

#ifdef USE_MINISAT
    virtual ID visit(ID node) override {
        const auto nv = enc.get_val(node);
        if (nv == Minisat::l_False)
            return factory.make_false();
        if (nv == Minisat::l_True)
            return factory.make_true();
        return Transform::visit(node);
    }
#endif

    virtual ID visit_and(ID node, IDVector operands) override {
        if (options.unit)
            return visit_and_or(node, operands, AND);
        else
            return Transform::visit_and(node, operands);
    }

    virtual ID visit_or(ID node, IDVector operands) override {
        if (options.unit)
            return visit_and_or(node, operands, OR);
        else
            return Transform::visit_or(node, operands);
    }

  protected:
    const Options &options;
    EncoderToSAT &enc;

    ID visit_and_or(ID node, IDVector operands, const NodeType operation) {
        assert(operation == AND || operation == OR);
        bool is_mixed;
        std::vector<ID> recursed;
        bool changed;
        changed = recurse(operands, operation, is_mixed, recursed);
        if (!is_mixed) {
            if (changed) {
                if (operation == AND)
                    return factory.make_and(recursed);
                else
                    return factory.make_or(recursed);
            } else
                return node;
        }

        Substitution substitution;
        make_substitution(recursed, operation, substitution);
        std::vector<ID> reduced;
        changed |=
            apply_substitution(substitution, recursed, operation, reduced);
        if (changed) {
            if (reduced.size() < 20 && has_neg(reduced)) {
                return operation == AND ? factory.make_false()
                                        : factory.make_true();
            }
            if (operation == AND)
                return factory.make_and(reduced);
            else
                return factory.make_or(reduced);
        } else {
            return node;
        }
    }

    bool recurse(const IDVector &operands, const NodeType operation,
                 bool &is_mixed, std::vector<ID> &new_operands) {
        assert(operation == AND || operation == OR);
        bool cx = false;   // there are complex expressions
        bool lits = false; // there are literals
        bool changed = false;

        for (const auto &old_n : operands) {
            const ID rn = visit(old_n);
            changed |= rn != old_n;
            changed |= factory.add_operand(new_operands, rn, operation);
            if (rn.get_type() == LITERAL)
                lits = true;
            else
                cx = true;
        }
        is_mixed = lits && cx;
        return changed;
    }

    void make_substitution(const std::vector<ID> &operands,
                           const NodeType operation,
                           Substitution &substitution) {
        assert(operation == AND || operation == OR);
        const bool negate = operation == OR;

        for (const auto &n : operands) {
            if (n.get_type() == LITERAL) {
                const Lit l = factory.open_lit(n);
                substitution[var(l)] = negate == sign(l);
            }
        }
    }

    bool apply_substitution(const Substitution &substitution,
                            const std::vector<ID> &operands,
                            const NodeType operation,
                            std::vector<ID> &new_operands) {
        Reduce reduce(factory, substitution);
        bool changed = false;

        for (const auto &old_n : operands) {
            if (old_n.get_type() == LITERAL) {
                new_operands.push_back(old_n);
            } else {
                const ID rn = reduce(old_n);
                changed |= rn != old_n;
                changed |= factory.add_operand(new_operands, rn, operation);
            }
        }
        return changed;
    }

    bool has_neg(IDVector ids) {
        if (ids.contains_compl())
            return true;
        for (size_t i = 0; i < ids.size(); ++i) {
            const auto &iid = ids[i];
            if (iid.get_type() != NEGATION)
                continue;
            const auto &inid = factory.open_not(iid);
            for (size_t j = i + 1; j < ids.size(); ++j) {
                if (ids[j] == inid)
                    return true;
            }
        }
        return false;
    }
};
} // namespace qesto
