/*
 * File:   LevelSolver.cpp
 * Author: mikolas
 *
 * Created on 18 May 2015, 13:38
 */

#include "auxiliary.h"
#include "minisat_auxiliary.h"
#include "LevelSolver.h"
#include "Eval.h"
#include "FindCut.h"
#include "MaxQLev.h"
#include "MakePossible.h"
#include <random>
using namespace qesto;
using SATSPC::lbool;
using SATSPC::l_Undef;
namespace qesto { extern NiceExpressionPrinter* dprn; }

std::mt19937  LevelSolver::rgen(1);

LevelSolver::LevelSolver(
   const Options& options,
   Expressions& factory,
   size_t lev,
#ifdef USE_SMS
   configSolver config,
#endif
   const LevelInfo& levs)
: options(options)
,factory(factory)
,lev(lev)
,levs(levs)
//,variable_manager(config.nextFreeVariable-1)
#ifdef USE_SMS
,sat(config)
#endif
,enc(factory,sat,variable_manager)
,simpl(options,factory,enc)
,pol(factory,enc)
{}

void LevelSolver::add_var(Var v, VarType vt) {
   assert(constrs.empty());
   while(variable_manager.max()<v) variable_manager.new_var();
   sat.new_variables(v);
   enc.alloc(v);
   insert_chk(vars,v);
   if(vt!=AUX) dom_vars.insert(v);
   if(vt==PLAYER&&(levs.qlev(v)<lev))
     plvars.insert(v);
}

void LevelSolver::add_constr(ID c) {
   const ID strengthening = options.get_simplify() ? simpl(c) : c;
   if(options.get_verbose()>3) {
     std::cerr<<"add_constr"<<std::endl; (*dprn)(strengthening)<<std::endl;
   }
   constrs.push_back(strengthening);
   sat.addClause(enc(strengthening));
   if(options.get_polarities()) pol(strengthening);
}

bool LevelSolver::solve(const Substitution& assumptions) {
  if(options.get_verbose()>3) {
    std::cerr<<"solving"<<std::endl;
    std::cerr<<"assump";
    FOR_EACH(i,assumptions) (*dprn)<<" "<<mkLit(i->first,!(i->second));
    std::cerr<<"["<<std::endl;
    FOR_EACH(i,constrs) (*dprn)(*i)<<std::endl;
    std::cerr<<"]"<<std::endl;
  }
   Eval ev(factory,assumptions);
   FOR_EACH(i,constrs) ev(*i);
   FindCut fc(factory,ev);
   FOR_EACH(i,constrs) fc(*i);
   vec<Lit> cut;
   cut2id.clear();
   FOR_EACH(i,fc.get_cut()) {
      const auto l=*i;
      assert(ev(l)!=l_Undef);
      const auto el=enc(l);
      cut.push(el);
      cut2id[el]=l;
   }
   return sat.solve(cut);
}

int LevelSolver::analyze() {
   MaxQLev mql(factory,plvars,levs);
   int bt=-1;
   const auto& confl=sat.conflict;
   for (int i=0; i<confl.size(); i++) {
     const auto el=confl[i];
     const auto j = cut2id.find(~el);
     assert(j != cut2id.end());
     const ID l=j->second;
     bt=std::max(bt,mql(l));
   }
   return bt;
}

ID LevelSolver::learn(const std::unordered_set<Var>& dom, const Substitution& opp) {
   MakePossible mp(factory,dom,opp);
   vector<ID> ops;
   const auto& confl=sat.conflict;
   for (int i=0; i<confl.size(); i++) {
     const auto el=confl[i];
     const auto j = cut2id.find(~el);
     assert(j != cut2id.end());
     const ID l=j->second;
     ops.push_back(mp(factory.make_not(l)));
   }
   const auto lrn=factory.make_or(ops);
   if(options.get_verbose()>3) {
       std::cerr<<"lrn:"; (*dprn)(lrn)<<std::endl;
   }
   return lrn;
}


void LevelSolver::randomize() {
    Var v = sat.nVars();
    std::uniform_int_distribution<> d(0, 99);
    while (v--)
        if (d(rgen) < 25) sat.bump(v);
}
