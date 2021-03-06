/*
 * File:   ZigZag.cpp
 * Author: mikolas
 *
 * Created on 18 May 2015, 12:07
 */

#include "ZigZag.h"
#include "minisat_auxiliary.h"
using namespace qesto;
using std::min;
using Minisat::l_True;
using Minisat::l_False;
using Minisat::l_Undef;

void ZigZag::init() {
   solvers.resize(levels.qlev_count()+1,NULL);
   for(size_t ql=0;ql<=levels.qlev_count();++ql) {
      const QuantifierType qt=
               ql<levels.qlev_count() ?  levels.level_type(ql)
                                      : opponent(levels.level_type(levels.qlev_count()-1));
      solvers[ql]=new LevelSolver(options,factory,ql,levels);
      LevelSolver& s = *(solvers[ql]);
      for(size_t j=0;j<=min(ql,levels.qlev_count()-1);++j) {
         FOR_EACH(vi,levels.level_vars(j))
           s.add_var(*vi,qt==levels.level_type(j) ?
                         LevelSolver::PLAYER : LevelSolver::OPPONENT);
      }
   }

   const auto lastlev=levels.qlev_count()-1;
   const auto lastlevt=levels.level_type(lastlev);
   solvers[lastlev+1]->add_constr(lastlevt==EXISTENTIAL ?
                                  factory.make_not(formula.output) : formula.output);
   if(options.get_full()) {
      Substitution exs;
      Substitution uns;
      for(size_t ql=levels.qlev_count(); ql; ) {
         --ql;
         //std::cerr<<"init full "<<ql<<std::endl;
         const QuantifierType qt=levels.level_type(ql);

         for(size_t j=ql+1; j<levels.qlev_count(); ++j) {
           if(levels.level_type(j)!=qt) continue;
           FOR_EACH(i, levels.level_vars(j))
             solvers[ql]->add_var(*i,LevelSolver::AUX);
         }
         auto& os=qt==UNIVERSAL ? exs : uns;
         Reduce red(factory,os);
         const auto fla=qt==UNIVERSAL ? factory.make_not(formula.output) : formula.output;
         solvers[ql]->add_constr(red(fla));

         auto& s=qt==UNIVERSAL ? uns : exs;
         FOR_EACH(i, levels.level_vars(ql)) s[*i]=false;
      }
   } else {
      solvers[lastlev]->add_constr(lastlevt==EXISTENTIAL ?
                                   formula.output : factory.make_not(formula.output));
   }
}

void ZigZag::randomize() {
    solvers[0]->randomize();
}

bool ZigZag::solve() {
    int curr_restarts = 0;
    lbool status=l_Undef;
    const double restart_inc=1.5;
    const int restart_first=100;
    const bool r = levels.qlev_count() > 2 && options.get_luby_rest() != 0;
    while(status==l_Undef){
        const double rest_base = luby(restart_inc, curr_restarts);
        status = solve_(r ? rest_base * restart_first : -1);
        curr_restarts++;
        randomize();
    }
    assert(status!=l_Undef);
    return status==l_True;
}

lbool ZigZag::solve_(int confl_budget) {
   size_t lev=0;
   Substitution vals;
   vector<Decision> trail;
   while(1) {
      if (confl_budget == 0) return l_Undef;
      if(verb) std::cerr<<"lev:"<<lev<<" ("<<read_cpu_time()<<")"<<std::endl;
      assert(lev <= levels.qlev_count());
      const bool has_sol=solvers[lev]->solve(vals);
      const QuantifierType qt = lev<levels.qlev_count() ? levels.level_type(lev)
                                                        : opponent(levels.level_type(levels.qlev_count()-1));
      if(has_sol) {
         assert(lev<levels.qlev_count());
         FOR_EACH(vi,levels.level_vars(lev)) {
            const auto v=*vi;
            const bool vv=solvers[lev]->val(v)==l_True;
            const Lit dl=vv ? mkLit(v) : ~mkLit(v);
            vals[v]=vv;
            trail.push_back(Decision(qt,dl));
         }
         ++lev;
      } else {
         ++conflict_count;
         if (confl_budget > 0) confl_budget--;
         if(verb)std::cerr<<"conflicts:"<<conflict_count<<std::endl;
         const int bt=solvers[lev]->analyze();
         if(verb)std::cerr<<"bt:"<<bt<<std::endl;
         if(bt<0) return qt==UNIVERSAL ? l_True : l_False;
         const size_t btx=(size_t)bt;
         Substitution opp;
         while(trail.size()) {
           const auto& d=trail.back();
           if(levels.qlev(d.decision_literal)<btx) break;
//           print_lit(std::cerr<<"btl:",d.decision_literal)<<std::endl;
           if(d.player==opponent(qt)) {
              const auto opv=!sign(d.decision_literal);
//              std::cerr<<"opv:"<<opv<<std::endl;
              opp[var(d.decision_literal)]=opv;
           }
           vals.erase(var(d.decision_literal));
           trail.pop_back();
         }
         solvers[bt]->add_constr(solvers[lev]->learn(solvers[bt]->get_dom_vars(),opp));
         lev=bt;
      }
   }
}
