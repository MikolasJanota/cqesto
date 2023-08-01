/*
 * File:   level_solver.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 13:38
 */
#pragma once
#include "encoder_to_sat.h"
#include "expressions.h"
#include "level_info.h"
#include "sat_interface.h"
#include "set_polarities.h"
#include "simplify.h"
#include <random>
#include <unordered_map>
namespace qesto {
class LevelSolver {
  public:
    enum VarType { PLAYER, OPPONENT, AUX };
    LevelSolver(const Options &options, Expressions &factory, size_t lev,
                const LevelInfo &levs);
    void add_var(Var v, VarType player);
    void add_constr(ID c);
    bool solve(const Substitution &assumptions);
    int analyze();
    ID learn(const std::unordered_set<Var> &dom, const Substitution &opp);
    SATSPC::lbool val(Var v) const {
        assert(vars.count(v));
        return sat.get_model_value(v);
    }
    const std::unordered_set<Var> &get_dom_vars() const { return dom_vars; }
    void randomize();

    NiceExpressionPrinter *dprn = nullptr;

    bool get_last_solve() const { return lastSolve; }

  private:
    static std::mt19937 rgen;
    const Options &options;
    Expressions &factory;
    const size_t lev;
    const LevelInfo &levs;
    std::unordered_set<Var> vars;
    std::unordered_set<Var> dom_vars;
    std::unordered_set<Var> plvars;
    std::vector<ID> constrs;
    VariableManager variable_manager;
    SATSOLVER sat;
    EncoderToSAT enc;
    std::unordered_map<Lit, ID> cut2id;
    Simplify simpl;
    SetPolarities pol;
    bool lastSolve;
};
} // namespace qesto
