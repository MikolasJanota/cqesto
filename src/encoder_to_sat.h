#pragma once
#include "data_structures.h"
#include "minisat_auxiliary.h"
#include "minisat_ext.h"
#include "variable_manager.h"
#include "visitor.h"
namespace qesto {
class EncoderToSAT : private MemoizedExpressionVisitor<Lit> {
  public:
    EncoderToSAT(Expressions &factory, SATSPC::MiniSatExt &sat_solver,
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
    inline lbool get_val(ID n) {
        const auto i = get_m().find(n);
        if (i == get_m().end())
            return Minisat::l_Undef;
        const Lit l = i->second;
        const Var v = var(l);
        const auto vv = sat_solver.value(v);
        return sign(l) ? SATSPC::neg(vv) : vv;
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
    SATSPC::MiniSatExt &sat_solver;
    VariableManager variable_manager;

  private:
    Lit encode_ops(const IDVector &operands, bool and_op) {
        const Lit r = new_lit();
        const size_t sz = operands.size();
        vec<Lit> ls(sz + 1);
        ls[sz] = and_op ? r : ~r;
        for (size_t i = 0; i < sz; ++i) {
            const Lit operand_encoding = visit(operands[i]);
            ls[(int)i] = and_op ? ~operand_encoding : operand_encoding;
            if (and_op)
                sat_solver.addClause(operand_encoding, ~r);
            else
                sat_solver.addClause(~operand_encoding, r);
        }
        sat_solver.addClause_(ls);
        return r;
    }

    Lit new_lit() {
        const Var v = variable_manager.new_var();
        sat_solver.new_variables(v);
        return mkLit(v);
    }
};
} // namespace qesto
