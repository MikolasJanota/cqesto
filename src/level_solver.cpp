/*
 * File:   level_solver.cpp
 * Author: mikolas
 *
 * Created on 18 May 2015, 13:38
 */

#include "level_solver.h"
#include "auxiliary.h"
#include "eval.h"
#include "find_cut.h"
#include "make_possible.h"
#include "max_qlev.h"
#include <random>
#include <vector>
using namespace qesto;

using SATSPC::mkLit;
std::mt19937 LevelSolver::rgen(1);

LevelSolver::LevelSolver(const Options &options, Expressions &factory,
                         size_t lev, const LevelInfo &levs)
    : options(options), factory(factory), lev(lev), levs(levs),
      enc(factory, sat, variable_manager), simpl(options, factory, enc),
      pol(factory, enc) {}

void LevelSolver::add_var(Var v, VarType vt) {
    assert(constrs.empty());
    enc.alloc(v);
    insert_chk(vars, v);
    if (vt != AUX)
        dom_vars.insert(v);
    if (vt == PLAYER && (levs.qlev(v) < lev))
        plvars.insert(v);
}

void LevelSolver::add_constr(ID c) {
    const ID strengthening = options.simplify ? simpl(c) : c;
    if (options.verbose > 3) {
        std::cerr << "add_constr" << std::endl;
        (*dprn)(strengthening) << std::endl;
    }
    constrs.push_back(strengthening);
    sat.addClause(enc(strengthening));
    if (options.polarities)
        pol(strengthening);
}

bool LevelSolver::solve(const Substitution &assumptions) {
    if (options.verbose > 3) {
        std::cerr << "solving" << std::endl;
        std::cerr << "assump";
        for (const auto &i : assumptions)
            (*dprn) << " " << mkLit(i.first, !(i.second));
        std::cerr << "[" << std::endl;
        for (const auto &i : constrs)
            (*dprn)(i) << std::endl;
        std::cerr << "]" << std::endl;
    }
    Eval ev(factory, assumptions);
    for (const auto &i : constrs)
        ev(i);
    FindCut fc(factory, ev);
    for (const auto &i : constrs)
        fc(i);
    const auto &cut = fc.get_cut();
    SATCLS cut_clause;
    SATCLS_CAPACITY(cut_clause, cut.size());
    cut2id.clear();
    for (const auto &l : fc.get_cut()) {
        assert(ev(l) != SATSPC::l_Undef);
        const auto el = enc(l);
        SATCLS_PUSH(cut_clause, el);
        cut2id[el] = l;
    }
    lastSolve = sat.solve(cut_clause);
    return lastSolve;
}

int LevelSolver::analyze() {
    MaxQLev mql(factory, plvars, levs);
    int bt = -1;
    const auto &confl = sat.get_conflict();
    for (auto i = confl.size(); i--;) {
        const auto el = confl[i];
        const auto j = cut2id.find(~el);
        assert(j != cut2id.end());
        const ID l = j->second;
        bt = std::max(bt, mql(l));
    }
    return bt;
}

ID LevelSolver::learn(const std::unordered_set<Var> &dom,
                      const Substitution &opp) {
    MakePossible mp(factory, dom, opp);
    std::vector<ID> ops;
    const auto &confl = sat.get_conflict();
    for (auto i = confl.size(); i--;) {
        const auto el = confl[i];
        const auto j = cut2id.find(~el);
        assert(j != cut2id.end());
        const ID l = j->second;
        ops.push_back(mp(factory.make_not(l)));
    }
    const auto lrn = factory.make_or(ops);
    if (options.verbose > 3) {
        std::cerr << "lrn:";
        (*dprn)(lrn) << std::endl;
    }
    return lrn;
}

void LevelSolver::randomize() {
#ifdef USE_MINISAT
    Var v = sat.nVars();
    std::uniform_int_distribution<> d(0, 99);
    while (v--)
        if (d(rgen) < 25)
            sat.bump(v);
#endif
}
