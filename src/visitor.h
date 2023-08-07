/*
 * File:  visitor.h
 * Author:  mikolas
 * Created on:  Fri, May 15, 2015 4:52:48 PM
 * Copyright (C) 2015, Mikolas Janota
 */
#pragma once
#include "expressions.h"

namespace qesto {
template <class R> class ExpressionVisitor {
  public:
    ExpressionVisitor(Expressions &factory) : factory(factory) {}
    virtual ~ExpressionVisitor() {}

    virtual R visit(ID expression) {
        switch (expression.get_type()) {
        case LITERAL:
            return visit_lit(expression, factory.open_lit(expression));
        case NEGATION:
            return visit_not(expression, factory.open_not(expression));
        case AND: return visit_and(expression, factory.open_and(expression));
        case OR: return visit_or(expression, factory.open_or(expression));
        case FALSE: return visit_false(expression);
        case TRUE: return visit_true(expression);
        }
        assert(0);
        abort();
    }

    virtual R visit_lit(ID node, Lit lit) = 0;
    virtual R visit_and(ID node, IDVector operands) = 0;
    virtual R visit_not(ID node, ID operand) = 0;
    virtual R visit_or(ID node, IDVector operands) = 0;
    virtual R visit_false(ID node) = 0;
    virtual R visit_true(ID node) = 0;
    Expressions &get_factory() { return factory; }

  protected:
    Expressions &factory;
};

template <class R, class A> class ExpressionArgumentVisitor {
  public:
    ExpressionArgumentVisitor(Expressions &factory) : factory(factory) {}
    virtual ~ExpressionArgumentVisitor() {}

    virtual R visit(ID expression, A argument) {
        switch (expression.get_type()) {
        case LITERAL:
            return visit_lit(expression, argument,
                             factory.open_lit(expression));
        case NEGATION:
            return visit_not(expression, argument,
                             factory.open_not(expression));
        case AND:
            return visit_and(expression, argument,
                             factory.open_and(expression));
        case OR:
            return visit_or(expression, argument, factory.open_or(expression));
        case FALSE: return visit_false(expression, argument);
        case TRUE: return visit_true(expression, argument);
        }
        std::cerr << "FATAL ERROR: " << expression.get_type() << std::endl;
        assert(0);
        abort();
    }

    virtual R visit_lit(ID node, A argument, Lit lit) = 0;
    virtual R visit_and(ID node, A argument, IDVector operands) = 0;
    virtual R visit_not(ID node, A argument, ID operand) = 0;
    virtual R visit_or(ID node, A argument, IDVector operands) = 0;
    virtual R visit_false(ID node, A argument) = 0;
    virtual R visit_true(ID node, A argument) = 0;
    Expressions &get_factory() { return factory; }

  protected:
    Expressions &factory;
};

template <class R>
class MemoizedExpressionVisitor : public ExpressionVisitor<R> {
  public:
    MemoizedExpressionVisitor(Expressions &factory)
        : ExpressionVisitor<R>(factory) {}
    virtual R visit(ID expression) {
        const auto i = m.find(expression);
        if (i != m.end())
            return i->second;
        R val = ExpressionVisitor<R>::visit(expression);
        m[expression] = val;
        return val;
    }

  protected:
    inline const std::unordered_map<ID, R, ID_hash, ID_equal> &get_m() {
        return m;
    }

  private:
    std::unordered_map<ID, R, ID_hash, ID_equal> m;
};

template <class R, class A>
class MemoizedExpressionArgumentVisitor
    : public ExpressionArgumentVisitor<R, A> {
  public:
    typedef pair<ID, A> KeyType;

    MemoizedExpressionArgumentVisitor(Expressions &factory)
        : ExpressionArgumentVisitor<R, A>(factory) {}

    virtual R visit(ID expression, A argument) {
        KeyType key(expression, argument);
        const auto i = m.find(key);
        if (i != m.end())
            return i->second;
        R val = ExpressionArgumentVisitor<R, A>::visit(expression, argument);
        m[key] = val;
        return val;
    }

  private:
    std::unordered_map<KeyType, R> m;
};

class ExpressionPrinter : public ExpressionVisitor<std::ostream &> {
  public:
    ExpressionPrinter(std::ostream &output, Expressions &factory)
        : ExpressionVisitor<std::ostream &>(factory), id_to_orig(nullptr),
          output(output) {}

    std::ostream &operator()(ID node) { return visit(node); }

    virtual std::ostream &visit_var(Var v) {
        if (id_to_orig) {
            ID id = factory.make_lit(SATSPC::mkLit(v));
            int orig = (*id_to_orig)[id.toInt()];
            return output << orig;
        } else {
            return output << v;
        }
    }

    virtual std::ostream &visit_lit(ID, Lit lit) {
        output << (sign(lit) ? '-' : '+');
        return visit_var(var(lit));
    }

    virtual std::ostream &visit_and(ID, IDVector operands) {
        output << "AND (";
        print_vector(operands);
        return output << ")";
    }

    virtual std::ostream &visit_not(ID, ID operand) {
        output << "NOT (";
        visit(operand);
        return output << ")";
    }

    virtual std::ostream &visit_or(ID, IDVector operands) {
        output << "OR (";
        print_vector(operands);
        return output << ")";
    }

    virtual std::ostream &visit_false(ID) { return output << "F"; }
    virtual std::ostream &visit_true(ID) { return output << "T"; }

    std::map<uint64_t, int> *id_to_orig;

  private:
    std::ostream &output;
    std::ostream &print_vector(IDVector operands) {
        bool first = true;
        for (const auto &n : operands) {
            const bool print_parentheses = n.get_type() != LITERAL;
            if (!first)
                output << " ";
            if (print_parentheses)
                output << "(";
            visit(n);
            if (print_parentheses)
                output << ')';
            first = false;
        }
        return output;
    }
};

class NiceExpressionPrinter
    : private ExpressionArgumentVisitor<std::ostream &, size_t> {
  public:
    NiceExpressionPrinter(Expressions &factory,
                          const std::unordered_map<int, std::string> &var2name,
                          std::ostream &output)
        : ExpressionArgumentVisitor<std::ostream &, size_t>(factory),
          output(output), var2name(var2name) {}

    std::ostream &operator()(ID node, size_t offset = 0) {
        return visit(node, offset);
    }

    NiceExpressionPrinter &operator<<(const char c) {
        output << c;
        return *this;
    }
    NiceExpressionPrinter &operator<<(const char *s) {
        output << s;
        return *this;
    }

    NiceExpressionPrinter &operator<<(Var v) {
        const auto i = var2name.find(v);
        if (i == var2name.end()) {
            output << "_V:" << v;
        } else {
            output << i->second;
        }
        return *this;
    }

    NiceExpressionPrinter &operator<<(Lit lit) {
        return (*this) << (sign(lit) ? '-' : '+') << var(lit);
    }

    NiceExpressionPrinter &operator<<(const VarVector &vs) {
        auto &me = *this;
        me << '[';
        for (size_t i = 0; i < vs.size(); i++)
            me << (i ? " " : "") << vs[i];
        return me << ']';
    }

    std::ostream &print(ID node, size_t offset = 0) {
        return visit(node, offset);
    }

  private:
    virtual std::ostream &visit_lit(ID, size_t offset, Lit lit) {
        const auto i = var2name.find(var(lit));
        if (i == var2name.end())
            o(offset) << "UNKNOWN:" << var(lit);
        assert(i != var2name.end());
        return o(offset) << (sign(lit) ? '-' : '+') << i->second;
    }

    virtual std::ostream &visit_and(ID, size_t offset, IDVector operands) {
        o(offset) << "AND (";
        return print_vector(operands, offset) << ")";
    }

    virtual std::ostream &visit_or(ID, size_t offset, IDVector operands) {
        o(offset) << "OR (";
        return print_vector(operands, offset) << ")";
    }

    virtual std::ostream &visit_not(ID, size_t offset, ID operand) {
        o(offset) << "NOT (" << std::endl;
        visit(operand, offset + 1) << std::endl;
        return o(offset) << ")";
    }

    virtual std::ostream &visit_false(ID, size_t offset) {
        return o(offset) << "F";
    }
    virtual std::ostream &visit_true(ID, size_t offset) {
        return o(offset) << "T";
    }

  private:
    std::ostream &output;
    const std::unordered_map<int, std::string> &var2name;

    std::ostream &o(size_t offset) {
        while (offset) {
            output << " ";
            --offset;
        }
        return output;
    }
    std::ostream &print_vector(IDVector operands, size_t offset) {
        bool first = true;
        bool nl = false;
        for (const auto &i : operands) {
            const NodeType t = i.get_type();
            if ((t != LITERAL) && (t != TRUE) && (t != FALSE)) {
                nl = true;
                break;
            }
        }

        for (const auto &n : operands) {
            if (nl)
                output << std::endl;
            if (!first && !nl)
                output << " ";
            visit(n, nl ? (offset + 1) : 0);
            first = false;
        }
        if (nl) {
            output << std::endl;
            o(offset);
        }
        return output;
    }
};

class NegationPusher : public MemoizedExpressionArgumentVisitor<ID, bool> {
  public:
    NegationPusher(Expressions &factory)
        : MemoizedExpressionArgumentVisitor<ID, bool>(factory) {}

    ID operator()(ID n) { return visit(n, false); }

  private:
    //  using ExpressionArgumentVisitor<ID,bool>::get_factory;
    //  using ExpressionArgumentVisitor<ID,bool>::factory;
    // using MemoizedExpressionArgumentVisitor<bool>::ops;

    virtual ID visit_lit(ID node, bool negate, Lit lit) {
        return negate ? factory.make_lit(~lit) : node;
    }

    virtual ID visit_and(ID node, bool negate, IDVector operands) {
        return negate ? factory.make_or(ops(operands, true, OR))
                      : MemoizedExpressionArgumentVisitor<ID, bool>::visit_and(
                            node, false, operands);
    }

    virtual ID visit_or(ID node, bool negate, IDVector operands) {
        return negate ? factory.make_and(ops(operands, true, AND))
                      : MemoizedExpressionArgumentVisitor<ID, bool>::visit_or(
                            node, false, operands);
    }

    virtual ID visit_not(ID, bool negate, ID operand) {
        return visit(operand, !negate); // negate negation
    }

    virtual ID visit_false(ID node, bool negate) {
        return negate ? factory.make_true() : node;
    }
    virtual ID visit_true(ID node, bool negate) {
        return negate ? factory.make_false() : node;
    }

  private:
    IDVector ops(IDVector es, bool n, NodeType op) {
        bool changed = false;
        assert(op == AND || op == OR);
        std::vector<ID> d;
        for (const auto &o : es) {
            const ID nv = visit(o, n);
            changed |= nv != o;
            changed |= factory.add_operand(d, nv, op);
        }
        return changed ? IDVector(d) : es;
    }
};

} // namespace qesto
