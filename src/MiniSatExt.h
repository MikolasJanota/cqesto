/*
 * File:   MiniSatExt.hh
 * Author: mikolas
 *
 * Created on November 29, 2010, 5:40 PM
 */
#ifndef MINISATEXT_HH
#define	MINISATEXT_HH
#include "auxiliary.h"
#include <vector>
#if USE_SIMP
#include "simp/SimpSolver.h"
#else
#include "minisat/core/Solver.h"
#endif

namespace SATSPC {
  class MiniSatExt : public Solver {
  public:
    inline void bump(const Var var)        { varBumpActivity(var); }
    inline Var new_variables(Var max_id);
    inline Var new_variables(const std::vector<Var>& variables);
    inline int get_cls_count() {return clauses.size();}
  };

  inline Var MiniSatExt::new_variables(Var max_id) {
    const int target_number = (int)max_id+1;
    while (nVars() < target_number) newVar();
    return max_id;
  }

  inline Var MiniSatExt::new_variables(const std::vector<Var>& variables) {
    Var max_id = 0;
    FOR_EACH(variable_index, variables) {
      const Var v = *variable_index;
      if (max_id < v) max_id = v;
    }
    return new_variables(max_id);
  }
}
#endif	/* MINISATEXT_HH */
