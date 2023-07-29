/*
 * File:   MiniSatExt.hh
 * Author: mikolas
 *
 * Created on November 29, 2010, 5:40 PM
 */
#pragma once
#include "auxiliary.h"
#include "minisat/core/Solver.h"
#include <vector>

namespace SATSPC {
class MiniSatExt : public Solver {
  public:
    inline void bump(Var var) { varBumpActivity(var); }
    inline void new_variables(Var max_id);
    inline void new_variables(const std::vector<Var> &variables);
    inline int get_cls_count() { return clauses.size(); }
    inline const Minisat::LSet &get_conflict() { return conflict; }

    inline Minisat::lbool get_model_value(Minisat::Var v) const {
        return v < model.size() ? model[v] : l_Undef;
    }
};

inline void MiniSatExt::new_variables(Var max_id) {
    const int target_number = (int)max_id + 1;
    while (nVars() < target_number)
        newVar();
}

inline void MiniSatExt::new_variables(const std::vector<Var> &variables) {
    Var max_id = 0;
    for (Var v : variables)
        if (max_id < v)
            max_id = v;
    new_variables(max_id);
}
} // namespace SATSPC
