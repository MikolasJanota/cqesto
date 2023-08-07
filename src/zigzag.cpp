/*
 * File:   zigzag.cpp
 * Author: mikolas
 *
 * Created on 18 May 2015, 12:07
 */

#include "zigzag.h"
#include "auxiliary.h"
#include "sat_interface.h"
#include <vector>
using namespace qesto;
using SATSPC::l_False;
using SATSPC::l_True;
using SATSPC::l_Undef;
using SATSPC::lbool;
using SATSPC::mkLit;

void ZigZag::init() {
    solvers.resize(levels->qlev_count() + 1, NULL);
    for (size_t ql = 0; ql <= levels->qlev_count(); ++ql) {
        const QuantifierType qt =
            ql < levels->qlev_count()
                ? levels->level_type(ql)
                : opponent(levels->level_type(levels->qlev_count() - 1));
        solvers[ql] = new LevelSolver(options, factory, ql, *(levels.get()));
        LevelSolver &s = *(solvers[ql]);
        s.dprn = &d_prn;
        for (size_t j = 0; j <= std::min(ql, levels->qlev_count() - 1); ++j) {
            for (const auto v : levels->level_vars(j))
                s.add_var(v, qt == levels->level_type(j)
                                 ? LevelSolver::PLAYER
                                 : LevelSolver::OPPONENT);
        }
    }

    const auto lastlev = levels->qlev_count() - 1;
    const auto lastlevt = levels->level_type(lastlev);
    solvers[lastlev + 1]->add_constr(lastlevt == EXISTENTIAL
                                         ? factory.make_not(formula.output)
                                         : formula.output);
    if (options.full) {
        Substitution exs;
        Substitution uns;
        for (size_t ql = levels->qlev_count(); ql;) {
            --ql;
            // std::cerr<<"init full "<<ql<<std::endl;
            const QuantifierType qt = levels->level_type(ql);

            for (size_t j = ql + 1; j < levels->qlev_count(); ++j) {
                if (levels->level_type(j) != qt)
                    continue;
                for (auto i : levels->level_vars(j))
                    solvers[ql]->add_var(i, LevelSolver::AUX);
            }
            auto &os = qt == UNIVERSAL ? exs : uns;
            Reduce red(factory, os);
            const auto fla = qt == UNIVERSAL ? factory.make_not(formula.output)
                                             : formula.output;
            solvers[ql]->add_constr(red(fla));

            auto &s = qt == UNIVERSAL ? uns : exs;
            for (auto i : levels->level_vars(ql))
                s[i] = false;
        }
    } else {
        solvers[lastlev]->add_constr(lastlevt == EXISTENTIAL
                                         ? formula.output
                                         : factory.make_not(formula.output));
    }
}

void ZigZag::randomize() { solvers[0]->randomize(); }

ZigZag::~ZigZag() {
    for (auto s : solvers)
        delete s;
}

void ZigZag::block_solution() {
    auto &solver = *solvers[0];
    const auto &project =
        options.has_free ? formula.free : levels->level_vars(0);
    if (verb > 1)
        d_prn << "project over: " << project << '\n';
    assert(solver.get_last_solve());
    std::vector<ID> lits;
    lits.reserve(project.size());
    for (auto v : project) {
        const bool vv = solver.val(v) == l_True;
        const Lit l = vv ? mkLit(v) : ~mkLit(v);
        lits.push_back(factory.make_lit(l));
    }
    solver.add_constr(factory.make_not(factory.make_and(lits)));
}

int ZigZag::solve_all() {
    int count = 0;
    while (solve()) {
        count++;
        block_solution();
    }
    return count;
}
bool ZigZag::solve() {
    int curr_restarts = 0;
    lbool status = l_Undef;
    const double restart_inc = 1.5;
    const int restart_first = 100;
    const bool r = levels->qlev_count() > 2 && options.luby_restart != 0;
    while (status == l_Undef) {
        const double rest_base = luby(restart_inc, curr_restarts);
        status = solve_(r ? rest_base * restart_first : -1);
        curr_restarts++;
#ifdef USE_MINISAT
        randomize();
#endif
    }
    assert(status != l_Undef);
    if (solvers.size() > 1 && solvers[0]->get_last_solve()) {
        std::cout << "v";
        for (auto v : levels->level_vars(0)) {
            const bool vv = solvers[0]->val(v) == l_True;
            const Lit dl = vv ? mkLit(v) : ~mkLit(v);
            d_prn << " " << dl;
        }
        std::cout << " 0" << std::endl;
        if (verb > 1) {
            std::cout << "iv";
            for (auto v : levels->level_vars(0)) {
                const bool vv = solvers[0]->val(v) == l_True;
                std::cout << " " << (vv ? '+' : '-') << v;
            }
            std::cout << " 0" << std::endl;
        }
    }
    return status == l_True;
}

lbool ZigZag::solve_(int confl_budget) {
    size_t lev = 0;
    Substitution vals;
    std::vector<Decision> trail;
    while (true) {
        if (confl_budget == 0)
            return l_Undef;
        if (verb)
            std::cerr << "lev:" << lev << " (" << read_cpu_time() << ")"
                      << std::endl;
        assert(lev <= levels->qlev_count());
        const bool has_sol = solvers[lev]->solve(vals);
        const QuantifierType qt =
            lev < levels->qlev_count()
                ? levels->level_type(lev)
                : opponent(levels->level_type(levels->qlev_count() - 1));
        if (has_sol) {
            assert(lev < levels->qlev_count());
            for (auto v : levels->level_vars(lev)) {
                const bool vv = solvers[lev]->val(v) == l_True;
                const Lit dl = vv ? mkLit(v) : ~mkLit(v);
                vals[v] = vv;
                trail.push_back(Decision(qt, dl));
            }
            ++lev;
        } else {
            ++conflict_count;
            if (confl_budget > 0)
                confl_budget--;
            if (verb)
                std::cerr << "conflicts:" << conflict_count << std::endl;
            const int bt = solvers[lev]->analyze();
            if (verb)
                std::cerr << "bt:" << bt << std::endl;
            if (bt < 0)
                return qt == UNIVERSAL ? l_True : l_False;
            const auto btx = static_cast<size_t>(bt);
            Substitution opp;
            while (trail.size()) {
                const auto &d = trail.back();
                if (levels->qlev(d.decision_literal) < btx)
                    break;
                //           print_lit(std::cerr<<"btl:",d.decision_literal)<<std::endl;
                if (d.player == opponent(qt)) {
                    const auto opv = !sign(d.decision_literal);
                    //              std::cerr<<"opv:"<<opv<<std::endl;
                    opp[var(d.decision_literal)] = opv;
                }
                vals.erase(var(d.decision_literal));
                trail.pop_back();
            }
            solvers[bt]->add_constr(
                solvers[lev]->learn(solvers[bt]->get_dom_vars(), opp));
            lev = bt;
        }
    }
}
