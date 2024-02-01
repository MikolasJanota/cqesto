/*
 * File:   zigzag.h
 * Author: mikolas
 *
 * Created on 18 May 2015, 12:07
 */

#pragma once
#include "level_info.h"
#include "level_solver.h"
#include "qtypes.h"
#include "statistics.h"
#include <memory>
#include <vector>
namespace qesto {

class ZigZag {
  public:
    ZigZag(const Options &options, StatisticsManager &statistics,
           Expressions &factory, const QFla &qf, NiceExpressionPrinter &prn)
        : options(options), m_statistics(statistics), factory(factory),
          d_prn(prn), formula(qf), verb(options.verbose) {

        if (options.enumerate && !formula.pref.empty()) {
            auto &block = formula.pref[0].second;
            block = VarVector::conc(formula.free, block);
            if (options.verbose > 3 && !formula.free.empty())
                d_prn << "free: " << formula.free << '\n';
        }

        if (options.verbose > 3) {
            std::cerr << "input" << std::endl;
            prn(qf.output) << std::endl;
        }
        levels = std::make_unique<LevelInfo>(formula.pref);
        init();
    }
    virtual ~ZigZag();

    bool solve();
    int solve_all();

  private:
    SATSPC::lbool solve_(int confl_budget);
    void randomize();
    void block_solution();

    struct Decision {
        Decision(QuantifierType player, Lit decision_literal)
            : player(player), decision_literal(decision_literal) {}

        QuantifierType player;
        Lit decision_literal;
    };

    const Options &options;
    StatisticsManager &m_statistics;
    Expressions &factory;
    NiceExpressionPrinter &d_prn;
    std::unique_ptr<LevelInfo> levels;
    QFla formula;
    std::vector<LevelSolver *> solvers;
    void init();
    int verb;
};
} // namespace qesto
