/*
 * File:   LevelSolver.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 13:38
 */

#ifndef LEVELSOLVER_H
#define	LEVELSOLVER_H
#include <unordered_map>
#include <random>
#include "Expressions.h"
#include "MiniSatExt.h"
#include "EncoderToSAT.h"
#include "LevelInfo.h"
#include "minisat_auxiliary.h"
#include "Simplify.h"
#include "SetPolarities.h"
namespace qesto {
   class LevelSolver {
   public:
      enum VarType { PLAYER, OPPONENT, AUX };
      LevelSolver(const Options& options,
                  Expressions& factory,  size_t lev, const LevelInfo& levs);
      void add_var(Var v, VarType player);
      void add_constr(ID c);
      bool solve(const Substitution& assumptions);
      int analyze();
      ID learn(const std::unordered_set<Var>& dom, const Substitution& opp);
      lbool val(Var v) const {
         assert(vars.count(v));
         const auto& m=sat.model;
         return v<m.size() ? m[v] : Minisat::l_Undef;
      }
      const std::unordered_set<Var>& get_dom_vars() const { return dom_vars; }
      void randomize();
   private:
      static std::mt19937        rgen;
      const Options& options;
      Expressions& factory;
      const size_t lev;
      const LevelInfo& levs;
      std::unordered_set<Var> vars;
      std::unordered_set<Var> dom_vars;
      std::unordered_set<Var> plvars;
      std::vector<ID> constrs;
      VariableManager variable_manager;
      SATSPC::MiniSatExt sat;
      EncoderToSAT enc;
      std::unordered_map<Lit,ID> cut2id;
      Simplify simpl;
      SetPolarities pol;
   };
}

#endif	/* LEVELSOLVER_H */

