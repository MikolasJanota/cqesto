#pragma once
#include "auxiliary.h"
#include "data_structures.h"
#include "sat_interface.h"
#include "variable_manager.h"
#include "visitor.h"
namespace qesto {
class EncoderToSAT : private MemoizedExpressionVisitor<Lit> {
  public:
    EncoderToSAT(Expressions &factory, SATSOLVER &sat_solver,
                 const VariableManager &variable_manager)
        : MemoizedExpressionVisitor<Lit>(factory), sat_solver(sat_solver),
          variable_manager(variable_manager) {}

    Lit operator()(ID node) { return visit(node); }

    Var alloc(Var v) {
        while (variable_manager.max() < v)
            variable_manager.new_var();
        sat_solver.new_variables(v);
        return v;
    }

#ifdef USE_MINISAT
    // try to identify variables set to a constant by unit propagation in the
    // sat solver
    inline SATSPC::lbool get_val(ID n) {
        const auto i = get_m().find(n);
        if (i == get_m().end())
            return SATSPC::l_Undef;
        const Lit l = i->second;
        const Var v = var(l);
        const auto vv = sat_solver.value(v);
        if (!sign(l))
            return vv;
        if (vv == SATSPC::l_False)
            return SATSPC::l_True;
        if (vv == SATSPC::l_True)
            return SATSPC::l_False;
        assert(vv == Minisat::l_Undef);
        return Minisat::l_Undef;
    }
#endif

#ifdef USE_MINISAT
    void set_polarity(ID node, bool polarity) {
        const Lit en = (*this)(node);
        sat_solver.setPolarity(
            var(en), polarity == sign(en) ? Minisat::l_True : Minisat::l_False);
    }
#else
    void set_polarity(ID, bool) {}
#endif

  private:
    virtual Lit visit_lit(ID, Lit lit) { return lit; }

    virtual Lit visit_and(ID, IDVector operands) {
        return encode_ops(operands, true);
    }

    virtual Lit visit_or(ID, IDVector operands) {
        return encode_ops(operands, false);
    }

    virtual Lit visit_not(ID, ID operand) { return ~visit(operand); }

    virtual Lit visit_false(ID) {
        const Lit r = new_lit();
        sat_solver.addClause(~r);
        return r;
    }

    virtual Lit visit_true(ID) {
        const Lit r = new_lit();
        sat_solver.addClause(r);
        return r;
    }

  private:
    SATSOLVER &sat_solver;
    VariableManager variable_manager;

  private:
    Lit encode_ops(const IDVector &operands, bool and_op) {
        const Lit r = new_lit();
        SATCLS ls;
        SATCLS_CAPACITY(ls, operands.size() + 1);
        for (const auto &op : operands) {
            const Lit operand_encoding = visit(op);
            if (and_op) {
                SATCLS_PUSH(ls, ~operand_encoding);
                sat_solver.addClause(operand_encoding, ~r);
            } else {
                SATCLS_PUSH(ls, operand_encoding);
                sat_solver.addClause(~operand_encoding, r);
            }
        }
        SATCLS_PUSH(ls, and_op ? r : ~r);
        sat_solver.addClause_(ls);
        return r;
    }

    Lit new_lit() {
        const Var v = variable_manager.new_var();
        sat_solver.new_variables(v);
        return SATSPC::mkLit(v);
    }
};
} // namespace qesto
